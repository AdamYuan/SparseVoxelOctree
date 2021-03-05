#ifndef UI_LOG_HPP
#define UI_LOG_HPP

#include <spdlog/sinks/ringbuffer_sink.h>

namespace UI {
void LogMenuItem(const std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> &log_sink);
} // namespace UI

#endif
