#ifndef COMPUTE_PIPELINE_HPP
#define COMPUTE_PIPELINE_HPP

#include "PipelineBase.hpp"
#include "ShaderModule.hpp"

namespace myvk {
class ComputePipeline : public PipelineBase {
public:
	static std::shared_ptr<ComputePipeline> Create(const std::shared_ptr<PipelineLayout> &pipeline_layout,
	                                               const VkComputePipelineCreateInfo &create_info);

	static std::shared_ptr<ComputePipeline> Create(const std::shared_ptr<PipelineLayout> &pipeline_layout,
	                                               const std::shared_ptr<ShaderModule> &shader_module);

	VkPipelineBindPoint GetBindPoint() const override { return VK_PIPELINE_BIND_POINT_COMPUTE; }
};
} // namespace myvk

#endif
