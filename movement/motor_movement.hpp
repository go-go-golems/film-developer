#pragma once
#include "movement.hpp"

#define TAG_MOTOR_MOVEMENT "MotorMovement"

class MotorMovement final : public AgitationMovement {
public:
    explicit MotorMovement(Type type, uint32_t duration)
        : AgitationMovement(type, duration) {
        if(type != Type::CW && type != Type::CCW) {
            // In production code, we might want to handle this error differently
            type = Type::CW;
        }
    }

    bool execute(MotorController& motor) override {
        if(elapsed_time >= duration) {
            return false;
        }

        FURI_LOG_D(
            TAG_MOTOR_MOVEMENT,
            "Executing MotorMovement: %s | Elapsed: %lu/%lu",
            type == Type::CW ? "CW" : "CCW",
            (uint32_t)(elapsed_time + 1),
            (uint32_t)duration);

        if(type == Type::CW) {
            motor.clockwise(true);
        } else {
            motor.counterClockwise(true);
        }

        elapsed_time++;

        if(elapsed_time >= duration) {
            return false;
        }

        return true;
    }

    bool isComplete() const override {
        return elapsed_time >= duration;
    }

    void reset() override {
        elapsed_time = 0;
    }

    void print() const override {
        FURI_LOG_D(
            TAG_MOTOR_MOVEMENT,
            "MotorMovement: %s | Duration: %lu ticks | Elapsed: %lu | Remaining: %lu",
            type == Type::CW ? "CW" : "CCW",
            (uint32_t)duration,
            (uint32_t)elapsed_time,
            (uint32_t)(duration > elapsed_time ? duration - elapsed_time : 0));
    }
};
