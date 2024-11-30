#pragma once

#include "process_interpreter_interface.hpp"
#include "motor_controller.hpp"

#define MAX_STEPS     10
#define MAX_PROCESSES 5

struct ContinuousStep {
    const char* name;
    uint32_t duration_ms;
    bool requires_confirmation;
};

struct ContinuousProcess {
    const char* name;
    ContinuousStep steps[MAX_STEPS];
    size_t step_count;
    float default_temperature;
};

class ContinuousAgitationProcessInterpreter : public ProcessInterpreterInterface {
public:
    ContinuousAgitationProcessInterpreter(MotorController* motor_controller);

    // ProcessInterpreterInterface implementation
    void init() override;
    bool tick() override;
    void reset() override;
    void confirm() override;
    void advanceToNextStep() override;

    // State information
    bool isWaitingForUser() const override;
    const char* getUserMessage() const override;
    ProcessState getState() const override {
        return state;
    }
    size_t getCurrentStepIndex() const override {
        return current_step_index;
    }

    // Timing information
    uint32_t getCurrentMovementTimeRemaining() const override;
    uint32_t getCurrentMovementTimeElapsed() const override;
    uint32_t getCurrentMovementDuration() const override;

    // Step information
    const char* getCurrentStepName() const override;
    const char* getCurrentMovementName() const override;

    // Process management
    size_t getProcessCount() const override;
    bool getProcessName(size_t index, char* buffer, size_t buffer_size) const override;
    bool selectProcess(const char* process_name) override;
    size_t getCurrentProcessIndex() const override;

    // Process parameters
    void setProcessPushPull(int stops) override {
        push_pull_stops = stops;
    }
    void setRolls(int count) override {
        roll_count = count;
    }
    void setTemperature(float temp) override {
        temperature = temp;
    }

    int getProcessPushPull() const override {
        return push_pull_stops;
    }
    int getRolls() const override {
        return roll_count;
    }
    float getTemperature() const override {
        return temperature;
    }

    // Pause/Resume
    void pause() override;
    void resume() override;

private:
    MotorController* motor_controller;
    const ContinuousProcess* current_process;
    size_t current_step_index;
    ProcessState state;
    uint32_t step_start_time;

    int push_pull_stops{0};
    int roll_count{1};
    float temperature{20.0f};

    static const ContinuousProcess available_processes[MAX_PROCESSES];
    static const size_t process_count;
};
