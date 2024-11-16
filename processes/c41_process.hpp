#pragma once

#include "common_sequences.hpp"

//------------------------------------------------------------------------------
// Color Development Sequences - C41 Process
//------------------------------------------------------------------------------

// Number of rolls being developed (affects development time)
static const int NUMBER_OF_ROLLS = 10; // Adjust this value as needed

// Base development time in seconds (3.5 minutes = 210 seconds)
static const double BASE_DEVELOPER_TIME = 210.0;

/**
 * @brief C41 Color Developer Stage (Constant CW Agitation)
 */
static const AgitationMovementStatic C41_COLOR_DEVELOPER[] = {
    {.type = AgitationMovementTypeCW,
     .duration = static_cast<uint32_t>(BASE_DEVELOPER_TIME *
                                       pow(1.02f, NUMBER_OF_ROLLS - 1))},
    {.type = AgitationMovementTypeWaitUser,
     .message = "Development complete. Ready for blix?"},
};
static const size_t C41_COLOR_DEVELOPER_LENGTH = 2;

/**
 * @brief C41 Bleach/Fix (Blix) Stage (Constant CW Agitation)
 */
static const AgitationMovementStatic C41_BLEACH_SEQUENCE[] = {
    {.type = AgitationMovementTypeCW, .duration = 8 * 60},
    {.type = AgitationMovementTypeWaitUser,
     .message = "Blix complete. Process finished!"},
};
static const size_t C41_BLEACH_LENGTH = 3;

//------------------------------------------------------------------------------
// C41 Process Steps
//------------------------------------------------------------------------------

/**
 * @brief C41 Color Developer Step
 */
static const AgitationStepStatic C41_COLOR_DEVELOPER_STEP = {
    .name = "Color Developer",
    .description =
        "Main color development stage with continuous gentle agitation",
    .temperature = 38.0f,
    .sequence = C41_COLOR_DEVELOPER,
    .sequence_length = C41_COLOR_DEVELOPER_LENGTH};

/**
 * @brief C41 Bleach Step
 */
static const AgitationStepStatic C41_BLEACH_STEP = {
    .name = "Bleach",
    .description = "Bleach stage with periodic gentle agitation",
    .temperature = 38.0f,
    .sequence = C41_BLEACH_SEQUENCE,
    .sequence_length = C41_BLEACH_LENGTH};

// C41 Full Process Static Steps (removed pre-wash and stabilizer)
static const AgitationStepStatic C41_FULL_PROCESS_STEPS[] = {
    C41_COLOR_DEVELOPER_STEP, C41_BLEACH_STEP};

/**
 * @brief Complete C41 Development Process
 */
static const AgitationProcessStatic C41_FULL_PROCESS_STATIC = {
    .process_name = "C41 Color Film Development",
    .film_type = "Color Negative",
    .tank_type = "Developing Tank",
    .chemistry = "C41 Color Chemistry",
    .temperature = 38.0f,
    .steps = C41_FULL_PROCESS_STEPS,
    .steps_length = 2};
