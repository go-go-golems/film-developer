#include "continuous_agitation_process_interpreter.hpp"
#include <furi.h>

#define TAG "ContinuousAgitationInterpreter"

// Define available processes
const ContinuousProcess ContinuousAgitationProcessInterpreter::available_processes[MAX_PROCESSES] =
    {{"Basic B&W",
      {
          {"Developer", 420000, true}, // 7 minutes
          {"Stop", 60000, true}, // 1 minute
          {"Fix", 300000, true}, // 5 minutes
          {"Wash", 600000, false}, // 10 minutes
      },
      4, // step_count
      20.0f},
     {"C41 Color",
      {
          {"Developer", (int)(3.5 * 60 * 1000), true}, // 3.5 minutes
          {"Blix", (int)(3 * 60 * 1000), true}, // 3 minutes
          {"Wash", (int)(3 * 60 * 1000), false}, // 3 minutes
      },
      5, // step_count
      38.0f}};

const size_t ContinuousAgitationProcessInterpreter::process_count =
    1; // Update as processes are added

ContinuousAgitationProcessInterpreter::ContinuousAgitationProcessInterpreter(
    MotorController* motor_controller)
    : motor_controller(motor_controller)
    , current_process(&available_processes[0])
    , current_step_index(0)
    , state(ProcessState::Idle)
    , step_start_time(0) {
}

void ContinuousAgitationProcessInterpreter::init() {
    reset();
}

bool ContinuousAgitationProcessInterpreter::tick() {
    if(state == ProcessState::Complete || state == ProcessState::Error || state == ProcessState::Paused) {
        return false;
    }

    if(state == ProcessState::Idle) {
        step_start_time = furi_get_tick();
        state = ProcessState::Running;
        motor_controller->clockwise(true);
        return true;
    }

    if(current_step_index >= current_process->step_count) {
        state = ProcessState::Complete;
        motor_controller->stop();
        return false;
    }

    const auto& current_step = current_process->steps[current_step_index];
    uint32_t elapsed = getCurrentMovementTimeElapsed();

    if(elapsed >= current_step.duration_ms) {
        motor_controller->stop();

        if(current_step.requires_confirmation) {
            state = ProcessState::WaitingForUser;
            motor_controller->stop();
            return true;
        }

        advanceToNextStep();
    }

    return true;
}

void ContinuousAgitationProcessInterpreter::reset() {
    current_step_index = 0;
    state = ProcessState::Idle;
    step_start_time = 0;
    motor_controller->stop();
}

void ContinuousAgitationProcessInterpreter::confirm() {
    if(isWaitingForUser()) {
        advanceToNextStep();
    }
}

void ContinuousAgitationProcessInterpreter::advanceToNextStep() {
    if(current_step_index + 1 < current_process->step_count) {
        current_step_index++;
        state = ProcessState::Idle;
    } else {
        state = ProcessState::Complete;
    }
}

bool ContinuousAgitationProcessInterpreter::isWaitingForUser() const {
    if(current_step_index >= current_process->step_count) {
        return false;
    }

    const auto& current_step = current_process->steps[current_step_index];
    return current_step.requires_confirmation &&
           getCurrentMovementTimeElapsed() >= current_step.duration_ms;
}

const char* ContinuousAgitationProcessInterpreter::getUserMessage() const {
    if(current_step_index + 1 >= current_process->step_count) {
        return "Process Complete";
    }
    return current_process->steps[current_step_index + 1].name;
}

uint32_t ContinuousAgitationProcessInterpreter::getCurrentMovementTimeElapsed() const {
    return furi_get_tick() - step_start_time;
}

uint32_t ContinuousAgitationProcessInterpreter::getCurrentMovementTimeRemaining() const {
    if(current_step_index >= current_process->step_count) {
        return 0;
    }

    uint32_t elapsed = getCurrentMovementTimeElapsed();
    uint32_t duration = current_process->steps[current_step_index].duration_ms;

    return elapsed >= duration ? 0 : duration - elapsed;
}

uint32_t ContinuousAgitationProcessInterpreter::getCurrentMovementDuration() const {
    if(current_step_index >= current_process->step_count) {
        return 0;
    }
    return current_process->steps[current_step_index].duration_ms;
}

const char* ContinuousAgitationProcessInterpreter::getCurrentStepName() const {
    if(current_step_index >= current_process->step_count) {
        return "Complete";
    }
    return current_process->steps[current_step_index].name;
}

const char* ContinuousAgitationProcessInterpreter::getCurrentMovementName() const {
    return motor_controller->getDirectionString();
}

size_t ContinuousAgitationProcessInterpreter::getProcessCount() const {
    return process_count;
}

bool ContinuousAgitationProcessInterpreter::getProcessName(
    size_t index,
    char* buffer,
    size_t buffer_size) const {
    if(index >= process_count || !buffer || buffer_size == 0) {
        return false;
    }

    strncpy(buffer, available_processes[index].name, buffer_size);
    return true;
}

bool ContinuousAgitationProcessInterpreter::selectProcess(const char* process_name) {
    for(size_t i = 0; i < process_count; i++) {
        if(strcmp(available_processes[i].name, process_name) == 0) {
            current_process = &available_processes[i];
            reset();
            return true;
        }
    }
    return false;
}

size_t ContinuousAgitationProcessInterpreter::getCurrentProcessIndex() const {
    for(size_t i = 0; i < process_count; i++) {
        if(&available_processes[i] == current_process) {
            return i;
        }
    }
    return 0;
}

void ContinuousAgitationProcessInterpreter::pause() {
    if (state == ProcessState::Running) {
        FURI_LOG_D(TAG, "Pausing process");
        motor_controller->stop();
        state = ProcessState::Paused;
    }
}

void ContinuousAgitationProcessInterpreter::resume() {
    if (state == ProcessState::Paused) {
        FURI_LOG_D(TAG, "Resuming process");
        state = ProcessState::Running;
        motor_controller->clockwise(true);
    }
}
