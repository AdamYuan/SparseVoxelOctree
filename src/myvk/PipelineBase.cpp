#include "PipelineBase.hpp"

namespace myvk {
PipelineBase::~PipelineBase() {
	if (m_pipeline)
		vkDestroyPipeline(m_pipeline_layout_ptr->GetDevicePtr()->GetHandle(), m_pipeline, nullptr);
}
} // namespace myvk
