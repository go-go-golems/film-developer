#pragma once
#include "movement.hpp"

#define TAG "PauseMovement"

class PauseMovement final : public AgitationMovement {
public:
    explicit PauseMovement(uint32_t duration)
        : AgitationMovement(Type::Pause, duration) {
    }

    bool execute(MotorController& motor) override {
        FURI_LOG_D(
            TAG,
            "Executing PauseMovement | Elapsed: %lu/%lu",
            (uint32_t)(elapsed_time + 1),
            (uint32_t)duration);

        if(elapsed_time >= duration) {
            return false;
        }

        motor.stop();
        elapsed_time++;

        if(elapsed_time >= duration) {
            FURI_LOG_D(TAG, "PauseMovement completed");
            return false;
        }
        return true;
    }

    bool isComplete() const override {
        return elapsed_time >= duration;
    }

    void reset() override {
        FURI_LOG_D(TAG, "Resetting PauseMovement");
        elapsed_time = 0;
    }

    void print() const override {
        FURI_LOG_D(
            TAG,
            "PauseMovement | Duration: %lu ticks | Elapsed: %lu | Remaining: %lu",
            (uint32_t)duration,
            (uint32_t)elapsed_time,
            (uint32_t)(duration > elapsed_time ? duration - elapsed_time : 0));
    }
};
