#pragma once

#include "../agitation/agitation_process_interpreter.hpp"
#include "../agitation/agitation_processes.hpp"
#include "../motor_controller.hpp"
#include "guard.hpp"
#include <cstdint>

class Model {
public:
    enum class ProcessState {
        NotStarted,
        Running,
        Paused,
        WaitingForUser
    };

    static const char* get_process_state_name(ProcessState state) {
        switch(state) {
        case ProcessState::NotStarted:
            return "NotStarted";
        case ProcessState::Running:
            return "Running";
        case ProcessState::Paused:
            return "Paused";
        case ProcessState::WaitingForUser:
            return "WaitingForUser";
        default:
            return "Unknown";
        }
    }

    // Process state
    const AgitationProcessStatic* current_process{&STAND_DEV_STATIC};
    ProcessState process_state{ProcessState::NotStarted};

    // Display information
    char status_text[64]{};
    char step_text[64]{};
    char movement_text[64]{};

    // Process settings
    int8_t push_pull_stops{0};
    static constexpr const char* PUSH_PULL_VALUES[] = {"-2", "-1", "0", "+1", "+2"};
    static constexpr std::size_t PUSH_PULL_COUNT = 5;

    uint8_t roll_count{1};
    static constexpr uint8_t MIN_ROLL_COUNT = 1;
    static constexpr uint8_t MAX_ROLL_COUNT = 100;

    AgitationProcessInterpreter process_interpreter;
    MotorController* motor_controller{nullptr};

    void init() {
        reset();
    }

    void set_process(const AgitationProcessStatic* process) {
        current_process = process;
        process_interpreter.init(process, motor_controller);
    }

    // Process state transitions
    bool start_process() {
        if(process_state != ProcessState::NotStarted) {
            FURI_LOG_W("FilmDev", "Cannot start process from state: %s", get_process_state_name(process_state));
            return false;
        }
        FURI_LOG_I("FilmDev", "Starting process");
        process_state = ProcessState::Running;
        return true;
    }

    bool pause_process() {
        if(process_state != ProcessState::Running) {
            FURI_LOG_W("FilmDev", "Cannot pause process from state: %s", get_process_state_name(process_state));
            return false;
        }
        FURI_LOG_I("FilmDev", "Pausing process");
        process_state = ProcessState::Paused;
        if(motor_controller) {
            motor_controller->stop();
        }
        return true;
    }

    bool resume_process() {
        if(process_state != ProcessState::Paused) {
            FURI_LOG_W("FilmDev", "Cannot resume process from state: %s", get_process_state_name(process_state));
            return false;
        }
        FURI_LOG_I("FilmDev", "Resuming process");
        process_state = ProcessState::Running;
        return true;
    }

    bool wait_for_user() {
        if(process_state != ProcessState::Running) {
            FURI_LOG_W("FilmDev", "Cannot wait for user from state: %s", get_process_state_name(process_state));
            return false;
        }
        FURI_LOG_I("FilmDev", "Waiting for user confirmation");
        process_state = ProcessState::WaitingForUser;
        if(motor_controller) {
            motor_controller->stop();
        }
        return true;
    }

    bool confirm_user_action() {
        if(process_state != ProcessState::WaitingForUser) {
            FURI_LOG_W("FilmDev", "Cannot confirm user action from state: %s", get_process_state_name(process_state));
            return false;
        }
        FURI_LOG_I("FilmDev", "User action confirmed");
        process_state = ProcessState::Running;
        return true;
    }

    bool complete_process() {
        FURI_LOG_I("FilmDev", "Process completed");
        process_state = ProcessState::NotStarted;
        if(motor_controller) {
            motor_controller->stop();
        }
        return true;
    }

    bool is_process_active() const {
        return process_state != ProcessState::NotStarted;
    }

    bool is_process_paused() const {
        return process_state == ProcessState::Paused;
    }

    bool is_waiting_for_user() const {
        return process_state == ProcessState::WaitingForUser;
    }

    void update_status(uint32_t elapsed, uint32_t duration) {
        const char* state_prefix = "";
        switch(process_state) {
        case ProcessState::Paused:
            state_prefix = "[PAUSED] ";
            break;
        case ProcessState::WaitingForUser:
            state_prefix = "[WAITING] ";
            break;
        default:
            break;
        }

        snprintf(
            status_text,
            sizeof(status_text),
            "%sTime: %lu/%lu",
            state_prefix,
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
        process_state = ProcessState::NotStarted;
        push_pull_stops = 0;
        roll_count = 1;
        current_process = &STAND_DEV_STATIC;
        process_interpreter.init(current_process, motor_controller);

        if(motor_controller) {
            motor_controller->stop();
        }

        snprintf(status_text, sizeof(status_text), "Press OK to start");
        snprintf(step_text, sizeof(step_text), "Ready");
        snprintf(movement_text, sizeof(movement_text), "Movement: Idle");
        
        FURI_LOG_I("FilmDev", "Model reset");
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
using ProtectedModel = Protected<Model>;
