#pragma once

#include "../agitation/agitation_process_interpreter.hpp"
#include "../agitation/agitation_processes.hpp"
#include "../motor_controller.hpp"
#include "guard.hpp"
#include <cstdint>

#define MODEL_TAG "FilmDevModel"

class Model {
public:
  enum class ProcessState { NotStarted, Running, Paused, WaitingForUser };

  static const char *get_process_state_name(ProcessState state) {
    switch (state) {
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
  ProcessState process_state{ProcessState::NotStarted};

  // Display information
  char status_text[64]{};
  char step_text[64]{};
  char movement_text[64]{};

  // Process settings
  int8_t push_pull_stops{0};
  static constexpr const char *PUSH_PULL_VALUES[] = {"-1", "0", "+1", "+2",
                                                     "+3"};
  static constexpr std::size_t PUSH_PULL_COUNT = 5;

  uint8_t roll_count{1};
  static constexpr uint8_t MIN_ROLL_COUNT = 1;
  static constexpr uint8_t MAX_ROLL_COUNT = 100;

  ProcessInterpreterInterface *process_interpreter{nullptr};
  MotorController *motor_controller{nullptr};

  void init() { reset(); }

  void set_process(const char *process_name) {
    if (process_interpreter) {
      process_interpreter->selectProcess(process_name);
      process_interpreter->setProcessPushPull(push_pull_stops);
      process_interpreter->setRolls(roll_count);
    }
  }

  // Process state transitions
  bool start_process() {
    if (process_state != ProcessState::NotStarted) {
      FURI_LOG_W(MODEL_TAG, "Cannot start process from state: %s",
                 get_process_state_name(process_state));
      return false;
    }
    FURI_LOG_I(MODEL_TAG, "Starting process, current state: %s",
               get_process_state_name(process_state));
    process_interpreter->start();
    process_state = ProcessState::Running;
    return true;
  }

  bool pause_process() {
    if (process_state != ProcessState::Running) {
      FURI_LOG_W(MODEL_TAG, "Cannot pause process from state: %s",
                 get_process_state_name(process_state));
      return false;
    }
    FURI_LOG_I(MODEL_TAG, "Pausing process, current state: %s",
               get_process_state_name(process_state));
    process_state = ProcessState::Paused;
    if (motor_controller) {
      motor_controller->stop();
    }
    return true;
  }

  bool resume_process() {
    if (process_state != ProcessState::Paused) {
      FURI_LOG_W(MODEL_TAG, "Cannot resume process from state: %s",
                 get_process_state_name(process_state));
      return false;
    }
    FURI_LOG_I(MODEL_TAG, "Resuming process, current state: %s",
               get_process_state_name(process_state));
    process_state = ProcessState::Running;
    return true;
  }

  bool wait_for_user() {
    if (process_state != ProcessState::Running) {
      FURI_LOG_W(MODEL_TAG, "Cannot wait for user from state: %s",
                 get_process_state_name(process_state));
      return false;
    }
    FURI_LOG_I(MODEL_TAG, "Waiting for user confirmation, current state: %s",
               get_process_state_name(process_state));
    process_state = ProcessState::WaitingForUser;
    if (motor_controller) {
      motor_controller->stop();
    }
    return true;
  }

  bool confirm_user_action() {
    if (process_state != ProcessState::WaitingForUser) {
      FURI_LOG_W(MODEL_TAG, "Cannot confirm user action from state: %s",
                 get_process_state_name(process_state));
      return false;
    }
    process_interpreter->confirm();
    FURI_LOG_I(MODEL_TAG, "User action confirmed, current state: %s",
               get_process_state_name(process_state));
    process_state = ProcessState::Running;
    return true;
  }

  void stop_process() {
    FURI_LOG_I(MODEL_TAG, "Stopping process, current state: %s",
               get_process_state_name(process_state));
    if (process_interpreter) {
      process_interpreter->stop();
    }
    process_state = ProcessState::NotStarted;
  }

  bool complete_process() {
    FURI_LOG_I(MODEL_TAG, "Process completed, current state: %s",
               get_process_state_name(process_state));
    process_state = ProcessState::NotStarted;
    if (motor_controller) {
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
    const char *state_prefix = "";
    switch (process_state) {
    case ProcessState::Paused:
      state_prefix = "[PAUSED] ";
      break;
    case ProcessState::WaitingForUser:
      state_prefix = "[WAITING] ";
      break;
    default:
      break;
    }

    snprintf(status_text, sizeof(status_text),
             "%sTime: %02lu:%02lu/%02lu:%02lu", state_prefix,
             (unsigned long)((elapsed / 1000) / 60),  // Minutes
             (unsigned long)((elapsed / 1000) % 60),  // Seconds
             (unsigned long)((duration / 1000) / 60), // Total Minutes
             (unsigned long)((duration / 1000) % 60)  // Total Seconds
    );
  }

  void update_step_text(const char *step_name) {
    snprintf(step_text, sizeof(step_text), "Step: %s", step_name);
  }

  void update_movement_text(const char *direction) {
    snprintf(movement_text, sizeof(movement_text), "Movement: %s", direction);
  }

  void reset() {
    process_state = ProcessState::NotStarted;
    push_pull_stops = 0;
    roll_count = 1;
    if (process_interpreter) {
      process_interpreter->init();
      process_interpreter->setProcessPushPull(push_pull_stops);
      process_interpreter->setRolls(roll_count);
    }

    if (motor_controller) {
      motor_controller->stop();
    }

    snprintf(status_text, sizeof(status_text), "Press OK to start");
    snprintf(step_text, sizeof(step_text), "Ready");
    snprintf(movement_text, sizeof(movement_text), "Movement: Idle");

    FURI_LOG_I(MODEL_TAG, "Model reset");
  }

  // Process settings methods
  float get_time_adjustment() const {
    // Each stop is a doubling/halving of time
    return 1.0f * (1 << push_pull_stops);
  }

  const char *get_push_pull_text() const {
    return PUSH_PULL_VALUES[push_pull_stops + 2];
  }
};

// Type alias for protected main view model
using ProtectedModel = Protected<Model>;
