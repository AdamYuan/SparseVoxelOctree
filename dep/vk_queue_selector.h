//
// version 1.0.0 (2021-01-24)
//

//
// Use this in *one* source file
//   #define VQS_IMPLEMENTATION
//   #include "vk_queue_selector.h"
//
#ifndef VK_QUEUE_SELECTOR_H
#define VK_QUEUE_SELECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VULKAN_H_
#include <vulkan/vulkan.h>
#endif

#ifndef VQS_API
#define VQS_API extern
#endif

#include <inttypes.h>
typedef struct VqsVulkanFunctions {
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
} VqsVulkanFunctions;

typedef struct VqsQueueRequirements {
	VkQueueFlags requiredFlags;
	float priority;                           // should be 1.0f as default
	VkSurfaceKHR requiredPresentQueueSurface; // set to NULL if don't need present queue
} VqsQueueRequirements;

typedef struct VqsQueueSelection {
	uint32_t queueFamilyIndex, queueIndex;
	uint32_t presentQueueFamilyIndex, presentQueueIndex;
} VqsQueueSelection;

typedef struct VqsQueryCreateInfo {
	VkPhysicalDevice physicalDevice;
	uint32_t queueRequirementCount;
	const VqsQueueRequirements *pQueueRequirements;
	const VqsVulkanFunctions *pVulkanFunctions;
} VqsQueryCreateInfo;

typedef struct VqsQuery_T *VqsQuery;

VQS_API VkResult vqsCreateQuery(const VqsQueryCreateInfo *pCreateInfo, VqsQuery *pQuery);
VQS_API void vqsDestroyQuery(VqsQuery query);

VQS_API VkResult vqsPerformQuery(VqsQuery query);
VQS_API void vqsGetQueueSelections(VqsQuery query, VqsQueueSelection *pQueueSelections);
VQS_API void vqsEnumerateDeviceQueueCreateInfos(VqsQuery query, uint32_t *pDeviceQueueCreateInfoCount,
                                                VkDeviceQueueCreateInfo *pDeviceQueueCreateInfos,
                                                uint32_t *pQueuePriorityCount, float *pQueuePriorities);

#ifdef __cplusplus
}
#endif

#endif // VK_QUEUE_SELECTOR_H

//////////////////////////////////////////////////////////////////////////////
//
//   IMPLEMENTATION
//
#ifdef VQS_IMPLEMENTATION
#undef VQS_IMPLEMENTATION

#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if !defined(VQS_STATIC_VULKAN_FUNCTIONS) && !defined(VK_NO_PROTOTYPES)
#define VQS_STATIC_VULKAN_FUNCTIONS 1
#endif

struct Node;
typedef struct Edge {
	struct Node *to;
	struct Edge *next, *rev;
	int32_t cap, cost;
} Edge;
typedef struct Node {
	Edge *head, *path;
	int32_t dist, flow;
	bool inQueue;
} Node;

struct VqsQuery_T {
	VkPhysicalDevice physicalDevice;
	VqsVulkanFunctions vulkanFunctions;
	VkQueueFamilyProperties *queueFamilyProperties;
	VqsQueueRequirements *queueRequirements;
	// BINARY GRAPH
	Node *nodes, *pLeftNodes, *pRightNodes, *pSNode, *pTNode, **spfaQueue;
	Edge *edges, *pInteriorEdges, *pLeftEdges, *pRightEdges;
	int32_t *minInteriorCosts; // the min interior edge cost for each left nodes
	union {
		uint32_t leftCount, queueFamilyCount;
	};
	union {
		uint32_t rightCount, queueRequirementCount;
	};
	uint32_t interiorEdgeCount, nodeCount, edgeCount;
	// RESULTS
	uint32_t *resultQueueFamilyIndices, *resultPresentQueueFamilyIndices, *queueFamilyCounters;
};

#define VQS_ALLOC(x, type, count)                                                                                      \
	{ (x) = (type *)calloc(count, sizeof(type)); }
#define VQS_ALLOC_VK(x, type, count)                                                                                   \
	{                                                                                                                  \
		VQS_ALLOC(x, type, count);                                                                                     \
		if (x == NULL)                                                                                                 \
			return VK_ERROR_OUT_OF_HOST_MEMORY;                                                                        \
	}
