#pragma once

#include <cstdint>

namespace sigil::vm {

enum class log_level_t : uint8_t {
    TRACE, DEBUG, INFO, WARN, ERROR, FATAL
};

struct log_sink_t {
    void (*write)(log_level_t, const char* msg, void* userdata);
    void* userdata;
};

void log_init(log_sink_t sink);
void log_write(log_level_t lvl, const char* fmt, ...);
void log_shutdown();

} // namespace sigil::vm
