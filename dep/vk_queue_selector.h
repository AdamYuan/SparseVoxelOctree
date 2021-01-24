// vk_queue_selector - v1.0.3 - https://github.com/AdamYuan/
//
// Use this in *one* source file
//   #define VQS_IMPLEMENTATION
//   #include "vk_queue_selector.h"
//
// version 1.0.3 (2021-01-24) separate vqs__BinaryGraph from VqsQuery_T
//         1.0.2 (2021-01-24) name private struct and func with "vqs__" prefix
//         1.0.1 (2021-01-24) move all graph-related things to vqsPerformQuery
//                            fix leaks and memory errors
//         1.0.0 (2021-01-24)
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

struct vqs__Node;

typedef struct vqs__Edge {
	struct vqs__Node *to;
	struct vqs__Edge *next, *rev;
	int32_t cap, cost;
} vqs__Edge;

typedef struct vqs__Node {
	vqs__Edge *head, *path;
	int32_t dist, flow;
	bool inQueue;
} vqs__Node;

typedef struct vqs__BinaryGraph {
	vqs__Node *nodes, *pLeftNodes, *pRightNodes, *pSNode, *pTNode, **spfaQueue;
	vqs__Edge *edges, *pInteriorEdges, *pLeftEdges, *pRightEdges;
	int32_t *minInteriorCosts;                 // the min interior edge cost for each left nodes
	uint32_t leftCount, rightCount, nodeCount; // leftCount = queueFamilyCount, rightCount = queueRequirementCount
	uint32_t interiorEdgeCount, edgeCount;
} vqs__BinaryGraph;

struct VqsQuery_T {
	// BASICS
	VkPhysicalDevice physicalDevice;
	VqsVulkanFunctions vulkanFunctions;
	VkQueueFamilyProperties *queueFamilyProperties;
	VqsQueueRequirements *queueRequirements;
	uint32_t queueFamilyCount, queueRequirementCount;
	// RESULTS
	uint32_t *resultQueueFamilyIndices, *resultPresentQueueFamilyIndices, *queueFamilyCounters;
};

#define VQS_FREE(x)                                                                                                    \
	{                                                                                                                  \
		free(x);                                                                                                       \
		(x) = NULL;                                                                                                    \
	}
#define VQS_ALLOC(x, type, count)                                                                                      \
	{ (x) = (type *)calloc(count, sizeof(type)); }
#define VQS_ALLOC_VK(x, type, count)                                                                                   \
	{                                                                                                                  \
		VQS_ALLOC(x, type, count);                                                                                     \
		if (x == NULL)                                                                                                 \
			return VK_ERROR_OUT_OF_HOST_MEMORY;                                                                        \
	}

static uint32_t vqs__queueFlagDist(uint32_t l, uint32_t r, float f) {
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

static void vqs__addEdge(vqs__Edge *edge, vqs__Edge *edgeRev, vqs__Node *from, vqs__Node *to, int32_t cap,
                         int32_t cost) {
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

////////////////////////////////
// vqs__BinaryGraph functions //
////////////////////////////////
static bool vqs__graphSpfa(vqs__BinaryGraph *graph) {
	for (uint32_t i = 0; i < graph->nodeCount; ++i) {
		graph->nodes[i].dist = INT32_MAX;
		graph->nodes[i].flow = INT32_MAX;
		graph->nodes[i].inQueue = false;
		graph->nodes[i].path = NULL;
	}

	uint32_t queTop = 0, queTail = 0;
	const uint32_t kQueSize = graph->nodeCount;
#define QUE_PUSH(x) graph->spfaQueue[(queTop++) % kQueSize] = (x)
#define QUE_POP graph->spfaQueue[(queTail++) % kQueSize]
	graph->pSNode->inQueue = true;
	graph->pSNode->dist = 0;
	QUE_PUSH(graph->pSNode);

	while (queTop != queTail) {
		vqs__Node *cur = QUE_POP;
		cur->inQueue = false;

		for (vqs__Edge *e = cur->head; e; e = e->next) {
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

	return graph->pTNode->path;
}

static int32_t vqs__graphMcmfWithLimits(vqs__BinaryGraph *graph, int32_t flow, uint32_t *leftFlowLimits) {
	for (;;) {
		{
			bool full = true;
			int32_t minCost = INT32_MAX;
			for (uint32_t i = 0; i < graph->leftCount; ++i) { // append flow
				if (leftFlowLimits[i] > 0) {
					full = false;
					++graph->pLeftEdges[i * 2u].cap;
					if (graph->minInteriorCosts[i] < minCost)
						minCost = graph->minInteriorCosts[i];
					--leftFlowLimits[i];
				}
			}
			for (uint32_t i = 0; i < graph->interiorEdgeCount; i += 2) { // cancel negative rings
				vqs__Edge *e = graph->pInteriorEdges + i;
				if (e->cost > minCost)
					e->rev->cap = 0;
			}
			if (full)
				return flow;
		}

		while (vqs__graphSpfa(graph)) {
			int32_t x = graph->pTNode->flow;
			flow += x;

			vqs__Node *cur = graph->pTNode;
			while (cur != graph->pSNode) {
				cur->path->cap -= x;
				cur->path->rev->cap += x;
				cur = cur->path->rev->to;
			}
		}
		if (flow == graph->rightCount)
			return flow;
	}
}

static uint32_t vqs__graphBuildInteriorEdges(vqs__BinaryGraph *graph, const VqsQuery query, bool buildEdge) {
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
						vqs__addEdge(graph->pInteriorEdges + counter, graph->pInteriorEdges + counter + 1,
						             graph->pLeftNodes + i, graph->pRightNodes + j, 1,
						             vqs__queueFlagDist(flags, requiredFlags, priority));
					}
					counter += 2u;
				}
			} else {
				if ((flags & requiredFlags) == requiredFlags) {
					if (buildEdge) {
						vqs__addEdge(graph->pInteriorEdges + counter, graph->pInteriorEdges + counter + 1,
						             graph->pLeftNodes + i, graph->pRightNodes + j, 1,
						             vqs__queueFlagDist(flags, requiredFlags, priority));
					}
					counter += 2u;
				}
			}
		}
	}
	return counter;
}

