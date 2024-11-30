#pragma once

#include "motor_controller.hpp"
#include "process_interpreter_interface.hpp"
#include <furi.h>

#define CINESTILL_TAG "CineStill"

struct CineStillStep {
  const char *name;
  uint32_t duration_ms;
  bool requires_confirmation;
};

namespace {
// Development time lookup table
struct DevTimeEntry {
  float temp;
  float normal;
  float push1;
  float push2;
  float push3;
  float pull1;
};

// constexpr DevTimeEntry TIME_TABLE[] = {
//     {75.0, 35.0, 50.0, -1.0, -1.0, 27.0}, {80.0, 21.0, 28.0, 37.0,
//     -1.0, 16.25}, {85.0, 13.0, 17.0, 25.0, 35.0, 10.0},
//     {90.0, 8.5, 11.0, 14.75, 21.0, 6.5}, {95.0, 5.75, 7.5, 10.0, 14.33, 4.5},
//     {102.0, 3.5, 4.55, 6.13, 8.75, 2.75}};

constexpr DevTimeEntry TIME_TABLE[] = {{75.0, 0.2, 0.2, 0.2, 0.2, 0.1},
                                       {102.0, 0.2, 0.2, 0.2, 0.2, 0.1}};

constexpr size_t TIME_TABLE_SIZE = sizeof(TIME_TABLE) / sizeof(TIME_TABLE[0]);
} // namespace

class CineStillProcessInterpreter : public ProcessInterpreterInterface {
public:
  CineStillProcessInterpreter(MotorController *motor_controller);

  // ProcessInterpreterInterface implementation
  void init() override;
  bool tick() override;
  void reset() override;
  void stop() override;
  void start() override;
  void confirm() override;
  void advanceToNextStep() override;
  void restartCurrentStep() override;

  // State information
  bool isWaitingForUser() const override;
  bool isComplete() const override;
  const char *getUserMessage() const override;
  ProcessState getState() const override { return state; }
  size_t getCurrentStepIndex() const override { return current_step_index; }

  // Timing information
  uint32_t getCurrentMovementTimeRemaining() const override;
  uint32_t getCurrentMovementTimeElapsed() const override;
  uint32_t getCurrentMovementDuration() const override;

  // Step information
  const char *getCurrentStepName() const override;
  const char *getCurrentMovementName() const override;

  // Process management (simplified - only one process)
  size_t getProcessCount() const override { return 1; }
  bool getProcessName(size_t index, char *buffer,
                      size_t buffer_size) const override;
  bool selectProcess(const char *) override { return true; }
  size_t getCurrentProcessIndex() const override { return 0; }

  // Process parameters
  void setProcessPushPull(int stops) override {
    FURI_LOG_D(CINESTILL_TAG, "Setting push pull stops to %d", stops);
    push_pull_stops = stops;
    updateDevelopTime();
  }
  void setRolls(int count) override {
    FURI_LOG_D(CINESTILL_TAG, "Setting roll count to %d", count);
    roll_count = count;
    updateDevelopTime();
  }
  void setTemperature(float temp) override {
    FURI_LOG_D(CINESTILL_TAG, "Setting temperature to %f",
               static_cast<double>(temp));
    temperature_f = temp;
    updateDevelopTime();
  }

  int getProcessPushPull() const override { return push_pull_stops; }
  int getRolls() const override { return roll_count; }
  float getTemperature() const override { return temperature_f; }

  void pause() override {
    if (state == ProcessState::Running) {
      FURI_LOG_D(CINESTILL_TAG, "Pausing process");
      motor_controller->stop();
      state = ProcessState::Paused;
    }
  }

  void resume() override {
    if (state == ProcessState::Paused) {
      FURI_LOG_D(CINESTILL_TAG, "Resuming process");
      state = ProcessState::Running;
      motor_controller->clockwise(true);
    }
  }

private:
  void updateDevelopTime();
  float calculate_dev_time_ms(float temp_f, int push_pull,
                              float exhaustion_factor);

  MotorController *motor_controller;
  CineStillStep steps[2]; // Developer and Blix
  size_t current_step_index;
  ProcessState state;
  uint32_t accumulated_time_ms{0};

  int push_pull_stops{0};
  int roll_count{1};
  float temperature_f{102.0f}; // Default temperature in Celsius
};

// Development time lookup table
inline CineStillProcessInterpreter::CineStillProcessInterpreter(
    MotorController *motor_controller)
    : motor_controller(motor_controller), current_step_index(0),
      state(ProcessState::Idle) {
  // Initialize with default times
  steps[0] = {"Developer", 0, true};        // Will be calculated
  steps[1] = {"Blix", 8 * 60 * 1000, true}; // 8 minutes
  updateDevelopTime();
}

