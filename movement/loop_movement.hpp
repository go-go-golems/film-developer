#pragma once
#include "movement.hpp"
#include <cstddef>

#define TAG_LOOP "LoopMovement"

class LoopMovement final : public AgitationMovement {
public:
    LoopMovement(
        AgitationMovement** sequence,
        size_t sequence_length,
        uint32_t iterations,
        uint32_t max_duration)
        : AgitationMovement(Type::Loop, max_duration)
        , sequence(sequence)
        , sequence_length(sequence_length)
        , iterations(iterations)
        , current_iteration(0)
        , current_index(0) {
    }

    bool execute(MotorController& motor) override {
        if(isComplete()) {
            return false;
        }

        FURI_LOG_D(
            TAG_LOOP,
            "Executing LoopMovement | Iteration: %lu/%lu | Movement: %lu/%lu",
            (uint32_t)(current_iteration + 1),
            (uint32_t)iterations,
            (uint32_t)(current_index + 1),
            (uint32_t)sequence_length);

        FURI_LOG_D(
            TAG_LOOP,
            "Executing Loop SubMovement: %lu/%lu",
            (uint32_t)(current_index + 1),
            (uint32_t)sequence_length);
        sequence[current_index]->print();
        bool result = sequence[current_index]->execute(motor);
        if(!result) {
            advanceToNextMovement();
        }

        elapsed_time++;
        return !isComplete();
    }

    bool isComplete() const override {
        return (iterations > 0 && current_iteration >= iterations) ||
               (duration > 0 && elapsed_time >= duration);
    }

    void reset() override {
        FURI_LOG_D(TAG_LOOP, "Resetting LoopMovement");
        elapsed_time = 0;
        current_iteration = 0;
        current_index = 0;
        for(size_t i = 0; i < sequence_length; i++) {
            sequence[i]->reset();
        }
    }

    void print() const override {
        FURI_LOG_D(
            TAG_LOOP,
            "LoopMovement | Iteration: %lu/%lu | Duration: %lu ticks | "
            "Elapsed: %lu | Remaining: %lu",
            (uint32_t)current_iteration,
            (uint32_t)iterations,
            (uint32_t)duration,
            (uint32_t)elapsed_time,
            (uint32_t)(duration > elapsed_time ? duration - elapsed_time : 0));

        FURI_LOG_D(TAG_LOOP, "Sequence:");
        for(size_t i = 0; i < sequence_length; i++) {
            if(sequence[i]) {
                FURI_LOG_D(
                    TAG_LOOP,
                    "%s[%lu]%s ",
                    i == current_index ? ">" : " ",
                    (uint32_t)i,
                    i == current_index ? "<" : " ");
                sequence[i]->print();
            }
        }
    }

private:
    void advanceToNextMovement() {
        FURI_LOG_D(
            TAG_LOOP,
            "Advancing loop to next movement: %lu/%lu",
            (uint32_t)(current_index + 1),
            (uint32_t)sequence_length);
        current_index++;
        if(current_index >= sequence_length) {
            current_index = 0;
            current_iteration++;
        }
        if(current_index < sequence_length) {
            sequence[current_index]->reset();
        }
    }

    AgitationMovement** sequence;
    size_t sequence_length;
    uint32_t iterations;
    uint32_t current_iteration;
    size_t current_index;
};
