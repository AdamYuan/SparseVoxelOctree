#include "UIOctreeTracer.hpp"

#include "ImGuiUtil.hpp"
#include <font-awesome/IconsFontAwesome5.h>

namespace UI {
void OctreeTracerMenuItems(const std::shared_ptr<OctreeTracer> &octree_tracer) {
	if (ImGui::BeginMenu("View")) {
		if (ImGui::MenuItem("Diffuse", nullptr, octree_tracer->m_view_type == OctreeTracer::ViewTypes::kDiffuse))
			octree_tracer->m_view_type = OctreeTracer::ViewTypes::kDiffuse;
		if (ImGui::MenuItem("Normal", nullptr, octree_tracer->m_view_type == OctreeTracer::ViewTypes::kNormal))
			octree_tracer->m_view_type = OctreeTracer::ViewTypes::kNormal;
		if (ImGui::MenuItem("Iterations", nullptr, octree_tracer->m_view_type == OctreeTracer::ViewTypes::kIteration))
			octree_tracer->m_view_type = OctreeTracer::ViewTypes::kIteration;

		ImGui::Separator();

		ImGui::Checkbox("Beam Optimization", &octree_tracer->m_beam_enable);
		ImGui::EndMenu();
	}
}
void OctreeTracerRightStatus(const std::shared_ptr<OctreeTracer> &octree_tracer) {
	float indent_w = ImGui::GetWindowContentRegionWidth(), spacing = ImGui::GetStyle().ItemSpacing.x;
	char buf[128];

	const auto &octree = octree_tracer->GetOctreePtr();

	sprintf(buf, "FPS %.1f", ImGui::GetIO().Framerate);
	indent_w -= ImGui::CalcTextSize(buf).x;
	ImGui::SameLine(indent_w);
	ImGui::TextUnformatted(buf);

	indent_w -= spacing;
	ImGui::SameLine(indent_w);
	ImGui::Separator();

	sprintf(buf, ICON_FA_CUBE " %d", octree->GetLevel());
	indent_w -= ImGui::CalcTextSize(buf).x + spacing;
	ImGui::SameLine(indent_w);
	ImGui::TextUnformatted(buf);

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Octree Level");
		ImGui::EndTooltip();
	}

	sprintf(buf, ICON_FA_DATABASE " %.0f/%.0f MB", octree->GetRange() / 1000000.0f,
	        octree->GetBuffer()->GetSize() / 1000000.0f);
	indent_w -= ImGui::CalcTextSize(buf).x + spacing;
	ImGui::SameLine(indent_w);
	ImGui::TextUnformatted(buf);

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Octree Used Size / Allocated Size");
		ImGui::EndTooltip();
	}
}
} // namespace UI
