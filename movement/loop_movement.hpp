#pragma once
#include "movement.hpp"
#include <cstddef>

class LoopMovement final : public AgitationMovement {
public:
  LoopMovement(AgitationMovement **sequence, size_t sequence_length,
               uint32_t iterations, uint32_t max_duration)
      : AgitationMovement(Type::Loop, max_duration), sequence(sequence),
        sequence_length(sequence_length), iterations(iterations),
        current_iteration(0), current_index(0) {}

  bool execute(MotorController &motor) override {
    if (isComplete()) {
      return false;
    }

    DEBUG_PRINT("Executing LoopMovement | Iteration: %u/%u | Movement: %lu/%lu",
                current_iteration + 1, iterations, current_index + 1,
                sequence_length);

    DEBUG_PRINT("Executing Loop SubMovement: %lu/%lu", current_index + 1,
                sequence_length);
    sequence[current_index]->print();
    bool result = sequence[current_index]->execute(motor);
    if (!result) {
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
    DEBUG_PRINT("Resetting LoopMovement");
    elapsed_time = 0;
    current_iteration = 0;
    current_index = 0;
    for (size_t i = 0; i < sequence_length; i++) {
      sequence[i]->reset();
    }
  }

  void print() const override {
    DEBUG_PRINT("LoopMovement | Iteration: %u/%u | Duration: %u ticks | "
                "Elapsed: %u | Remaining: %u",
                current_iteration, iterations, duration, elapsed_time,
                duration > elapsed_time ? duration - elapsed_time : 0);

    DEBUG_PRINT("Sequence:");
    for (size_t i = 0; i < sequence_length; i++) {
      if (sequence[i]) {
        DEBUG_PRINT("%s[%lu]%s ", i == current_index ? ">" : " ", i,
                    i == current_index ? "<" : " ");
        sequence[i]->print();
      }
    }
  }

private:
  void advanceToNextMovement() {
    DEBUG_PRINT("Advancing loop to next movement: %lu/%lu", current_index + 1,
                sequence_length);
    current_index++;
    if (current_index >= sequence_length) {
      current_index = 0;
      current_iteration++;
    }
    if (current_index < sequence_length) {
      sequence[current_index]->reset();
    }
  }

  AgitationMovement **sequence;
  size_t sequence_length;
  uint32_t iterations;
  uint32_t current_iteration;
  size_t current_index;
};