static VkResult vqs__graphMainAlgorithm(vqs__BinaryGraph *graph, const VqsQuery query) {
	// RUN MAIN ALGORITHM
	uint32_t *leftFlowLimits = NULL;
	VQS_ALLOC_VK(leftFlowLimits, uint32_t, graph->leftCount);

	for (uint32_t i = 0; i < graph->leftCount; ++i)
		leftFlowLimits[i] = query->queueFamilyProperties[i].queueCount;

	int32_t flow = 0;
	flow = vqs__graphMcmfWithLimits(graph, flow, leftFlowLimits);
	if (flow < graph->rightCount) {
		for (uint32_t i = 0; i < graph->leftCount; ++i) {
			uint32_t queueCount = query->queueFamilyProperties[i].queueCount;
			leftFlowLimits[i] = graph->rightCount > queueCount ? graph->rightCount - queueCount : 0u;
		}
		flow = vqs__graphMcmfWithLimits(graph, flow, leftFlowLimits);

		if (flow < graph->rightCount) {
			VQS_FREE(leftFlowLimits);
			return VK_ERROR_UNKNOWN;
		}
	}
	VQS_FREE(leftFlowLimits);
	return VK_SUCCESS;
}

static VkResult vqs__graphInit(vqs__BinaryGraph *graph, const VqsQuery query) {
	// Set leftCount and rightCount
	graph->leftCount = query->queueFamilyCount;
	graph->rightCount = query->queueRequirementCount;

	// ALLOC GRAPH
	graph->nodeCount = graph->leftCount + graph->rightCount + 2u;
	VQS_ALLOC_VK(graph->nodes, vqs__Node, graph->nodeCount);

	graph->interiorEdgeCount = vqs__graphBuildInteriorEdges(graph, query, false);
	graph->edgeCount = graph->interiorEdgeCount + (graph->leftCount + graph->rightCount) * 2u;
	VQS_ALLOC_VK(graph->edges, vqs__Edge, graph->edgeCount);

	// not needed since calloc is used
	/*for (uint32_t i = 0; i < query->nodeCount; ++i) {
	    query->nodes[i].head = NULL;
	}*/
	graph->pSNode = graph->nodes;
	graph->pTNode = graph->pSNode + 1;
	graph->pLeftNodes = graph->pTNode + 1;
	graph->pRightNodes = graph->pLeftNodes + graph->leftCount;

	graph->pLeftEdges = graph->edges;
	graph->pRightEdges = graph->pLeftEdges + graph->leftCount * 2u;
	graph->pInteriorEdges = graph->pRightEdges + graph->rightCount * 2u;

	// BUILD GRAPH
	for (uint32_t i = 0; i < graph->leftCount; ++i) { // S -> LEFT NODES
		vqs__addEdge(graph->pLeftEdges + (i * 2), graph->pLeftEdges + (i * 2 + 1), graph->pSNode, graph->pLeftNodes + i,
		             0, 0);
	}
	for (uint32_t i = 0; i < graph->rightCount; ++i) { // RIGHT NODES -> T
		vqs__addEdge(graph->pRightEdges + (i * 2), graph->pRightEdges + (i * 2 + 1), graph->pRightNodes + i,
		             graph->pTNode, 1, 0);
	}
	vqs__graphBuildInteriorEdges(graph, query, true);

	// SET MIN INTERIOR COSTS (for negative ring canceling)
	VQS_ALLOC_VK(graph->minInteriorCosts, int32_t, graph->leftCount);
	for (uint32_t i = 0; i < graph->leftCount; ++i) {
		graph->minInteriorCosts[i] = INT32_MAX;
	}

	for (uint32_t i = 0; i < graph->interiorEdgeCount; i += 2) {
		vqs__Edge *e = graph->pInteriorEdges + i;
		int32_t *target = graph->minInteriorCosts + (e->rev->to - graph->pLeftNodes);
		if (e->cost < *target)
			*target = e->cost;
	}

	// ALLOC SPFA QUEUE
	VQS_ALLOC_VK(graph->spfaQueue, vqs__Node *, graph->nodeCount);

	return VK_SUCCESS;
}

