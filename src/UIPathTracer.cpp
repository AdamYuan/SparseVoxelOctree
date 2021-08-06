#include "UIPathTracer.hpp"

#include "Config.hpp"
#include "ImGuiUtil.hpp"
#include <font-awesome/IconsFontAwesome5.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <spdlog/spdlog.h>
#include <tinyexr.h>

namespace UI {
void PathTracerStartButton(const std::shared_ptr<PathTracerThread> &path_tracer_thread, const char **open_modal) {
	if (ImGui::MenuItem(ICON_FA_PLAY)) {
		*open_modal = kPathTracerStartModal;
	}
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Start Path Tracer");
		ImGui::EndTooltip();
	}
}
void PathTracerControlButtons(const std::shared_ptr<PathTracerThread> &path_tracer_thread, const char **open_modal) {
	if (ImGui::MenuItem(ICON_FA_FILE_EXPORT)) {
		*open_modal = kPathTracerExportEXRModal;
	}
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Export OpenEXR");
		ImGui::EndTooltip();
	}

	if (ImGui::MenuItem(ICON_FA_STOP)) {
		*open_modal = kPathTracerStopModal;
	}
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Stop Path Tracer");
		ImGui::EndTooltip();
	}

	if (path_tracer_thread->IsPause()) {
		if (ImGui::MenuItem(ICON_FA_PLAY)) {
			path_tracer_thread->SetPause(false);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Resume Path Tracer");
			ImGui::EndTooltip();
		}
	} else {
		if (ImGui::MenuItem(ICON_FA_PAUSE)) {
			path_tracer_thread->SetPause(true);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Pause Path Tracer");
			ImGui::EndTooltip();
		}
	}
}
void PathTracerMenuItems(const std::shared_ptr<PathTracerThread> &path_tracer_thread) {
	const auto &path_tracer_viewer = path_tracer_thread->GetPathTracerViewerPtr();
	if (ImGui::BeginMenu("Channel")) {
		if (ImGui::MenuItem("Color", nullptr, path_tracer_viewer->m_view_type == PathTracerViewer::ViewTypes::kColor)) {
			path_tracer_viewer->m_view_type = PathTracerViewer::ViewTypes::kColor;
			path_tracer_thread->UpdateViewer();
		}
		if (ImGui::MenuItem("Albedo", nullptr,
		                    path_tracer_viewer->m_view_type == PathTracerViewer::ViewTypes::kAlbedo)) {
			path_tracer_viewer->m_view_type = PathTracerViewer::ViewTypes::kAlbedo;
			path_tracer_thread->UpdateViewer();
		}
		if (ImGui::MenuItem("Normal", nullptr,
		                    path_tracer_viewer->m_view_type == PathTracerViewer::ViewTypes::kNormal)) {
			path_tracer_viewer->m_view_type = PathTracerViewer::ViewTypes::kNormal;
			path_tracer_thread->UpdateViewer();
		}

		ImGui::EndMenu();
	}
}
void PathTracerRightStatus(const std::shared_ptr<PathTracerThread> &path_tracer_thread) {
	float indent_w = ImGui::GetWindowContentRegionWidth(), spacing = ImGui::GetStyle().ItemSpacing.x;
	char buf[128];

	sprintf(buf, "SPP %u", path_tracer_thread->GetSPP());
	indent_w -= ImGui::CalcTextSize(buf).x;
	ImGui::SameLine(indent_w);
	ImGui::TextUnformatted(buf);

	indent_w -= spacing;
	ImGui::SameLine(indent_w);
	ImGui::Separator();

	sprintf(buf, ICON_FA_IMAGE " %ux%u", path_tracer_thread->GetPathTracerViewerPtr()->GetPathTracerPtr()->m_width,
	        path_tracer_thread->GetPathTracerViewerPtr()->GetPathTracerPtr()->m_height);
	indent_w -= ImGui::CalcTextSize(buf).x + spacing;
	ImGui::SameLine(indent_w);
	ImGui::TextUnformatted(buf);

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Resolution");
		ImGui::EndTooltip();
	}

	sprintf(buf, ICON_FA_BOLT " %u", path_tracer_thread->GetPathTracerViewerPtr()->GetPathTracerPtr()->m_bounce);
	indent_w -= ImGui::CalcTextSize(buf).x + spacing;
	ImGui::SameLine(indent_w);
	ImGui::TextUnformatted(buf);

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Bounce");
		ImGui::EndTooltip();
	}

	sprintf(buf, ICON_FA_STOPWATCH " %u sec", uint32_t(path_tracer_thread->GetRenderTime()));
	indent_w -= ImGui::CalcTextSize(buf).x + spacing;
	ImGui::SameLine(indent_w);
	ImGui::TextUnformatted(buf);

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Render Time");
		ImGui::EndTooltip();
	}

	if (path_tracer_thread->GetPathTracerQueue()->GetFamilyIndex() ==
	    path_tracer_thread->GetMainQueue()->GetFamilyIndex()) {
		indent_w -= ImGui::CalcTextSize(ICON_FA_EXCLAMATION_TRIANGLE).x + spacing;
		ImGui::SameLine(indent_w);
		ImGui::TextUnformatted(ICON_FA_EXCLAMATION_TRIANGLE);

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Async path tracing queue not available");
			ImGui::EndTooltip();
		}
	}
}
void PathTracerStartModal(const std::shared_ptr<PathTracerThread> &path_tracer_thread) {
	ImGui::SetNextWindowCentering();
	if (ImGui::BeginPopupModal(kPathTracerStartModal, nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		const auto &path_tracer = path_tracer_thread->GetPathTracerViewerPtr()->GetPathTracerPtr();

		int width = path_tracer->m_width;
		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		if (ImGui::DragInt("Width", &width, 1, kMinWidth, kMaxWidth))
			path_tracer->m_width = width;
		ImGui::PopItemWidth();
		ImGui::SameLine();
		int height = path_tracer->m_height;
		if (ImGui::DragInt("Height", &height, 1, kMinHeight, kMaxHeight))
			path_tracer->m_height = height;
		ImGui::PopItemWidth();

		int bounce = path_tracer->m_bounce;
		if (ImGui::DragInt("Bounce", &bounce, 1, kMinBounce, kMaxBounce))
			path_tracer->m_bounce = bounce;

		float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

		if (ImGui::Button("Start", {button_width, 0})) {
			path_tracer_thread->Launch();

			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", {button_width, 0}))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
void PathTracerStopModal(const std::shared_ptr<PathTracerThread> &path_tracer_thread) {
	ImGui::SetNextWindowCentering();
	if (ImGui::BeginPopupModal(kPathTracerStopModal, nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		ImGui::Text("Are you sure?\nThe render result might be lost.");

		float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

		if (ImGui::Button("Stop", {button_width, 0})) {
			path_tracer_thread->StopAndJoin();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", {button_width, 0}))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
void PathTracerExportEXRModal(const std::shared_ptr<PathTracerThread> &path_tracer_thread) {
	ImGui::SetNextWindowCentering();
	if (ImGui::BeginPopupModal(kPathTracerExportEXRModal, nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		const auto &path_tracer = path_tracer_thread->GetPathTracerViewerPtr()->GetPathTracerPtr();

		constexpr const char *kChannels[] = {"Color", "Albedo", "Normal"};
		static const char *const *current_channel = kChannels + 0;
		if (ImGui::BeginCombo("Channel", *current_channel)) {
			for (int n = 0; n < IM_ARRAYSIZE(kChannels); n++) {
				bool is_selected = (current_channel == kChannels + n);
				if (ImGui::Selectable(kChannels[n], is_selected))
					current_channel = kChannels + n;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		static char exr_name_buf[kFilenameBufSize]{};
		static bool save_as_fp16{false};

		constexpr const char *kFilter[] = {"*.exr"};
		ImGui::FileSave("OpenEXR Filename", "...", exr_name_buf, kFilenameBufSize, "Export OpenEXR", 1, kFilter);

		ImGui::Checkbox("Export as FP16", &save_as_fp16);

		float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

		{
			if (ImGui::Button("Export", {button_width, 0})) {
				//bool tmp_pause = path_tracer_thread->IsPause();
				//path_tracer_thread->SetPause(true);

				std::vector<float> pixels;
				{
					std::shared_ptr<myvk::CommandPool> command_pool =
					    myvk::CommandPool::Create(path_tracer_thread->GetPathTracerQueue());

					if (current_channel == kChannels + 0) // color
						pixels = path_tracer->ExtractColorImage(command_pool);
					else if (current_channel == kChannels + 1) // albedo
						pixels = path_tracer->ExtractAlbedoImage(command_pool);
					else // normal
						pixels = path_tracer->ExtractNormalImage(command_pool);
				}

				//path_tracer_thread->SetPause(tmp_pause);

				char *err{nullptr};
				if (SaveEXR(pixels.data(), path_tracer->m_width, path_tracer->m_height, 3, save_as_fp16, exr_name_buf,
				            (const char **)&err) < 0)
					spdlog::error("{}", err);
				else
					spdlog::info("Saved EXR image to {}", exr_name_buf);
				free(err);

				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", {button_width, 0})) {
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}
} // namespace UI