inline void CineStillProcessInterpreter::init() { reset(); }

inline void CineStillProcessInterpreter::updateDevelopTime() {
  // Convert Celsius to Fahrenheit
  FURI_LOG_D(CINESTILL_TAG, "Temperature: %f",
             static_cast<double>(temperature_f));

  // Calculate exhaustion factor based on roll count (2% per roll)
  double exhaustion_factor = static_cast<double>(roll_count) - 1;

  FURI_LOG_D(CINESTILL_TAG, "Exhaustion factor: %f",
             static_cast<double>(exhaustion_factor));

  // Update developer time
  steps[0].duration_ms =
      calculate_dev_time_ms(temperature_f, push_pull_stops, exhaustion_factor);
}

inline bool CineStillProcessInterpreter::tick() {
  if (state == ProcessState::Complete || state == ProcessState::Error ||
      state == ProcessState::Idle || state == ProcessState::Paused ||
      state == ProcessState::WaitingForUser) {
    FURI_LOG_T(CINESTILL_TAG, "Ignoring tick, current state: %s",
               get_process_state_name(state));
    return false;
  }

  if (current_step_index >= 2) {
    FURI_LOG_I(CINESTILL_TAG, "Process complete");
    state = ProcessState::Complete;
    motor_controller->stop();
    return false;
  }

  // Accumulate time when running
  accumulated_time_ms += 1000; // Add 1 second per tick

  uint32_t elapsed = accumulated_time_ms;
  FURI_LOG_D(CINESTILL_TAG, "Elapsed time: %lu",
             static_cast<unsigned long>(elapsed));
  FURI_LOG_D(CINESTILL_TAG, "Duration: %lu",
             static_cast<unsigned long>(steps[current_step_index].duration_ms));
  if (elapsed >= steps[current_step_index].duration_ms) {
    FURI_LOG_D(CINESTILL_TAG, "Elapsed time >= duration, stopping motor");
    motor_controller->stop();
    if (steps[current_step_index].requires_confirmation) {
      FURI_LOG_D(CINESTILL_TAG, "Requires confirmation, stopping motor");
      state = ProcessState::WaitingForUser;
      return true;
    }
    advanceToNextStep();
  }

  return true;
}

inline void CineStillProcessInterpreter::stop() {
  FURI_LOG_I(CINESTILL_TAG, "Stopping process");
  reset();
}

inline void CineStillProcessInterpreter::start() {
  FURI_LOG_I(CINESTILL_TAG, "Starting process");
  reset();
  state = ProcessState::Running;
  motor_controller->clockwise(true);
}

inline void CineStillProcessInterpreter::reset() {
  FURI_LOG_D(CINESTILL_TAG, "Resetting process");
  current_step_index = 0;
  state = ProcessState::Idle;
  accumulated_time_ms = 0; // Reset accumulated time
  motor_controller->stop();
  updateDevelopTime();
}

inline void CineStillProcessInterpreter::confirm() {
  if (isWaitingForUser()) {
    FURI_LOG_D(CINESTILL_TAG, "Confirming user action");
    advanceToNextStep();
  } else {
    FURI_LOG_W(CINESTILL_TAG, "Not waiting for user, ignoring confirm");
  }
}

inline void CineStillProcessInterpreter::advanceToNextStep() {
  FURI_LOG_I(CINESTILL_TAG, "Advancing to next step");
  if (current_step_index + 1 < 2) {
    FURI_LOG_I(CINESTILL_TAG, "Advancing to step %d", current_step_index + 1);
    current_step_index++;
    accumulated_time_ms = 0;
    state = ProcessState::Running;
  } else {
    FURI_LOG_I(CINESTILL_TAG, "Process complete");
    state = ProcessState::Complete;
  }
}

inline void CineStillProcessInterpreter::restartCurrentStep() {
  FURI_LOG_I(CINESTILL_TAG, "Restarting current step");
  accumulated_time_ms = 0;
  state = ProcessState::Running;
}

inline bool CineStillProcessInterpreter::isWaitingForUser() const {
  return state == ProcessState::WaitingForUser;
}

inline bool CineStillProcessInterpreter::isComplete() const {
  return state == ProcessState::Complete;
}

inline const char *CineStillProcessInterpreter::getUserMessage() const {
  if (isComplete()) {
    return "Process Complete";
  }
  return steps[current_step_index + 1].name;
}

inline uint32_t
CineStillProcessInterpreter::getCurrentMovementTimeElapsed() const {
  return accumulated_time_ms;
}

inline uint32_t
CineStillProcessInterpreter::getCurrentMovementTimeRemaining() const {
  if (current_step_index >= 2)
    return 0;
  uint32_t elapsed = getCurrentMovementTimeElapsed();
  uint32_t duration = steps[current_step_index].duration_ms;
  return elapsed >= duration ? 0 : duration - elapsed;
}

