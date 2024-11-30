#pragma once

#include <stddef.h>
#include <stdint.h>

enum class ProcessState {
    Idle,
    Running,
    Complete,
    Error
};

class ProcessInterpreterInterface {
public:
    virtual ~ProcessInterpreterInterface() = default;

    // Process list management
    virtual size_t getProcessCount() const = 0;
    virtual bool getProcessName(size_t index, char* buffer, size_t buffer_size) const = 0;
    virtual bool selectProcess(const char* process_name) = 0;
    virtual size_t getCurrentProcessIndex() const = 0;

    // Core functionality
    virtual void init() = 0;
    virtual bool tick() = 0;
    virtual void reset() = 0;
    virtual void confirm() = 0;

    // Step management
    virtual void advanceToNextStep() = 0;
    virtual size_t getCurrentStepIndex() const = 0;

    // State information
    virtual bool isWaitingForUser() const = 0;
    virtual const char* getUserMessage() const = 0;
    virtual ProcessState getState() const = 0;

    // Timing information
    virtual uint32_t getCurrentMovementTimeRemaining() const = 0;
    virtual uint32_t getCurrentMovementTimeElapsed() const = 0;
    virtual uint32_t getCurrentMovementDuration() const = 0;

    // Step information
    virtual const char* getCurrentStepName() const = 0;
    virtual const char* getCurrentMovementName() const = 0;
};
