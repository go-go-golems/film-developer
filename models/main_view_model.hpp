#pragma once

#include "../agitation_process_interpreter.hpp"
#include "../agitation_processes.hpp"
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

    void update_status(uint32_t elapsed, uint32_t duration) {
        snprintf(status_text, sizeof(status_text),
                "%s Time: %lu/%lu",
                paused ? "[PAUSED]" : "",
                (unsigned long)elapsed, (unsigned long)duration);
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
        snprintf(status_text, sizeof(status_text), "Press OK to start");
        snprintf(step_text, sizeof(step_text), "Ready");
        snprintf(movement_text, sizeof(movement_text), "Movement: Idle");
    }
}; 