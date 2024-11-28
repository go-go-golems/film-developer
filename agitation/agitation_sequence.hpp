#pragma once

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>
#include "debug.hpp"

#include "host_helpers.hpp"

#define TAG_AGITATION_SEQUENCE "AgitationSequence"

/**
 * @brief Movement types for agitation sequence
 */
typedef enum {
    AgitationMovementTypeCW = 0, // Clockwise movement
    AgitationMovementTypeCCW = 1, // Counter-clockwise movement
    AgitationMovementTypePause = 2, // Pause/wait
    AgitationMovementTypeLoop = 3, // Repeating sequence
    AgitationMovementTypeWaitUser = 4, // Wait for user interaction
} AgitationMovementType;

//------------------------------------------------------------------------------
// Static (const) versions of structures
//------------------------------------------------------------------------------

typedef struct AgitationMovementStatic AgitationMovementStatic;
typedef struct AgitationStepStatic AgitationStepStatic;

/**
 * @brief Static version of movement
 * For loops, duration is ignored. For other types, count and sequence are ignored.
 */
struct AgitationMovementStatic {
    AgitationMovementType type;
    union {
        // For regular movements
        uint32_t duration;
        // For loops
        struct {
            uint32_t count; // 0 = use max_duration or infinite
            uint32_t max_duration; // 0 = use count or infinite
            const AgitationMovementStatic* sequence;
            size_t sequence_length;
        } loop;
        // For user wait
        const char* message; // Optional message to display
    };
};

/**
 * @brief Static version of step
 */
struct AgitationStepStatic {
    const char* name;
    const char* description;
    float temperature;
    const AgitationMovementStatic* sequence;
    size_t sequence_length;
};

/**
 * @brief Static version of process
 */
typedef struct {
    const char* process_name;
    const char* film_type;
    const char* tank_type;
    const char* chemistry;
    float temperature;
    const AgitationStepStatic* steps;
    size_t steps_length;
} AgitationProcessStatic;

//------------------------------------------------------------------------------
// Dynamic versions of structures (with FuriString)
//------------------------------------------------------------------------------

/**
 * @brief Dynamic version of movement
 */
typedef struct AgitationMovement_ AgitationMovement_;

struct AgitationMovement_ {
    AgitationMovementType type;
    union {
        // For regular movements
        uint32_t duration;
        // For loops
        struct {
            uint32_t count;
            struct AgitationMovement_* sequence;
            size_t sequence_length;
        } loop;
    };

#ifdef HOST
    void print() const {
        switch(type) {
        case AgitationMovementTypeCW:
            FURI_LOG_D(
                TAG_AGITATION_SEQUENCE,
                "CW movement, duration: %u",
                duration);
            break;
        case AgitationMovementTypeCCW:
            FURI_LOG_D(
                TAG_AGITATION_SEQUENCE,
                "CCW movement, duration: %u",
                duration);
            break;
        case AgitationMovementTypePause:
            FURI_LOG_D(
                TAG_AGITATION_SEQUENCE,
                "Pause, duration: %u",
                duration);
            break;
        case AgitationMovementTypeLoop:
            FURI_LOG_D(
                TAG_AGITATION_SEQUENCE,
                "Loop, count: %u, sequence length: %zu",
                loop.count,
                loop.sequence_length);
            break;
        case AgitationMovementTypeWaitUser:
            FURI_LOG_D(
                TAG_AGITATION_SEQUENCE,
                "Wait for user");
            break;
        }
    }
#endif
};

/**
 * @brief Dynamic version of step
 */
typedef struct {
    FuriString* name;
    FuriString* description;
    float temperature;
    AgitationMovement_* sequence;
    size_t sequence_length;

#ifdef HOST
    void print() const {
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Step: %s",
            name->getData());
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Description: %s",
            description->getData());
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Temperature: %.1f°C",
            temperature);
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Sequence (%zu movements):",
            sequence_length);
        for(size_t i = 0; i < sequence_length; i++) {
            sequence[i].print();
        }
    }
#endif
} AgitationStep;

/**
 * @brief Dynamic version of process
 */
typedef struct {
    FuriString* process_name;
    FuriString* film_type;
    FuriString* tank_type;
    FuriString* chemistry;
    float temperature;
    AgitationStep* steps;
    size_t steps_length;

#ifdef HOST
    void print() const {
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Process: %s",
            process_name->getData());
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Film Type: %s",
            film_type->getData());
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Tank Type: %s",
            tank_type->getData());
        FURI_LOG_D(TAG_AGITATION_SEQUENCE, "Chemistry: %s", chemistry->getData());
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Temperature: %.1f°C",
            temperature);
        FURI_LOG_D(
            TAG_AGITATION_SEQUENCE,
            "Steps (%zu):",
            steps_length);
        for(size_t i = 0; i < steps_length; i++) {
            steps[i].print();
        }
    }
#endif
} AgitationProcess;

//------------------------------------------------------------------------------
// Example of static initialization
//------------------------------------------------------------------------------

/**
 * @brief Standard inversion sequence
 */
#define AGITATION_STANDARD_INVERSION                         \
    {                                                        \
        {.type = AgitationMovementTypeCW, .duration = 1},    \
        {.type = AgitationMovementTypePause, .duration = 1}, \
        {.type = AgitationMovementTypeCCW, .duration = 1},   \
        {.type = AgitationMovementTypePause, .duration = 1}, \
    }

/**
 * @brief Initial agitation sequence with loop
 */
#define AGITATION_INITIAL_SEQUENCE                                         \
    {                                                                      \
        {.type = AgitationMovementTypeLoop,                                \
         .loop =                                                           \
             {.count = 4,                                                  \
              .max_duration = 0,                                           \
              .sequence =                                                  \
                  (AgitationMovementStatic[]){                             \
                      {.type = AgitationMovementTypeCW, .duration = 1},    \
                      {.type = AgitationMovementTypePause, .duration = 1}, \
                      {.type = AgitationMovementTypeCCW, .duration = 1},   \
                      {.type = AgitationMovementTypePause, .duration = 1}, \
                  },                                                       \
              .sequence_length = 4}},                                      \
        {.type = AgitationMovementTypePause, .duration = 24},              \
    }

//------------------------------------------------------------------------------
// Conversion functions
//------------------------------------------------------------------------------

/**
 * @brief Convert static process to dynamic
 * @param static_process Static process definition
 * @return AgitationProcess* Newly allocated dynamic process
 */
AgitationProcess* agitation_process_from_static(const AgitationProcessStatic* static_process);

/**
 * @brief Convert static step to dynamic
 * @param static_step Static step definition
 * @return AgitationStep* Newly allocated dynamic step
 */
AgitationStep* agitation_step_from_static(const AgitationStepStatic* static_step);

//------------------------------------------------------------------------------
// Original dynamic API functions
//------------------------------------------------------------------------------

AgitationProcess* agitation_process_alloc();
void agitation_process_free(AgitationProcess* process);
AgitationStep* agitation_step_alloc();
void agitation_step_free(AgitationStep* step);

void agitation_sequence_add_movement(
    AgitationMovement_** sequence,
    size_t* length,
    AgitationMovementType type,
    uint32_t duration);

uint32_t agitation_sequence_get_duration(AgitationMovement_* sequence, size_t length);
bool agitation_sequence_validate(AgitationMovement_* sequence, size_t length);
AgitationProcess* agitation_process_from_yaml(const char* yaml_content);
FuriString* agitation_process_to_yaml(AgitationProcess* process);
