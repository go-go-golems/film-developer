#pragma once

#ifdef HOST
#include <string>
#include <cstring>

/**
 * @brief Host system implementation of FuriString
 * Provides a minimal string implementation compatible with Flipper Zero's FuriString
 */
class FuriString {
public:
    FuriString(const char* str = nullptr) {
        if (str) {
            data = str;
        }
    }

    const char* getData() const {
        return data.c_str();
    }

    void set(const char* str) {
        if (str) {
            data = str;
        } else {
            data.clear();
        }
    }

    void cat(const char* str) {
        if (str) {
            data += str;
        }
    }

    size_t size() const {
        return data.size();
    }

private:
    std::string data;
};

/**
 * @brief Host system implementations of FuriString allocation functions
 */
inline FuriString* furi_string_alloc() {
    return new FuriString();
}

inline void furi_string_free(FuriString* str) {
    delete str;
}

inline FuriString* furi_string_alloc_set(const char* str) {
    return new FuriString(str);
}

inline void furi_string_cat(FuriString* dest, const char* src) {
    dest->cat(src);
}

inline void furi_string_set(FuriString* dest, const char* src) {
    dest->set(src);
}

/**
 * @brief Host system implementation of Flipper Zero's delay function
 */
inline void furi_delay_ms(uint32_t ms) {
    // Use platform-specific sleep function
    #ifdef _WIN32
        Sleep(ms);
    #else
        usleep(ms * 1000);
    #endif
}

/**
 * @brief Host system mutex implementation
 */
class FuriMutex {
public:
    FuriMutex() {
        #ifdef _WIN32
            InitializeCriticalSection(&cs);
        #else
            pthread_mutex_init(&mutex, NULL);
        #endif
    }

    ~FuriMutex() {
        #ifdef _WIN32
            DeleteCriticalSection(&cs);
        #else
            pthread_mutex_destroy(&mutex);
        #endif
    }

    void acquire() {
        #ifdef _WIN32
            EnterCriticalSection(&cs);
        #else
            pthread_mutex_lock(&mutex);
        #endif
    }

    void release() {
        #ifdef _WIN32
            LeaveCriticalSection(&cs);
        #else
            pthread_mutex_unlock(&mutex);
        #endif
    }

private:
    #ifdef _WIN32
        CRITICAL_SECTION cs;
    #else
        pthread_mutex_t mutex;
    #endif
};
#else 
#include <furi.h>
#include <furi_hal_gpio.h>

#endif // HOST 