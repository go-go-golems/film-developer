#include "agitation/agitation_process_interpreter.hpp"
#include "agitation/agitation_processes.hpp"
#include "debug.hpp"
#include "motor_controller.hpp"
#include <memory>
#include <stdio.h>
#include <string.h>


AgitationProcessInterpreter::AgitationProcessInterpreter()
    : process(&STAND_DEV_STATIC)
    , current_step_index(0)
    , process_state(ProcessState::Idle)
    , current_temperature(20.0f)
    , target_temperature(20.0f)
    , motor_controller(nullptr)
    , movement_loader(movement_factory)
    , sequence_length(0)
    , current_movement_index(0)
    , time_remaining(0)
    , movement_completed(false) {
    memset(loaded_sequence, 0, sizeof(loaded_sequence));
}

void AgitationProcessInterpreter::initAgitation(
    const AgitationProcessStatic* process,
    MotorController* motor_controller) {
    furi_assert(process);
    furi_assert(motor_controller);

    this->process = process;
    this->motor_controller = motor_controller;
    current_step_index = 0;
    process_state = ProcessState::Idle;

    push_pull_stops = 0;
    roll_count = 1;
    temperature = process->temperature;
    target_temperature = process->temperature;
    
    movement_completed = false;

    sequence_length = 0;
    current_movement_index = 0;

    FURI_LOG_I(TAG_AGITATION_INTERPRETER, "Process Interpreter Initialized:");
    FURI_LOG_I(TAG_AGITATION_INTERPRETER, "  Process Name: %s", process->process_name);
    FURI_LOG_I(TAG_AGITATION_INTERPRETER, "  Film Type: %s", process->film_type);
    FURI_LOG_I(TAG_AGITATION_INTERPRETER, "  Total Steps: %u", (unsigned int)process->steps_length);
    FURI_LOG_I(TAG_AGITATION_INTERPRETER, "  Initial Temperature: %.1f", static_cast<double>(target_temperature));
}

void AgitationProcessInterpreter::initializeMovementSequence(const AgitationStepStatic* step) {
    memset(loaded_sequence, 0, sizeof(loaded_sequence));

    sequence_length =
        movement_loader.loadSequence(step->sequence, step->sequence_length, loaded_sequence);

    current_movement_index = 0;

    if(sequence_length == 0) {
        FURI_LOG_E(TAG_AGITATION_INTERPRETER, "Failed to load movement sequence");
        process_state = ProcessState::Error;
        return;
    }

    for(size_t i = 0; i < sequence_length; i++) {
        if(loaded_sequence[i]) {
            loaded_sequence[i]->reset();
        }
    }

    FURI_LOG_D(
        TAG_AGITATION_INTERPRETER,
        "Loaded movement sequence with %u movements",
        (unsigned int)sequence_length);
}

bool AgitationProcessInterpreter::tick() {
    furi_assert(process);
    furi_assert(motor_controller);

    if(current_step_index >= process->steps_length || process_state == ProcessState::Error) {
        FURI_LOG_I(
            TAG_AGITATION_INTERPRETER,
            "Process %s",
            process_state == ProcessState::Error ? "Error" : "Completed");
        process_state = process_state == ProcessState::Error ? ProcessState::Error :
                                                               ProcessState::Complete;
        return false;
    }

    if(movement_completed) {
        advanceToNextMovement();
        movement_completed = false;

        if(current_movement_index >= sequence_length) {
            FURI_LOG_D(
                TAG_AGITATION_INTERPRETER, "Movement sequence completed, advancing to next step");
            advanceToNextStep();
            return true;
        }
    }

    const AgitationStepStatic* current_step = &process->steps[current_step_index];
    target_temperature = current_step->temperature;

    if(process_state == ProcessState::Idle || process_state == ProcessState::Complete) {
        FURI_LOG_I(
            TAG_AGITATION_INTERPRETER,
            "Initializing Movement Sequence for Step %u: %s",
            (unsigned int)current_step_index,
            current_step->name ? current_step->name : "Unnamed Step");

        initializeMovementSequence(current_step);
        process_state = ProcessState::Running;
    }

    bool movement_active = false;
    if(current_movement_index < sequence_length) {
        AgitationMovement* current_movement = loaded_sequence[current_movement_index];

        if(current_movement) {
            movement_active = current_movement->execute(*motor_controller);

            if(current_movement->getType() == AgitationMovement::Type::WaitUser) {
                return true;
            }

            if(!movement_active) {
                movement_completed = true;
            }
        }
    }

    return movement_active || current_step_index < process->steps_length;
}