static void vqs__graphFree(vqs__BinaryGraph *graph) {
	VQS_FREE(graph->nodes);
	VQS_FREE(graph->edges);
	VQS_FREE(graph->minInteriorCosts);
	VQS_FREE(graph->spfaQueue);
}

//////////////////////////
// VqsQuery_T functions //
//////////////////////////
static VkResult vqs__queryPreprocessPresentQueues(VqsQuery query) {
	// FIND PRESENT QUEUES
	for (uint32_t j = 0; j < query->queueRequirementCount; ++j) {
		VkQueueFlags requiredFlags = query->queueRequirements[j].requiredFlags;
		VkSurfaceKHR requiredPresentQueueSurface = query->queueRequirements[j].requiredPresentQueueSurface;
		query->resultPresentQueueFamilyIndices[j] = UINT32_MAX;

		if (requiredPresentQueueSurface) { // require present queue
			bool existQueueWithPresentSupport = false;
			for (uint32_t i = 0; i < query->queueFamilyCount; ++i) {
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

static VkResult vqs__queryFetchResults(VqsQuery query, const vqs__BinaryGraph *graph) {
	// FETCH RESULTS
	for (uint32_t i = 0; i < graph->interiorEdgeCount; i += 2) {
		vqs__Edge *e = graph->pInteriorEdges + i;
		if (e->cap == 0) {
			query->resultQueueFamilyIndices[e->to - graph->pRightNodes] = e->rev->to - graph->pLeftNodes;
		}
	}

	return VK_SUCCESS;
}

static void vqs__querySetQueueFamilyCounters(VqsQuery query) {
	for (uint32_t i = 0; i < query->queueFamilyCount; ++i) {
		query->queueFamilyCounters[i] = 0u;
	}
	for (uint32_t i = 0; i < query->queueRequirementCount; ++i) {
		++query->queueFamilyCounters[query->resultQueueFamilyIndices[i]];
		if (query->resultPresentQueueFamilyIndices[i] != UINT32_MAX)
			++query->queueFamilyCounters[query->resultPresentQueueFamilyIndices[i]];
	}
}

static VkResult vqs__queryInit(VqsQuery query, const VqsQueryCreateInfo *pCreateInfo) {
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

static void vqs__queryFree(VqsQuery query) {
	VQS_FREE(query->resultQueueFamilyIndices);
	VQS_FREE(query->resultPresentQueueFamilyIndices);
	VQS_FREE(query->queueFamilyCounters);
	VQS_FREE(query->queueFamilyProperties);
	VQS_FREE(query->queueRequirements);
}

/////////////////////
// API definitions //
/////////////////////
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
	TRY_STMT(vqs__queryInit(*pQuery, pCreateInfo));
	TRY_STMT(vqs__queryPreprocessPresentQueues(*pQuery));
#undef TRY_STMT

	return VK_SUCCESS;
}

VkResult vqsPerformQuery(VqsQuery query) {
	vqs__BinaryGraph *graph = NULL;
	VQS_ALLOC_VK(graph, vqs__BinaryGraph, 1);

#define TRY_STMT(stmt)                                                                                                 \
	{                                                                                                                  \
		VkResult result = stmt;                                                                                        \
		if (result != VK_SUCCESS)                                                                                      \
			return result;                                                                                             \
	}
	TRY_STMT(vqs__graphInit(graph, query));
	TRY_STMT(vqs__graphMainAlgorithm(graph, query));
	TRY_STMT(vqs__queryFetchResults(query, graph));
#undef TRY_STMT

	vqs__graphFree(graph);
	VQS_FREE(graph);

	return VK_SUCCESS;
}

void vqsDestroyQuery(VqsQuery query) {
	if (query != VK_NULL_HANDLE) {
		vqs__queryFree(query);
		VQS_FREE(query);
	}
}

void vqsGetQueueSelections(VqsQuery query, VqsQueueSelection *pQueueSelections) {
	vqs__querySetQueueFamilyCounters(query);
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
	vqs__querySetQueueFamilyCounters(query);

	float **familyPriorityOffsets = NULL;
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
