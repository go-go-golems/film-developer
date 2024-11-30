#pragma once

#include <stddef.h>
#include <stdint.h>

enum class ProcessState {
  Idle,
  Running,
  Paused,
  WaitingForUser,
  Complete,
  Error
};

/**
 * @brief Interface for film development process interpretation and control
 *
 * The ProcessInterpreterInterface manages the execution of film development
 * processes, handling timing, agitation sequences, and user interactions. It
 * serves as the core engine that drives the film development workflow.
 *
 * ## Settings View Integration
 * The settings view allows users to configure process parameters through the
 * interface:
 * ```cpp
 * // Example from settings_view.hpp
 * void push_pull_change_callback(VariableItem* item) {
 *     process_interpreter->setProcessPushPull(stops);
 *     process_interpreter->setRolls(count);
 * }
 * ```
 *
 * ## Main Development Loop
 * The main development loop calls tick() periodically to advance the process:
 * ```cpp
 * // In FilmDeveloperApp::update()
 * bool still_active = process_interpreter->tick();
 * model->update_status(
 *     process_interpreter->getCurrentMovementTimeElapsed(),
 *     process_interpreter->getCurrentMovementDuration()
 * );
 * ```
 *
 * ## Main Development View
 * The main development view displays the current process state by querying the
 * interpreter:
 * ```cpp
 * // In MainDevelopmentView
 * void update_display() {
 *     display_step(interpreter->getCurrentStepName());
 *     display_time(interpreter->getCurrentMovementTimeElapsed());
 *     display_movement(interpreter->getCurrentMovementName());
 * }
 * ```
 *
 * ## Process Selection
 * The process selection view uses the interpreter to list and select processes:
 * ```cpp
 * // In ProcessSelectionView
 * void init() {
 *     for(size_t i = 0; i < interpreter->getProcessCount(); i++) {
 *         char name[32];
 *         interpreter->getProcessName(i, name, sizeof(name));
 *         add_item(name);
 *     }
 * }
 * ```
 *
 * ## Process Parameters
 * - Push/Pull: Adjusts development times by stops (-2 to +2)
 * - Roll Count: Number of rolls being developed (affects agitation)
 * - Temperature: Process temperature in Celsius
 *
 * ## State Management
 * The interpreter maintains the process state (Idle, Running, Complete, Error)
 * and handles transitions between steps and movements within steps.
 *
 * @see AgitationProcessInterpreter for the concrete implementation
 * @see MainViewModel for the model layer integration
 * @see SettingsView for the settings UI
 */
class ProcessInterpreterInterface {
public:
  virtual ~ProcessInterpreterInterface() = default;

  // Process list management
  virtual size_t getProcessCount() const = 0;
  virtual bool getProcessName(size_t index, char *buffer,
                              size_t buffer_size) const = 0;
  virtual bool selectProcess(const char *process_name) = 0;
  virtual size_t getCurrentProcessIndex() const = 0;

  // Core functionality
  virtual void init() = 0;
  virtual bool tick() = 0;
  virtual void reset() = 0;
  virtual void stop() = 0;
  virtual void confirm() = 0;

  // Step management
  virtual void advanceToNextStep() = 0;
  virtual size_t getCurrentStepIndex() const = 0;

  // State information
  virtual bool isWaitingForUser() const = 0;
  virtual bool isComplete() const = 0;
  virtual const char *getUserMessage() const = 0;
  virtual ProcessState getState() const = 0;

  // Timing information
  virtual uint32_t getCurrentMovementTimeRemaining() const = 0;
  virtual uint32_t getCurrentMovementTimeElapsed() const = 0;
  virtual uint32_t getCurrentMovementDuration() const = 0;

  // Step information
  virtual const char *getCurrentStepName() const = 0;
  virtual const char *getCurrentMovementName() const = 0;

  // Add these methods to the interface class
  virtual void setProcessPushPull(int stops) = 0;
  virtual void setRolls(int count) = 0;
  virtual void setTemperature(float temp) = 0;

  virtual int getProcessPushPull() const = 0;
  virtual int getRolls() const = 0;
  virtual float getTemperature() const = 0;

  // Add new methods for pause/resume
  virtual void pause() = 0;
  virtual void resume() = 0;
};