#define VQS_FREE(x)                                                                                                    \
	{                                                                                                                  \
		free(x);                                                                                                       \
		(x) = NULL;                                                                                                    \
	}

uint32_t queueFlagDist(uint32_t l, uint32_t r, float f) {
	const uint32_t kMask = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	uint32_t i = (l ^ r) & kMask;
	// popcount
	i = i - ((i >> 1u) & 0x55555555u);
	i = (i & 0x33333333u) + ((i >> 2u) & 0x33333333u);
	i = (((i + (i >> 4u)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24u;
	// multiplied by f
	int32_t ret = (i + 1) * f * 100.0f;
	return (uint32_t)ret;
}

void addEdge(Edge *edge, Edge *edgeRev, Node *from, Node *to, int32_t cap, int32_t cost) {
	edge->to = to;
	edge->next = from->head;
	edge->rev = edgeRev;
	edge->cap = cap;
	edge->cost = cost;

	edgeRev->to = from;
	edgeRev->next = to->head;
	edgeRev->rev = edge;
	edgeRev->cap = 0;
	edgeRev->cost = -cost;

	from->head = edge;
	to->head = edgeRev;
}

uint32_t queryBuildInteriorEdges(VqsQuery query, bool buildEdge) {
	uint32_t counter = 0;

	for (uint32_t j = 0; j < query->queueRequirementCount; ++j) {
		VkQueueFlags requiredFlags = query->queueRequirements[j].requiredFlags;
		VkSurfaceKHR requiredPresentQueueSurface = query->queueRequirements[j].requiredPresentQueueSurface;
		float priority = query->queueRequirements[j].priority;

		for (uint32_t i = 0; i < query->queueFamilyCount; ++i) {
			if (query->queueFamilyProperties[i].queueCount == 0)
				continue; // skip empty queue families

			VkQueueFlags flags = query->queueFamilyProperties[i].queueFlags;

			if (requiredPresentQueueSurface && query->resultPresentQueueFamilyIndices[j] == UINT32_MAX) {
				VkBool32 presentSupport;
				if (query->vulkanFunctions.vkGetPhysicalDeviceSurfaceSupportKHR(
				        query->physicalDevice, i, requiredPresentQueueSurface, &presentSupport) != VK_SUCCESS)
					presentSupport = VK_FALSE;

				if (presentSupport && (flags & requiredFlags) == requiredFlags) {
					if (buildEdge) {
						addEdge(query->pInteriorEdges + counter, query->pInteriorEdges + counter + 1,
						        query->pLeftNodes + i, query->pRightNodes + j, 1,
						        queueFlagDist(flags, requiredFlags, priority));
					}
					counter += 2u;
				}
			} else {
				if ((flags & requiredFlags) == requiredFlags) {
					if (buildEdge) {
						addEdge(query->pInteriorEdges + counter, query->pInteriorEdges + counter + 1,
						        query->pLeftNodes + i, query->pRightNodes + j, 1,
						        queueFlagDist(flags, requiredFlags, priority));
					}
					counter += 2u;
				}
			}
		}
	}
	return counter;
}

bool querySpfa(VqsQuery query) {
	for (uint32_t i = 0; i < query->nodeCount; ++i) {
		query->nodes[i].dist = INT32_MAX;
		query->nodes[i].flow = INT32_MAX;
		query->nodes[i].inQueue = false;
		query->nodes[i].path = NULL;
	}

	uint32_t queTop = 0, queTail = 0;
	const uint32_t kQueSize = query->nodeCount;
#define QUE_PUSH(x) query->spfaQueue[(queTop++) % kQueSize] = (x)
#define QUE_POP query->spfaQueue[(queTail++) % kQueSize]
	query->pSNode->inQueue = true;
	query->pSNode->dist = 0;
	QUE_PUSH(query->pSNode);

	while (queTop != queTail) {
		Node *cur = QUE_POP;
		cur->inQueue = false;
		// printf("node: %u, dist: %d\n", cur - query->nodes, cur->dist);

		for (Edge *e = cur->head; e; e = e->next) {
			if (e->cap > 0 && e->to->dist > cur->dist + e->cost) {
				e->to->dist = cur->dist + e->cost;
				e->to->path = e;
				e->to->flow = (cur->flow < e->cap) ? cur->flow : e->cap; // min

				if (!e->to->inQueue) {
					e->to->inQueue = true;
					QUE_PUSH(e->to);
				}
			}
		}
	}
#undef QUE_PUSH
#undef QUE_POP

	return query->pTNode->path;
}

int32_t queryMcmfWithLimits(VqsQuery query, int32_t flow, uint32_t *leftFlowLimits) {
	for (;;) {
		bool full = true;

		{
			int32_t minCost = INT32_MAX;
			for (uint32_t i = 0; i < query->leftCount; ++i) { // append flow
				if (leftFlowLimits[i] > 0) {
					full = false;
					++query->pLeftEdges[i * 2u].cap;
					if (query->minInteriorCosts[i] < minCost)
						minCost = query->minInteriorCosts[i];
					--leftFlowLimits[i];
				}
			}
			for (uint32_t i = 0; i < query->interiorEdgeCount; i += 2) { // cancel negative rings
				Edge *e = query->pInteriorEdges + i;
				if (e->cost > minCost)
					e->rev->cap = 0;
			}
		}
		if (full)
			return flow;

		/*printf("DATA:\n");
		printf("%u %u %ld %ld\n", query->nodeCount, query->edgeCount / 2, query->pSNode - query->nodes + 1,
		       query->pTNode - query->nodes + 1);
		for (uint32_t i = 0; i < query->edgeCount; i += 2) {
		    printf("%ld %ld %u %u\n", query->edges[i].rev->to - query->nodes + 1, query->edges[i].to - query->nodes + 1,
		           query->edges[i].cap, query->edges[i].cost);
		}
		printf("\n");*/

		while (querySpfa(query)) {
			int32_t x = query->pTNode->flow;
			flow += x;

			Node *cur = query->pTNode;
			while (cur != query->pSNode) {
				cur->path->cap -= x;
				cur->path->rev->cap += x;
				cur = cur->path->rev->to;
			}
		}
		if (flow == query->rightCount)
			return flow;
	}
}

VkResult queryInit(VqsQuery query, const VqsQueryCreateInfo *pCreateInfo) {
	// SET PHYSICAL DEVICE
	query->physicalDevice = pCreateInfo->physicalDevice;

	// SET VULKAN FUNCTIONS
	if (pCreateInfo->pVulkanFunctions) {
		query->vulkanFunctions = *(pCreateInfo->pVulkanFunctions);
	} else {
#if VQS_STATIC_VULKAN_FUNCTIONS == 1
		query->vulkanFunctions.vkGetPhysicalDeviceQueueFamilyProperties =
		    (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vkGetPhysicalDeviceQueueFamilyProperties;
		query->vulkanFunctions.vkGetPhysicalDeviceSurfaceSupportKHR =
		    (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetPhysicalDeviceSurfaceSupportKHR;
#endif
	}

	// SET QUEUE REQUIREMENT COUNT & GRAPH RIGHT COUNT
	query->queueRequirementCount = pCreateInfo->queueRequirementCount;

	// GET QUEUE FAMILY PROPERTIES
	query->vulkanFunctions.vkGetPhysicalDeviceQueueFamilyProperties(query->physicalDevice, &query->queueFamilyCount,
	                                                                NULL); // SET QUEUE FAMILY COUNT & GRAPH LEFT COUNT
	if (query->queueFamilyCount == 0)
		return VK_ERROR_UNKNOWN;
	VQS_ALLOC_VK(query->queueFamilyProperties, VkQueueFamilyProperties, query->queueFamilyCount);
	query->vulkanFunctions.vkGetPhysicalDeviceQueueFamilyProperties(query->physicalDevice, &query->queueFamilyCount,
	                                                                query->queueFamilyProperties);

	// COPY QUEUE REQUIREMENTS
	VQS_ALLOC_VK(query->queueRequirements, VqsQueueRequirements, query->queueRequirementCount);
	memcpy(query->queueRequirements, pCreateInfo->pQueueRequirements,
	       query->queueRequirementCount * sizeof(VqsQueueRequirements));

	// ALLOC RESULT ARRAYS
	VQS_ALLOC_VK(query->resultQueueFamilyIndices, uint32_t, query->queueRequirementCount);
	VQS_ALLOC_VK(query->resultPresentQueueFamilyIndices, uint32_t, query->queueRequirementCount);

	// ALLOC QUEUE FAMILY COUNTERS
	VQS_ALLOC_VK(query->queueFamilyCounters, uint32_t, query->queueFamilyCount);

	return VK_SUCCESS;
}

VkResult queryPreprocessPresentQueues(VqsQuery query) {
	// FIND PRESENT QUEUES
	for (uint32_t j = 0; j < query->queueRequirementCount; ++j) {
		VkQueueFlags requiredFlags = query->queueRequirements[j].requiredFlags;
		VkSurfaceKHR requiredPresentQueueSurface = query->queueRequirements[j].requiredPresentQueueSurface;
		query->resultPresentQueueFamilyIndices[j] = UINT32_MAX;

		if (requiredPresentQueueSurface) { // require present queue
			bool existQueueWithPresentSupport = false;
			for (uint32_t i = 0; i < query->leftCount; ++i) {
				if (query->queueFamilyProperties[i].queueCount == 0)
					continue; // skip empty queue families
				VkQueueFlags flags = query->queueFamilyProperties[i].queueFlags;
				VkBool32 presentSupport;
				if (query->vulkanFunctions.vkGetPhysicalDeviceSurfaceSupportKHR(
				        query->physicalDevice, i, requiredPresentQueueSurface, &presentSupport) != VK_SUCCESS)
					presentSupport = VK_FALSE;

				if (presentSupport) {
					existQueueWithPresentSupport = true;
					query->resultPresentQueueFamilyIndices[j] = i;

					if ((flags & requiredFlags) == requiredFlags) {
						// exist queue family that both support present and meet
						// requiredFlags leave presentQueueFamilyIndex as
						// UINT32_MAX
						query->resultPresentQueueFamilyIndices[j] = UINT32_MAX;
						break;
					}
				}
			}

			if (!existQueueWithPresentSupport)
				return VK_ERROR_UNKNOWN;
		}
	}
	return VK_SUCCESS;
}

VkResult queryBuildGraph(VqsQuery query) {
	// ALLOC GRAPH
	query->nodeCount = query->leftCount + query->rightCount + 2u;
	VQS_ALLOC_VK(query->nodes, Node, query->nodeCount);

	query->interiorEdgeCount = queryBuildInteriorEdges(query, false);
	query->edgeCount = query->interiorEdgeCount + (query->leftCount + query->rightCount) * 2u;
	VQS_ALLOC_VK(query->edges, Edge, query->edgeCount);

	// not needed since calloc is used
	/*for (uint32_t i = 0; i < query->nodeCount; ++i) {
	    query->nodes[i].head = NULL;
	}*/
	query->pSNode = query->nodes;
	query->pTNode = query->pSNode + 1;
	query->pLeftNodes = query->pTNode + 1;
	query->pRightNodes = query->pLeftNodes + query->leftCount;

	query->pLeftEdges = query->edges;
	query->pRightEdges = query->pLeftEdges + query->leftCount * 2u;
	query->pInteriorEdges = query->pRightEdges + query->rightCount * 2u;

	// BUILD GRAPH
	for (uint32_t i = 0; i < query->leftCount; ++i) { // S -> LEFT NODES
		addEdge(query->pLeftEdges + (i * 2), query->pLeftEdges + (i * 2 + 1), query->pSNode, query->pLeftNodes + i, 0,
		        0);
	}
	for (uint32_t i = 0; i < query->rightCount; ++i) { // RIGHT NODES -> T
		addEdge(query->pRightEdges + (i * 2), query->pRightEdges + (i * 2 + 1), query->pRightNodes + i, query->pTNode,
		        1, 0);
	}
	queryBuildInteriorEdges(query, true);

	// SET MIN INTERIOR COSTS (for negative ring canceling)
	VQS_ALLOC_VK(query->minInteriorCosts, int32_t, query->leftCount);
	for (uint32_t i = 0; i < query->leftCount; ++i) {
		query->minInteriorCosts[i] = INT32_MAX;
	}

	for (uint32_t i = 0; i < query->interiorEdgeCount; i += 2) {
		Edge *e = query->pInteriorEdges + i;
		int32_t *target = query->minInteriorCosts + (e->rev->to - query->pLeftNodes);
		if (e->cost < *target)
			*target = e->cost;
	}

	// ALLOC SPFA QUEUE
	VQS_ALLOC_VK(query->spfaQueue, Node *, query->nodeCount);

	return VK_SUCCESS;
}

VkResult queryMainAlgorithm(VqsQuery query) {
	// RUN MAIN ALGORITHM
	uint32_t *leftFlowLimits;
	VQS_ALLOC_VK(leftFlowLimits, uint32_t, query->leftCount);

	for (uint32_t i = 0; i < query->leftCount; ++i)
		leftFlowLimits[i] = query->queueFamilyProperties[i].queueCount;

	int32_t flow = 0;
	flow = queryMcmfWithLimits(query, flow, leftFlowLimits);
	if (flow < query->rightCount) {
		for (uint32_t i = 0; i < query->leftCount; ++i) {
			uint32_t queueCount = query->queueFamilyProperties[i].queueCount;
			leftFlowLimits[i] = query->rightCount > queueCount ? query->rightCount - queueCount : 0u;
		}
		flow = queryMcmfWithLimits(query, flow, leftFlowLimits);

		if (flow < query->rightCount) {
			VQS_FREE(leftFlowLimits);
			return VK_ERROR_UNKNOWN;
		}
	}
	VQS_FREE(leftFlowLimits);
	return VK_SUCCESS;
}

VkResult queryFetchResults(VqsQuery query) {
	// FETCH RESULTS
	for (uint32_t i = 0; i < query->interiorEdgeCount; i += 2) {
		Edge *e = query->pInteriorEdges + i;
		if (e->cap == 0) {
			query->resultQueueFamilyIndices[e->to - query->pRightNodes] = e->rev->to - query->pLeftNodes;
		}
	}

	return VK_SUCCESS;
}

void querySetQueueFamilyCounters(VqsQuery query) {
	for (uint32_t i = 0; i < query->queueFamilyCount; ++i) {
		query->queueFamilyCounters[i] = 0u;
	}
	for (uint32_t i = 0; i < query->queueRequirementCount; ++i) {
		++query->queueFamilyCounters[query->resultQueueFamilyIndices[i]];
		if (query->resultPresentQueueFamilyIndices[i] != UINT32_MAX)
			++query->queueFamilyCounters[query->resultPresentQueueFamilyIndices[i]];
	}
}

void queryFree(VqsQuery query) {
	VQS_FREE(query->nodes);
	VQS_FREE(query->edges);
	VQS_FREE(query->minInteriorCosts);
	VQS_FREE(query->spfaQueue);
	VQS_FREE(query->resultQueueFamilyIndices);
	VQS_FREE(query->resultPresentQueueFamilyIndices);
	VQS_FREE(query->queueFamilyCounters);
	VQS_FREE(query->queueFamilyProperties);
	VQS_FREE(query->queueRequirements);
}

VkResult vqsCreateQuery(const VqsQueryCreateInfo *pCreateInfo, VqsQuery *pQuery) {
	VQS_ALLOC_VK(*pQuery, struct VqsQuery_T, 1);

#define TRY_STMT(stmt)                                                                                                 \
	{                                                                                                                  \
		VkResult result = stmt;                                                                                        \
		if (result != VK_SUCCESS) {                                                                                    \
			vqsDestroyQuery(*pQuery);                                                                                  \
			return result;                                                                                             \
		}                                                                                                              \
	}
	TRY_STMT(queryInit(*pQuery, pCreateInfo));
	TRY_STMT(queryPreprocessPresentQueues(*pQuery));
	TRY_STMT(queryBuildGraph(*pQuery));

	return VK_SUCCESS;
#undef TRY_STMT
}

VkResult vqsPerformQuery(VqsQuery query) {
#define TRY_STMT(stmt)                                                                                                 \
	{                                                                                                                  \
		VkResult result = stmt;                                                                                        \
		if (result != VK_SUCCESS)                                                                                      \
			return result;                                                                                             \
	}
	TRY_STMT(queryMainAlgorithm(query));
	TRY_STMT(queryFetchResults(query));

	return VK_SUCCESS;
#undef TRY_STMT
}

void vqsDestroyQuery(VqsQuery query) {
	if (query != VK_NULL_HANDLE) {
		queryFree(query);
		VQS_FREE(query);
	}
}

void vqsGetQueueSelections(VqsQuery query, VqsQueueSelection *pQueueSelections) {
	querySetQueueFamilyCounters(query);
	for (uint32_t i = 0; i < query->queueRequirementCount; ++i) {
		uint32_t family;
		family = query->resultQueueFamilyIndices[i];
		pQueueSelections[i].queueFamilyIndex = family;
		pQueueSelections[i].queueIndex =
		    (--query->queueFamilyCounters[family]) % query->queueFamilyProperties[family].queueCount;

		if (query->queueRequirements[i].requiredPresentQueueSurface) {
			if (query->resultPresentQueueFamilyIndices[i] != UINT32_MAX) {
				family = query->resultPresentQueueFamilyIndices[i];
				pQueueSelections[i].presentQueueIndex =
				    (--query->queueFamilyCounters[family]) % query->queueFamilyProperties[family].queueCount;
			} else {
				pQueueSelections[i].presentQueueIndex = pQueueSelections[i].queueIndex;
			}
			pQueueSelections[i].presentQueueFamilyIndex = family;
		} else {
			pQueueSelections[i].presentQueueIndex = pQueueSelections[i].presentQueueFamilyIndex = UINT32_MAX;
		}
	}
}

void vqsEnumerateDeviceQueueCreateInfos(VqsQuery query, uint32_t *pDeviceQueueCreateInfoCount,
                                        VkDeviceQueueCreateInfo *pDeviceQueueCreateInfos, uint32_t *pQueuePriorityCount,
                                        float *pQueuePriorities) {
	querySetQueueFamilyCounters(query);

	float **familyPriorityOffsets;
	if (pQueuePriorities) {
		VQS_ALLOC(familyPriorityOffsets, float *, query->queueFamilyCount);
		if (familyPriorityOffsets == NULL)
			return;
	}

	uint32_t infoCounter = 0, priorityCounter = 0;
	for (uint32_t i = 0; i < query->queueFamilyCount; ++i) {
		uint32_t queueCount = query->queueFamilyCounters[i];
		if (queueCount > query->queueFamilyProperties[i].queueCount) {
			queueCount = query->queueFamilyProperties[i].queueCount;
		}
		if (pQueuePriorities)
			familyPriorityOffsets[i] = pQueuePriorities + priorityCounter;
		if (queueCount == 0)
			continue;

		if (pDeviceQueueCreateInfos && pQueuePriorities) {
			VkDeviceQueueCreateInfo createInfo;
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueFamilyIndex = i;
			createInfo.queueCount = queueCount;
			createInfo.pQueuePriorities = familyPriorityOffsets[i];
			createInfo.flags = 0;
			createInfo.pNext = NULL;

			pDeviceQueueCreateInfos[infoCounter] = createInfo;
		}
		priorityCounter += queueCount;
		++infoCounter;
	}

	if (pQueuePriorities) {
		// initialize to 0
		for (uint32_t i = 0; i < priorityCounter; ++i)
			pQueuePriorities[i] = 0.0f;

		for (uint32_t i = 0; i < query->queueRequirementCount; ++i) {
			uint32_t family, queueIndex;
			family = query->resultQueueFamilyIndices[i];
			queueIndex = (--query->queueFamilyCounters[family]) % query->queueFamilyProperties[family].queueCount;

			if (query->queueRequirements[i].priority > familyPriorityOffsets[family][queueIndex]) // set to max priority
				familyPriorityOffsets[family][queueIndex] = query->queueRequirements[i].priority;

			family = query->resultPresentQueueFamilyIndices[i];
			if (family != UINT32_MAX) {
				queueIndex = (--query->queueFamilyCounters[family]) % query->queueFamilyProperties[family].queueCount;

				if (query->queueRequirements[i].priority >
				    familyPriorityOffsets[family][queueIndex]) // set to max priority
					familyPriorityOffsets[family][queueIndex] = query->queueRequirements[i].priority;
			}
		}

		if (familyPriorityOffsets)
			VQS_FREE(familyPriorityOffsets);
	}

	if (pDeviceQueueCreateInfoCount)
		*pDeviceQueueCreateInfoCount = infoCounter;
	if (pQueuePriorityCount)
		*pQueuePriorityCount = priorityCounter;
}

#undef VQS_ALLOC_VK
#undef VQS_ALLOC
#undef VQS_FREE

#endif // VQS_IMPLEMENTATION
