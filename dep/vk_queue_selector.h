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
	PFN_vkGetPhysicalDeviceQueueFamilyProperties
		vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
} VqsVulkanFunctions;

typedef struct VqsQueueRequirements {
	VkQueueFlags requiredFlags;
	float priority; // should be 1.0f as default
	VkSurfaceKHR
		requiredPresentQueueSurface; // set to NULL if don't need present queue
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

VQS_API VkResult vqsCreateQuery(const VqsQueryCreateInfo *pCreateInfo,
								VqsQuery *pQuery);

VQS_API void vqsDestroyQuery(VqsQuery query);

VQS_API void vqsGetQueueSelections(VqsQuery query,
								   VqsQueueSelection *pQueueSelections);

VQS_API void vqsEnumerateDeviceQueueCreateInfos(
	VqsQuery query, uint32_t *pDeviceQueueCreateInfoCount,
	VkDeviceQueueCreateInfo *pDeviceQueueCreateInfos,
	uint32_t *pQueuePriorityCount, float *pQueuePriorities);

#ifdef __cplusplus
}
#endif

#endif
