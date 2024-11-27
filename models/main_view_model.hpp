#pragma once

#include "../agitation_process_interpreter.hpp"
#include "../agitation_processes.hpp"
#include "../motor_controller.hpp"
#include "guard.hpp"
#include <cstdint>

class MainViewModel {
public:
    // Process state
    const AgitationProcessStatic* current_process{nullptr};
    bool process_active{false};
    bool paused{false};

    // Display information
    char status_text[64]{};
    char step_text[32]{};
    char movement_text[32]{};

    // Process settings
    int8_t push_pull_stops{0};
    static constexpr const char* PUSH_PULL_VALUES[] = {"-2", "-1", "0", "+1", "+2"};
    static constexpr std::size_t PUSH_PULL_COUNT = 5;

    uint8_t roll_count{1};
    static constexpr uint8_t MIN_ROLL_COUNT = 1;
    static constexpr uint8_t MAX_ROLL_COUNT = 100;

    AgitationProcessInterpreter process_interpreter;
    MotorController* motor_controller{nullptr};

    void set_process(const AgitationProcessStatic* process) {
        current_process = process;
        process_interpreter.init(process, motor_controller);
    }

    void update_status(uint32_t elapsed, uint32_t duration) {
        snprintf(
            status_text,
            sizeof(status_text),
            "%s Time: %lu/%lu",
            paused ? "[PAUSED]" : "",
            (unsigned long)elapsed,
            (unsigned long)duration);
    }

    void update_step_text(const char* step_name) {
        snprintf(step_text, sizeof(step_text), "Step: %s", step_name);
    }

    void update_movement_text(const char* direction) {
        snprintf(movement_text, sizeof(movement_text), "Movement: %s", direction);
    }

    void reset() {
        process_active = false;
        paused = false;
        push_pull_stops = 0;
        roll_count = 1;
        process_interpreter.reset();
        motor_controller->stop();
        snprintf(status_text, sizeof(status_text), "Press OK to start");
        snprintf(step_text, sizeof(step_text), "Ready");
        snprintf(movement_text, sizeof(movement_text), "Movement: Idle");
    }

    // Process settings methods
    float get_time_adjustment() const {
        // Each stop is a doubling/halving of time
        return 1.0f * (1 << push_pull_stops);
    }

    const char* get_push_pull_text() const {
        return PUSH_PULL_VALUES[push_pull_stops + 2];
    }
};

// Type alias for protected main view model
using ProtectedMainViewModel = Protected<MainViewModel>;