inline uint32_t
CineStillProcessInterpreter::getCurrentMovementDuration() const {
  if (current_step_index >= 2)
    return 0;
  return steps[current_step_index].duration_ms;
}

inline const char *CineStillProcessInterpreter::getCurrentStepName() const {
  if (isComplete())
    return "Complete";
  return steps[current_step_index].name;
}

inline const char *CineStillProcessInterpreter::getCurrentMovementName() const {
  return motor_controller->getDirectionString();
}

inline bool
CineStillProcessInterpreter::getProcessName(size_t index, char *buffer,
                                            size_t buffer_size) const {
  if (index > 0 || !buffer || buffer_size == 0)
    return false;
  strncpy(buffer, "CineStill C41", buffer_size);
  return true;
}

inline float
CineStillProcessInterpreter::calculate_dev_time_ms(float temp_f, int push_pull,
                                                   float exhaustion_factor) {
  // Handle temperature outside range
  if (temp_f < 75.0f || temp_f > 102.0f) {
    return -1.0;
  }

  // Find temperature brackets
  size_t lower_idx = 0;
  size_t upper_idx = 1;

  for (size_t i = 0; i < TIME_TABLE_SIZE - 1; i++) {
    if (temp_f >= TIME_TABLE[i].temp && temp_f <= TIME_TABLE[i + 1].temp) {
      lower_idx = i;
      upper_idx = i + 1;
      break;
    }
  }

  FURI_LOG_D(CINESTILL_TAG, "Lower index: %d, Upper index: %d", lower_idx,
             upper_idx);

  // Get base times for interpolation based on push/pull value
  float lower_time = 0.0f;
  float upper_time = 0.0f;

  switch (push_pull) {
  case -1: // Pull -1

    lower_time = TIME_TABLE[lower_idx].pull1;
    upper_time = TIME_TABLE[upper_idx].pull1;
    FURI_LOG_D(CINESTILL_TAG, "Pull1: Lower time: %f, Upper time: %f",
               static_cast<double>(lower_time),
               static_cast<double>(upper_time));
    break;
  case 0: // Normal
    lower_time = TIME_TABLE[lower_idx].normal;
    upper_time = TIME_TABLE[upper_idx].normal;
    FURI_LOG_D(CINESTILL_TAG, "Normal: Lower time: %f, Upper time: %f",
               static_cast<double>(lower_time),
               static_cast<double>(upper_time));
    break;
  case 1: // Push +1
    lower_time = TIME_TABLE[lower_idx].push1;
    upper_time = TIME_TABLE[upper_idx].push1;
    FURI_LOG_D(CINESTILL_TAG, "Push1: Lower time: %f, Upper time: %f",
               static_cast<double>(lower_time),
               static_cast<double>(upper_time));
    break;
  case 2: // Push +2
    if (TIME_TABLE[lower_idx].push2 < 0 || TIME_TABLE[upper_idx].push2 < 0) {
      return -1.0f; // Error: push not available at this temperature
    }
    lower_time = TIME_TABLE[lower_idx].push2;
    upper_time = TIME_TABLE[upper_idx].push2;
    FURI_LOG_D(CINESTILL_TAG, "Push2: Lower time: %f, Upper time: %f",
               static_cast<double>(lower_time),
               static_cast<double>(upper_time));
    break;
  case 3: // Push +3
    if (TIME_TABLE[lower_idx].push3 < 0 || TIME_TABLE[upper_idx].push3 < 0) {
      return -1.0f; // Error: push not available at this temperature
    }
    lower_time = TIME_TABLE[lower_idx].push3;
    upper_time = TIME_TABLE[upper_idx].push3;
    FURI_LOG_D(CINESTILL_TAG, "Push3: Lower time: %f, Upper time: %f",
               static_cast<double>(lower_time),
               static_cast<double>(upper_time));
    break;
  default:
    return -1.0; // Error: invalid push/pull value
  }

  // Linear interpolation for temperature
  float temp_ratio = (temp_f - TIME_TABLE[lower_idx].temp) /
                     (TIME_TABLE[upper_idx].temp - TIME_TABLE[lower_idx].temp);
  float base_time = lower_time + (upper_time - lower_time) * temp_ratio;
  FURI_LOG_D(CINESTILL_TAG, "Base time: %f", static_cast<double>(base_time));
  FURI_LOG_D(CINESTILL_TAG, "Exhaustion factor: %f",
             static_cast<double>(exhaustion_factor));

  // Apply exhaustion factor - compounding 2% for each roll
  float final_time = base_time * std::pow(1.02f, exhaustion_factor);

  // Convert to milliseconds
  return final_time * 60.0f * 1000.0f;
}
