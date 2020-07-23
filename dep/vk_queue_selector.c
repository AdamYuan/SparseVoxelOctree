#include "vk_queue_selector.h"

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
	uint32_t cap, cost;
} Edge;
typedef struct Node {
	Edge *head, *path;
	uint32_t dist, flow;
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
	uint32_t leftCount, rightCount, interiorEdgeCount, nodeCount, edgeCount;
	// RESULTS
	uint32_t *resultQueueFamilyIndices, *resultPresentQueueFamilyIndices,
		*queueFamilyCounters;
};

uint32_t queueFlagDist(uint32_t l, uint32_t r, float f) {
	const uint32_t kMask =
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	uint32_t i = (l ^ r) & kMask;
	// popcount
	i = i - ((i >> 1u) & 0x55555555u);
	i = (i & 0x33333333u) + ((i >> 2u) & 0x33333333u);
	i = (((i + (i >> 4u)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24u;
	// multiplied by f
	return i * (uint32_t) (f * 10000.0f);
}

void addEdge(Edge *edge, Edge *edgeRev, Node *from, Node *to, uint32_t cap,
			 uint32_t cost) {
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

	for (uint32_t j = 0; j < query->rightCount; ++j) {
		VkQueueFlags requiredFlags = query->queueRequirements[j].requiredFlags;
		VkSurfaceKHR requiredPresentQueueSurface =
			query->queueRequirements[j].requiredPresentQueueSurface;
		float priority = query->queueRequirements[j].priority;

		for (uint32_t i = 0; i < query->leftCount; ++i) {
			if (query->queueFamilyProperties[i].queueCount == 0)
				continue; // skip empty queue families

			VkQueueFlags flags = query->queueFamilyProperties[i].queueFlags;

			if (requiredPresentQueueSurface &&
				query->resultPresentQueueFamilyIndices[j] == UINT32_MAX) {
				VkBool32 presentSupport;
				if (query->vulkanFunctions.vkGetPhysicalDeviceSurfaceSupportKHR(
					query->physicalDevice, i, requiredPresentQueueSurface,
					&presentSupport) != VK_SUCCESS)
					presentSupport = VK_FALSE;

				if (presentSupport && (flags & requiredFlags) == requiredFlags) {
					if (buildEdge) {
						addEdge(query->pInteriorEdges + counter,
								query->pInteriorEdges + counter + 1, query->pLeftNodes + i,
								query->pRightNodes + j, 1,
								queueFlagDist(flags, requiredFlags, priority));
					}
					counter += 2u;
				}
			} else {
				if ((flags & requiredFlags) == requiredFlags) {
					if (buildEdge) {
						addEdge(query->pInteriorEdges + counter,
								query->pInteriorEdges + counter + 1, query->pLeftNodes + i,
								query->pRightNodes + j, 1,
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
		query->nodes[i].dist = query->nodes[i].flow = UINT32_MAX;
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

uint32_t queryMcmfWithLimits(VqsQuery query, uint32_t flow,
							 uint32_t *leftFlowLimits) {
	for (;;) {
		bool full = true;
		for (uint32_t i = 0; i < query->leftCount; ++i) // append flow
			if (leftFlowLimits[i] > 0) {
				full = false;
				++query->pLeftEdges[i * 2u].cap;
				--leftFlowLimits[i];
			}

		while (querySpfa(query)) {
			uint32_t x = query->pTNode->flow;
			flow += x;

			Node *cur = query->pTNode;
			while (cur != query->pSNode) {
				cur->path->cap -= x;
				cur->path->rev->cap += x;
				cur = cur->path->rev->to;
			}
		}
		if (flow == query->rightCount || full)
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
			(PFN_vkGetPhysicalDeviceQueueFamilyProperties)
			vkGetPhysicalDeviceQueueFamilyProperties;
		query->vulkanFunctions.vkGetPhysicalDeviceSurfaceSupportKHR =
			(PFN_vkGetPhysicalDeviceSurfaceSupportKHR)
			vkGetPhysicalDeviceSurfaceSupportKHR;
#endif
	}

	// SET GRAPH RIGHT COUNT
	query->rightCount = pCreateInfo->queueRequirementCount;

	// GET QUEUE FAMILY PROPERTIES
	query->vulkanFunctions.vkGetPhysicalDeviceQueueFamilyProperties(
		query->physicalDevice, &query->leftCount, NULL);
	if (query->leftCount == 0)
		return VK_ERROR_UNKNOWN;
	query->queueFamilyProperties = (VkQueueFamilyProperties *) malloc(
		query->leftCount * sizeof(VkQueueFamilyProperties));
	if (query->queueFamilyProperties == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	query->vulkanFunctions.vkGetPhysicalDeviceQueueFamilyProperties(
		query->physicalDevice, &query->leftCount, query->queueFamilyProperties);

	// COPY QUEUE REQUIREMENTS
	query->queueRequirements = (VqsQueueRequirements *) malloc(
		query->rightCount * sizeof(VqsQueueRequirements));
	if (query->queueRequirements == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	memcpy(query->queueRequirements, pCreateInfo->pQueueRequirements,
		   query->rightCount * sizeof(VqsQueueRequirements));

	// ALLOC RESULT ARRAYS
	query->resultQueueFamilyIndices =
		(uint32_t *) malloc(query->rightCount * sizeof(uint32_t));
	query->resultPresentQueueFamilyIndices =
		(uint32_t *) malloc(query->rightCount * sizeof(uint32_t));
	if (query->resultQueueFamilyIndices == NULL ||
		query->resultPresentQueueFamilyIndices == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	return VK_SUCCESS;
}

VkResult queryPreprocessPresentQueues(VqsQuery query) {
	// find present queues
	for (uint32_t j = 0; j < query->rightCount; ++j) {
		VkQueueFlags requiredFlags = query->queueRequirements[j].requiredFlags;
		VkSurfaceKHR requiredPresentQueueSurface =
			query->queueRequirements[j].requiredPresentQueueSurface;
		query->resultPresentQueueFamilyIndices[j] = UINT32_MAX;

		if (requiredPresentQueueSurface) { // require present queue
			bool existQueueWithPresentSupport = false;
			for (uint32_t i = 0; i < query->leftCount; ++i) {
				if (query->queueFamilyProperties[i].queueCount == 0)
					continue; // skip empty queue families
				VkQueueFlags flags = query->queueFamilyProperties[i].queueFlags;
				VkBool32 presentSupport;
				if (query->vulkanFunctions.vkGetPhysicalDeviceSurfaceSupportKHR(
					query->physicalDevice, i, requiredPresentQueueSurface,
					&presentSupport) != VK_SUCCESS)
					presentSupport = VK_FALSE;

				if (presentSupport) {
					existQueueWithPresentSupport = true;
					query->resultPresentQueueFamilyIndices[j] = i;

					if ((flags & requiredFlags) == requiredFlags) {
						// exist queue family that both support present and meet
						// requiredFlags leave presentQueueFamilyIndex as UINT32_MAX
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
	query->nodes = (Node *) malloc(query->nodeCount * sizeof(Node));

	query->interiorEdgeCount = queryBuildInteriorEdges(query, false);
	query->edgeCount =
		query->interiorEdgeCount + (query->leftCount + query->rightCount) * 2u;
	query->edges = (Edge *) malloc(query->edgeCount * sizeof(Edge));

	if (query->nodes == NULL || query->edges == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	for (uint32_t i = 0; i < query->nodeCount; ++i) {
		query->nodes[i].head = NULL;
	}
	query->pSNode = query->nodes;
	query->pTNode = query->pSNode + 1;
	query->pLeftNodes = query->pTNode + 1;
	query->pRightNodes = query->pLeftNodes + query->leftCount;

	query->pLeftEdges = query->edges;
	query->pRightEdges = query->pLeftEdges + query->leftCount * 2u;
	query->pInteriorEdges = query->pRightEdges + query->rightCount * 2u;

	// BUILD GRAPH
	for (uint32_t i = 0; i < query->leftCount; ++i) {
		addEdge(query->pLeftEdges + (i * 2), query->pLeftEdges + (i * 2 + 1),
				query->pSNode, query->pLeftNodes + i, 0u, 0u);
	}
	for (uint32_t i = 0; i < query->rightCount; ++i) {
		addEdge(query->pRightEdges + (i * 2), query->pRightEdges + (i * 2 + 1),
				query->pRightNodes + i, query->pTNode, 1u, 0u);
	}
	queryBuildInteriorEdges(query, true);

	return VK_SUCCESS;
}

VkResult queryMainAlgorithm(VqsQuery query) {
	// ALLOC SPFA QUEUE
	query->spfaQueue = (Node **) malloc(query->nodeCount * sizeof(Node *));
	if (query->spfaQueue == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	// RUN MAIN ALGORITHM
	uint32_t *leftFlowLimits =
		(uint32_t *) malloc(query->leftCount * sizeof(uint32_t));
	if (leftFlowLimits == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	for (uint32_t i = 0; i < query->leftCount; ++i)
		leftFlowLimits[i] = query->queueFamilyProperties[i].queueCount;

	uint32_t flow = 0;
	flow = queryMcmfWithLimits(query, flow, leftFlowLimits);
	if (flow < query->rightCount) {
		for (uint32_t i = 0; i < query->leftCount; ++i) {
			uint32_t queueCount = query->queueFamilyProperties[i].queueCount;
			leftFlowLimits[i] =
				query->rightCount > queueCount ? query->rightCount - queueCount : 0u;
		}
		flow = queryMcmfWithLimits(query, flow, leftFlowLimits);

		if (flow < query->rightCount) {
			free(leftFlowLimits);
			return VK_ERROR_UNKNOWN;
		}
	}
	free(leftFlowLimits);
	return VK_SUCCESS;
}

VkResult queryFetchResults(VqsQuery query) {
	// FETCH RESULTS
	for (uint32_t i = 0; i < query->interiorEdgeCount; i += 2) {
		Edge *e = query->pInteriorEdges + i;
		if (e->cap == 0) {
			query->resultQueueFamilyIndices[e->to - query->pRightNodes] =
				e->rev->to - query->pLeftNodes;
		}
	}
	/*for (uint32_t i = 0; i < query->rightCount; ++i) {
	  if (query->queueRequirements[i].requiredPresentQueueSurface &&
	  query->resultPresentQueueFamilyIndices[i] == UINT32_MAX) {
	  query->resultPresentQueueFamilyIndices[i] =
	  query->resultQueueFamilyIndices[i];
	  }
	  }*/

	// ALLOC QUEUE FAMILY COUNTERS
	query->queueFamilyCounters =
		(uint32_t *) malloc(query->leftCount * sizeof(uint32_t));
	if (query->queueFamilyCounters == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	return VK_SUCCESS;
}

void querySetQueueFamilyCounters(VqsQuery query) {
	for (uint32_t i = 0; i < query->leftCount; ++i) {
		query->queueFamilyCounters[i] = 0u;
	}
	for (uint32_t i = 0; i < query->rightCount; ++i) {
		++query->queueFamilyCounters[query->resultQueueFamilyIndices[i]];
		if (query->resultPresentQueueFamilyIndices[i] != UINT32_MAX)
			++query->queueFamilyCounters[query->resultPresentQueueFamilyIndices[i]];
	}
}

void queryFree(VqsQuery query) {
	if (query->nodes)
		free(query->nodes);
	if (query->edges)
		free(query->edges);
	if (query->spfaQueue)
		free(query->spfaQueue);
	if (query->resultQueueFamilyIndices)
		free(query->resultQueueFamilyIndices);
	if (query->resultPresentQueueFamilyIndices)
		free(query->resultPresentQueueFamilyIndices);
	if (query->queueFamilyCounters)
		free(query->queueFamilyCounters);
	if (query->queueFamilyProperties)
		free(query->queueFamilyProperties);
	if (query->queueRequirements)
		free(query->queueRequirements);
}

VkResult vqsCreateQuery(const VqsQueryCreateInfo *pCreateInfo,
						VqsQuery *pQuery) {
	*pQuery = (VqsQuery) malloc(sizeof(struct VqsQuery_T));
	if (*pQuery == NULL)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	VkResult result;
#define TRY_STMT(stmt)                                                         \
    result = stmt;                                                               \
    if (result != VK_SUCCESS) {                                                  \
        vqsDestroyQuery(*pQuery);                                                  \
        return result;                                                             \
    }
	TRY_STMT(queryInit(*pQuery, pCreateInfo));
	TRY_STMT(queryPreprocessPresentQueues(*pQuery));
	TRY_STMT(queryBuildGraph(*pQuery));
	TRY_STMT(queryMainAlgorithm(*pQuery));
	TRY_STMT(queryFetchResults(*pQuery));
#undef TRY_STMT

	return VK_SUCCESS;
}

void vqsDestroyQuery(VqsQuery query) {
	if (query != VK_NULL_HANDLE) {
		queryFree(query);
		free(query);
	}
}

void vqsGetQueueSelections(VqsQuery query,
						   VqsQueueSelection *pQueueSelections) {
	querySetQueueFamilyCounters(query);
	for (uint32_t i = 0; i < query->rightCount; ++i) {
		uint32_t family;
		family = query->resultQueueFamilyIndices[i];
		pQueueSelections[i].queueFamilyIndex = family;
		pQueueSelections[i].queueIndex =
			(--query->queueFamilyCounters[family]) %
			query->queueFamilyProperties[family].queueCount;

		if (query->queueRequirements[i].requiredPresentQueueSurface) {
			if (query->resultPresentQueueFamilyIndices[i] != UINT32_MAX) {
				family = query->resultPresentQueueFamilyIndices[i];
				pQueueSelections[i].presentQueueIndex =
					(--query->queueFamilyCounters[family]) %
					query->queueFamilyProperties[family].queueCount;
			} else {
				pQueueSelections[i].presentQueueIndex = pQueueSelections[i].queueIndex;
			}
			pQueueSelections[i].presentQueueFamilyIndex = family;
		} else {
			pQueueSelections[i].presentQueueIndex =
			pQueueSelections[i].presentQueueFamilyIndex = UINT32_MAX;
		}
	}
}

void vqsEnumerateDeviceQueueCreateInfos(
	VqsQuery query, uint32_t *pDeviceQueueCreateInfoCount,
	VkDeviceQueueCreateInfo *pDeviceQueueCreateInfos,
	uint32_t *pQueuePriorityCount, float *pQueuePriorities) {
	querySetQueueFamilyCounters(query);

	float **familyPriorityOffsets;
	if (pQueuePriorities) {
		familyPriorityOffsets =
			(float **) malloc(query->leftCount * sizeof(float *));
		if (familyPriorityOffsets == NULL)
			return;
	}

	uint32_t infoCounter = 0, priorityCounter = 0;
	for (uint32_t i = 0; i < query->leftCount; ++i) {
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

		for (uint32_t i = 0; i < query->rightCount; ++i) {
			uint32_t family, queueIndex;
			family = query->resultQueueFamilyIndices[i];
			queueIndex = (--query->queueFamilyCounters[family]) %
						 query->queueFamilyProperties[family].queueCount;

			if (query->queueRequirements[i].priority >
				familyPriorityOffsets[family][queueIndex]) // set to max priority
				familyPriorityOffsets[family][queueIndex] =
					query->queueRequirements[i].priority;

			family = query->resultPresentQueueFamilyIndices[i];
			if (family != UINT32_MAX) {
				queueIndex = (--query->queueFamilyCounters[family]) %
							 query->queueFamilyProperties[family].queueCount;

				if (query->queueRequirements[i].priority >
					familyPriorityOffsets[family][queueIndex]) // set to max priority
					familyPriorityOffsets[family][queueIndex] =
						query->queueRequirements[i].priority;
			}
		}

		if (familyPriorityOffsets)
			free(familyPriorityOffsets);
	}

	if (pDeviceQueueCreateInfoCount)
		*pDeviceQueueCreateInfoCount = infoCounter;
	if (pQueuePriorityCount)
		*pQueuePriorityCount = priorityCounter;
}
