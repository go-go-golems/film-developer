#pragma once

#include <cstdint>

class ProcessSettingsModel {
public:
    // Push/Pull settings (-2 to +2 stops)
    int8_t push_pull_stops{0};
    static constexpr const char* PUSH_PULL_VALUES[] = {"-2", "-1", "0", "+1", "+2"};
    static constexpr size_t PUSH_PULL_COUNT = 5;

    // Roll count (1-100)
    uint8_t roll_count{1};
    static constexpr uint8_t MIN_ROLL_COUNT = 1;
    static constexpr uint8_t MAX_ROLL_COUNT = 100;

    void reset() {
        push_pull_stops = 0;
        roll_count = 1;
    }

    // Calculate time adjustment factor based on push/pull setting
    float get_time_adjustment() const {
        // Each stop is a doubling/halving of time
        return 1.0f * (1 << push_pull_stops);
    }

    const char* get_push_pull_text() const {
        return PUSH_PULL_VALUES[push_pull_stops + 2];
    }
}; 