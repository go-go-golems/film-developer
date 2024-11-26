#ifdef HOST
#include "debug.hpp"

// Global variables for host logging configuration
LogLevel g_log_level = LogLevelInfo;  // Default to INFO level
const char* g_tag_filter = nullptr;   // No tag filtering by default

void set_log_level(LogLevel level) {
    g_log_level = level;
}

void set_tag_filter(const char* tag) {
    g_tag_filter = tag;
}
#endif 