void AgitationProcessInterpreter::reset() {
    initAgitation(process, motor_controller);
}

void AgitationProcessInterpreter::confirm() {
    if(isWaitingForUser()) {
        if(current_step_index + 1 >= process->steps_length) {
            // If this is the last step, just advance the movement
            advanceToNextMovement();
        } else {
            // If there's a next step, advance to it
            advanceToNextStep();
        }
    }
}

void AgitationProcessInterpreter::advanceToNextStep() {
    if(!(current_step_index + 1 < process->steps_length)) {
        FURI_LOG_W(TAG, "Cannot advance to next step, already at last step");
        return;
    }
    FURI_LOG_I(
        TAG,
        "Advancing to next step: %s, current step index: %u/%u",
        process->steps[current_step_index + 1].name,
        (unsigned int)(current_step_index + 1),
        (unsigned int)process->steps_length);
    current_step_index++;
    process_state = ProcessState::Idle;
    sequence_length = 0;
    current_movement_index = 0;

    // Reset all movements in current sequence
    for(size_t i = 0; i < sequence_length; i++) {
        if(loaded_sequence[i]) {
            loaded_sequence[i]->reset();
        }
    }
}

uint32_t AgitationProcessInterpreter::getCurrentMovementTimeRemaining() const {
    if(current_movement_index < sequence_length && loaded_sequence[current_movement_index]) {
        return loaded_sequence[current_movement_index]->timeRemaining();
    }
    return 0;
}

uint32_t AgitationProcessInterpreter::getCurrentMovementTimeElapsed() const {
    if(current_movement_index < sequence_length && loaded_sequence[current_movement_index]) {
        return loaded_sequence[current_movement_index]->timeElapsed();
    }
    return 0;
}

uint32_t AgitationProcessInterpreter::getCurrentMovementDuration() const {
    if(current_movement_index < sequence_length && loaded_sequence[current_movement_index]) {
        return loaded_sequence[current_movement_index]->getDuration();
    }
    return 0;
}

bool AgitationProcessInterpreter::isWaitingForUser() const {
    if(current_movement_index < sequence_length && loaded_sequence[current_movement_index]) {
        return loaded_sequence[current_movement_index]->getType() ==
               AgitationMovement::Type::WaitUser;
    }
    return false;
}

const char* AgitationProcessInterpreter::getUserMessage() const {
    static char next_step_message[32]; // Static buffer for the message

    if(current_step_index + 1 >= process->steps_length) {
        return "Finish";
    } else {
        snprintf(
            next_step_message,
            sizeof(next_step_message),
            "Next: %s",
            process->steps[current_step_index + 1].name);
        return next_step_message;
    }
}

void AgitationProcessInterpreter::advanceToNextMovement() {
    if(current_movement_index < sequence_length) {
        FURI_LOG_D(
            TAG,
            "Advancing to next movement: %u/%u",
            (unsigned int)(current_movement_index + 1),
            (unsigned int)sequence_length);

        current_movement_index++;
        if(current_movement_index < sequence_length && loaded_sequence[current_movement_index]) {
            loaded_sequence[current_movement_index]->reset();
        }
    }
}

const AgitationStepStatic* AgitationProcessInterpreter::getCurrentStep() const {
    if(!process || current_step_index >= process->steps_length) {
        return nullptr;
    }
    return &process->steps[current_step_index];
}

const AgitationMovement* AgitationProcessInterpreter::getCurrentMovement() const {
    if(current_movement_index >= sequence_length) {
        return nullptr;
    }
    return loaded_sequence[current_movement_index];
}
