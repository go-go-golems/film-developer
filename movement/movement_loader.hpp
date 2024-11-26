#pragma once
#include "../agitation_sequence.hpp"
#include "movement.hpp"
#include "movement_factory.hpp"
#include <array>

#define TAG_MOVEMENT_LOADER "MovementLoader"

class MovementLoader {
private:
public:
    // Maximum number of movements in a sequence
    static constexpr size_t MAX_SEQUENCE_LENGTH = 32;

    /**
   * @brief Construct a MovementLoader with a movement factory
   * @param factory The factory to use for creating movements
   */
    explicit MovementLoader(MovementFactory& factory)
        : factory_(factory) {
    }

    /**
   * @brief Load a sequence of movements from static declarations
   * @param static_sequence Array of static movement declarations
   * @param sequence_length Length of the static sequence
   * @param sequence Array to store the created movements
   * @return Actual length of the loaded sequence
   */
    size_t loadSequence(
        const AgitationMovementStatic* static_sequence,
        size_t sequence_length,
        AgitationMovement* sequence[]) {
        FURI_LOG_T(
            TAG_MOVEMENT_LOADER,
            "Loading sequence with length: %lu",
            (uint32_t)sequence_length);

        size_t loaded_length = 0;

        for(size_t i = 0; i < sequence_length && i < MAX_SEQUENCE_LENGTH; i++) {
            FURI_LOG_T(TAG_MOVEMENT_LOADER, "Loading movement %lu", (uint32_t)i);
            sequence[loaded_length] = loadMovement(static_sequence[i]);
            if(sequence[loaded_length]) {
                FURI_LOG_T(TAG_MOVEMENT_LOADER, "Loaded movement %lu", (uint32_t)loaded_length);
                loaded_length++;
            }
        }

        FURI_LOG_T(TAG_MOVEMENT_LOADER, "Loaded sequence length: %lu", (uint32_t)loaded_length);

        return loaded_length;
    }

private:
    MovementFactory& factory_;

    /**
   * @brief Load a single movement from static declaration
   */
    AgitationMovement* loadMovement(const AgitationMovementStatic& static_movement) {
        AgitationMovement* result = nullptr;

        switch(static_movement.type) {
        case AgitationMovementTypeCW:
            FURI_LOG_T(
                TAG_MOVEMENT_LOADER,
                "Creating CW movement with duration: %lu",
                (uint32_t)static_movement.duration);
            result = factory_.createCW(static_movement.duration);
            break;

        case AgitationMovementTypeCCW:
            FURI_LOG_T(
                TAG_MOVEMENT_LOADER,
                "Creating CCW movement with duration: %lu",
                (uint32_t)static_movement.duration);
            result = factory_.createCCW(static_movement.duration);
            break;

        case AgitationMovementTypePause:
            FURI_LOG_T(
                TAG_MOVEMENT_LOADER,
                "Creating pause movement with duration: %lu",
                (uint32_t)static_movement.duration);
            result = factory_.createPause(static_movement.duration);
            break;

        case AgitationMovementTypeLoop: {
            // Create temporary array for inner sequence
            AgitationMovement* inner_sequence[MAX_SEQUENCE_LENGTH];

            // Load the inner sequence
            FURI_LOG_T(
                TAG_MOVEMENT_LOADER,
                "Loading loop sequence with length: %lu",
                (uint32_t)static_movement.loop.sequence_length);
            size_t inner_length = loadSequence(
                static_movement.loop.sequence,
                static_movement.loop.sequence_length,
                inner_sequence);

            if(inner_length == 0) {
                FURI_LOG_T(TAG_MOVEMENT_LOADER, "Failed to load inner sequence");
                return nullptr;
            }

            // Create the loop movement
            FURI_LOG_T(
                TAG_MOVEMENT_LOADER,
                "Creating loop movement with count: %lu, max_duration: %lu",
                (uint32_t)static_movement.loop.count,
                (uint32_t)static_movement.loop.max_duration);

            result = factory_.createLoop(
                const_cast<const AgitationMovement**>(inner_sequence),
                inner_length,
                static_movement.loop.count,
                static_movement.loop.max_duration);
            break;
        }

        case AgitationMovementTypeWaitUser:
            result = factory_.createWaitUser();
            break;

        default:
            return nullptr;
        }

        if(!result) {
            FURI_LOG_T(
                TAG_MOVEMENT_LOADER,
                "Failed to create movement of type %lu",
                (uint32_t)static_movement.type);
        }
        return result;
    }
};
