#pragma once

#include <stdio.h>
#include <string.h>

// Log levels
typedef enum {
    LogLevelNone = 0,
    LogLevelError = 1,
    LogLevelWarn = 2,
    LogLevelInfo = 3,
    LogLevelDebug = 4,
    LogLevelTrace = 5,
} LogLevel;

#ifdef HOST
// Host system logging implementation

// Global log level and tag filter
extern LogLevel g_log_level;
extern const char* g_tag_filter;

// Log level control
void set_log_level(LogLevel level);
void set_tag_filter(const char* tag);

// ANSI color codes for host logging
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_PURPLE  "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Host logging macros
#define FURI_LOG_E(tag, fmt, ...) do { \
    if(g_log_level >= LogLevelError && (!g_tag_filter || strcmp(g_tag_filter, tag) == 0)) { \
        printf(ANSI_COLOR_RED "[E][%s] " fmt ANSI_COLOR_RESET "\n", tag, ##__VA_ARGS__); \
    } \
} while(0)

#define FURI_LOG_W(tag, fmt, ...) do { \
    if(g_log_level >= LogLevelWarn && (!g_tag_filter || strcmp(g_tag_filter, tag) == 0)) { \
        printf(ANSI_COLOR_YELLOW "[W][%s] " fmt ANSI_COLOR_RESET "\n", tag, ##__VA_ARGS__); \
    } \
} while(0)

#define FURI_LOG_I(tag, fmt, ...) do { \
    if(g_log_level >= LogLevelInfo && (!g_tag_filter || strcmp(g_tag_filter, tag) == 0)) { \
        printf("[I][%s] " fmt "\n", tag, ##__VA_ARGS__); \
    } \
} while(0)

#define FURI_LOG_D(tag, fmt, ...) do { \
    if(g_log_level >= LogLevelDebug && (!g_tag_filter || strcmp(g_tag_filter, tag) == 0)) { \
        printf("[D][%s] " fmt "\n", tag, ##__VA_ARGS__); \
    } \
} while(0)

#define FURI_LOG_T(tag, fmt, ...) do { \
    if(g_log_level >= LogLevelTrace && (!g_tag_filter || strcmp(g_tag_filter, tag) == 0)) { \
        printf(ANSI_COLOR_PURPLE "[T][%s] " fmt ANSI_COLOR_RESET "\n", tag, ##__VA_ARGS__); \
    } \
} while(0)

// Raw logging macros for host
#define FURI_LOG_RAW_E(fmt, ...) do { \
    if(g_log_level >= LogLevelError) { \
        printf(ANSI_COLOR_RED fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__); \
    } \
} while(0)

#define FURI_LOG_RAW_W(fmt, ...) do { \
    if(g_log_level >= LogLevelWarn) { \
        printf(ANSI_COLOR_YELLOW fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__); \
    } \
} while(0)

#else
// Flipper Zero device logging implementation
#include <furi.h>
#endif 