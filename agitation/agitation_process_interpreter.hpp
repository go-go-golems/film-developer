#pragma once

#include "../movement/movement.hpp"
#include "../movement/movement_factory.hpp"
#include "../movement/movement_loader.hpp"
#include "agitation_sequence.hpp"
#include "motor_controller.hpp"
#include "process_interpreter_interface.hpp"
#include "processes/bw_standard_process.hpp"
#include "processes/c41_process.hpp"
#include "processes/continuous_gentle_process.hpp"
#include "processes/stand_dev_process.hpp"
#include <stddef.h>
#include <stdint.h>

#define TAG_AGITATION_INTERPRETER "AgitationInterpreter"

class AgitationProcessInterpreter : public ProcessInterpreterInterface {
public:
  AgitationProcessInterpreter();

  void init() override { initAgitation(&STAND_DEV_STATIC, motor_controller); }

  const char *getCurrentStepName() const override {
    auto step = getCurrentStep();
    return step ? step->name : "Unknown";
  }

  const char *getCurrentMovementName() const override {
    return motor_controller ? motor_controller->getDirectionString()
                            : "Unknown";
  }

  void initAgitation(const AgitationProcessStatic *process,
                     MotorController *motor_controller);
  bool tick() override;
  void reset() override;
  void confirm() override;

  // Advances to the next step and resets the interpreter state
  void advanceToNextStep() override;

  // Getters for state information
  bool isWaitingForUser() const override;
  const char *getUserMessage() const override;
  size_t getCurrentStepIndex() const override { return current_step_index; }
  const AgitationProcessStatic *getCurrentProcess() const { return process; }
  ProcessState getState() const override { return process_state; }
  uint32_t getCurrentMovementTimeRemaining() const override;
  uint32_t getCurrentMovementTimeElapsed() const override;
  uint32_t getCurrentMovementDuration() const override;

  // Advances to the next movement in the current sequence
  void advanceToNextMovement();

  // New helper methods
  const AgitationStepStatic *getCurrentStep() const;
  const AgitationMovement *getCurrentMovement() const;

  // Process list management implementation
  size_t getProcessCount() const override { return PROCESS_COUNT; }

  bool getProcessName(size_t index, char *buffer,
                      size_t buffer_size) const override {
    if (index >= PROCESS_COUNT || !buffer || buffer_size == 0) {
      return false;
    }
    const char *name = available_processes[index]->process_name;
    size_t name_len = strlen(name);
    if (name_len >= buffer_size) {
      return false;
    }
    strncpy(buffer, name, buffer_size);
    return true;
  }

  bool selectProcess(const char *process_name) override {
    for (size_t i = 0; i < PROCESS_COUNT; i++) {
      if (strcmp(available_processes[i]->process_name, process_name) == 0) {
        initAgitation(available_processes[i], motor_controller);
        return true;
      }
    }
    return false;
  }

  size_t getCurrentProcessIndex() const override {
    for (size_t i = 0; i < PROCESS_COUNT; i++) {
      if (available_processes[i] == process) {
        return i;
      }
    }
    return 0; // Default to first process if current not found
  }

  void setProcessPushPull(int stops) override {
    push_pull_stops = stops;
    FURI_LOG_D(TAG_AGITATION_INTERPRETER, "Set push/pull stops to %d", stops);
  }

  void setRolls(int count) override {
    roll_count = count;
    FURI_LOG_D(TAG_AGITATION_INTERPRETER, "Set roll count to %d", count);
  }

  void setTemperature(float temp) override {
    temperature = temp;
    FURI_LOG_D(TAG_AGITATION_INTERPRETER, "Set temperature to %.1f",
               static_cast<double>(temp));
  }

  int getProcessPushPull() const override { return push_pull_stops; }
  int getRolls() const override { return roll_count; }
  float getTemperature() const override { return temperature; }

  void pause() override {
    if (process_state == ProcessState::Running) {
      FURI_LOG_D(TAG_AGITATION_INTERPRETER, "Pausing process");
      motor_controller->stop();
      process_state = ProcessState::Paused;
    }
  }

  void resume() override {
    if (process_state == ProcessState::Paused) {
      FURI_LOG_D(TAG_AGITATION_INTERPRETER, "Resuming process");
      process_state = ProcessState::Running;
      // XXX we should read the motor state before we stopped
      motor_controller->clockwise(true);
    }
  }

private:
  void initializeMovementSequence(const AgitationStepStatic *step);

  // Process state
  const AgitationProcessStatic *process;
  size_t current_step_index;
  ProcessState process_state;

  // Temperature tracking
  float current_temperature;
  float target_temperature;

  // Motor control
  MotorController *motor_controller;

  // Movement system
  MovementFactory movement_factory;
  MovementLoader movement_loader;
  AgitationMovement *loaded_sequence[MovementLoader::MAX_SEQUENCE_LENGTH];
  size_t sequence_length;
  size_t current_movement_index;

  uint32_t time_remaining;

  bool movement_completed_previous_tick;
  bool movement_completed;

  const AgitationProcessStatic *available_processes[4] = {
      &C41_FULL_PROCESS_STATIC, &BW_STANDARD_DEV_STATIC, &STAND_DEV_STATIC,
      &CONTINUOUS_GENTLE_STATIC};
  static constexpr size_t PROCESS_COUNT =
      sizeof(available_processes) / sizeof(available_processes[0]);

  int push_pull_stops{0};
  int roll_count{1};
  float temperature{20.0f};
};
