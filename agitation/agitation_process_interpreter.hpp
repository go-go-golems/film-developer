#pragma once

#include "agitation_sequence.hpp"
#include "motor_controller.hpp"
#include "../movement/movement.hpp"
#include "../movement/movement_factory.hpp"
#include "../movement/movement_loader.hpp"
#include <stddef.h>
#include <stdint.h>

enum class AgitationProcessState {
    Idle,
    Running,
    Complete,
    Error
};

class AgitationProcessInterpreter {
public:
    AgitationProcessInterpreter();

    void init(const AgitationProcessStatic* process, MotorController* motor_controller);
    bool tick();
    void reset();
    void confirm();

    // Advances to the next step and resets the interpreter state
    void advanceToNextStep();

    // Alias for advanceToNextStep for backward compatibility
    void skipToNextStep();

    // Getters for state information
    bool isWaitingForUser() const;
    const char* getUserMessage() const;
    size_t getCurrentStepIndex() const {
        return current_step_index;
    }
    const AgitationProcessStatic* getCurrentProcess() const {
        return process;
    }
    AgitationProcessState getState() const {
        return process_state;
    }
    uint32_t getCurrentMovementTimeRemaining() const;
    uint32_t getCurrentMovementTimeElapsed() const;
    uint32_t getCurrentMovementDuration() const;

    // Advances to the next movement in the current sequence
    void advanceToNextMovement();

    // New helper methods
    const AgitationStepStatic* getCurrentStep() const;
    const AgitationMovement* getCurrentMovement() const;

private:
    void initializeMovementSequence(const AgitationStepStatic* step);

    // Process state
    const AgitationProcessStatic* process;
    size_t current_step_index;
    AgitationProcessState process_state;

    // Temperature tracking
    float current_temperature;
    float target_temperature;

    // Motor control
    MotorController* motor_controller;

    // Movement system
    MovementFactory movement_factory;
    MovementLoader movement_loader;
    AgitationMovement* loaded_sequence[MovementLoader::MAX_SEQUENCE_LENGTH];
    size_t sequence_length;
    size_t current_movement_index;

    uint32_t time_remaining;

    bool movement_completed_previous_tick;
    bool movement_completed;
};
