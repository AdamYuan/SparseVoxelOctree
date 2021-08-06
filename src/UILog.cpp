#include "UILog.hpp"

#include "Config.hpp"
#include <font-awesome/IconsFontAwesome5.h>
#include <imgui/imgui.h>

namespace UI {
void LogSetRequiredPattern(const std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> &log_sink) {
	log_sink->set_pattern("%H:%M:%S.%e"); // only display time
}

void LogMenuItems(const std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> &log_sink) {
	if (ImGui::BeginMenu("Log")) {
		const auto &logs_raw = log_sink->last_raw();
		const auto &logs_time = log_sink->last_formatted();

		static constexpr ImU32 kLogColors[7] = {0xffffffffu, 0xffffff00u, 0xff00bd00u, 0xff00ffffu,
		                                        0xff0000ffu, 0xff0000ffu, 0xffffffffu};
		static constexpr const char *kLogLevelStrs[7] = {"Trace", "Debug", "Info", "Warn", "Error", "Critical", "Off"};
		static bool log_level_disable[7] = {};

		ImGuiTableFlags flags =
		    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Log Table", 4, flags,
		                      {ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f})) {
			if (ImGui::IsMouseReleased(1))
				ImGui::OpenPopup("Filter");
			if (ImGui::BeginPopup("Filter")) {
				for (uint32_t i = 0; i < 6; ++i) {
					if (ImGui::MenuItem(kLogLevelStrs[i], nullptr, !log_level_disable[i]))
						log_level_disable[i] ^= 1;
				}
				ImGui::EndPopup();
			}

			ImGui::TableSetupColumn("Time");
			ImGui::TableSetupColumn("Level");
			ImGui::TableSetupColumn("Thread");
			ImGui::TableSetupColumn("Text");
			ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
			ImGui::TableHeadersRow();

			for (uint32_t i = 0; i < logs_raw.size(); ++i) {
				const auto &log = logs_raw[i];
				if (log_level_disable[log.level])
					continue;

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Selectable(logs_time[i].c_str(), false,
				                  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);

				ImGui::PushID(i);
				if (ImGui::BeginPopupContextItem("Copy")) {
					if (ImGui::MenuItem(ICON_FA_COPY " Copy"))
						ImGui::SetClipboardText(std::string{log.payload.begin(), log.payload.end()}.c_str());
					ImGui::Separator();
					for (uint32_t i = 0; i < 6; ++i) {
						if (ImGui::MenuItem(kLogLevelStrs[i], nullptr, !log_level_disable[i]))
							log_level_disable[i] ^= 1;
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();

				ImGui::TableSetColumnIndex(1);
				ImGui::PushStyleColor(ImGuiCol_Text, kLogColors[log.level]);
				ImGui::TextUnformatted(kLogLevelStrs[log.level]);
				ImGui::PopStyleColor();

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%lu", log.thread_id);

				ImGui::TableSetColumnIndex(3);
				ImGui::TextUnformatted(log.payload.begin(), log.payload.end());
			}

			ImGui::EndTable();
		}
		ImGui::EndMenu();
	}
}
} // namespace UI
