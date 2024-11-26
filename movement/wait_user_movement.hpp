#pragma once
#include "movement.hpp"

#define TAG_WAIT_USER "WaitUserMovement"

class WaitUserMovement final : public AgitationMovement {
public:
    WaitUserMovement()
        : AgitationMovement(Type::WaitUser) {
    }

    bool execute(MotorController& motor) override {
        FURI_LOG_D(
            TAG_WAIT_USER,
            "Executing WaitUserMovement | State: %s | Elapsed: %lu",
            user_acknowledged ? "acknowledged" : "waiting",
            (uint32_t)(elapsed_time + 1));

        motor.stop();
        return !user_acknowledged;
    }

    bool isComplete() const override {
        return user_acknowledged;
    }

    void reset() override {
        user_acknowledged = false;
    }

    void acknowledgeUser() {
        user_acknowledged = true;
    }

    void print() const override {
        FURI_LOG_D(
            TAG_WAIT_USER,
            "WaitUserMovement | State: %s | Elapsed: %lu",
            user_acknowledged ? "acknowledged" : "waiting",
            (uint32_t)elapsed_time);
    }

private:
    bool user_acknowledged{false};
};
