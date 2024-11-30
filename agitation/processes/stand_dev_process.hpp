#pragma once

#include "common_sequences.hpp"

//------------------------------------------------------------------------------
// Stand Development Process
//------------------------------------------------------------------------------

/**
 * @brief Stand Development Initial Agitation Step
 */
static const AgitationMovementStatic STAND_DEV_INITIAL_SEQUENCE[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 3,
         .max_duration = 0,
         .sequence = (const struct AgitationMovementStatic*)STANDARD_INVERSION,
         .sequence_length = STANDARD_INVERSION_LENGTH}}};

static const AgitationStepStatic STAND_DEV_INITIAL_STEP = {
    .name = "Initial Agitation",
    .description = "Initial agitation before long stand period",
    .temperature = 20.0f,
    .sequence = STAND_DEV_INITIAL_SEQUENCE,
    .sequence_length = 1};

/**
 * @brief Stand Development Long Stand Step
 */
static const AgitationMovementStatic STAND_DEV_LONG_STAND_SEQUENCE[] = {
    {.type = AgitationMovementTypePause, .duration = 3600} // 1 hour stand
};

static const AgitationStepStatic STAND_DEV_LONG_STAND_STEP = {
    .name = "Long Stand",
    .description = "Extended period with minimal agitation",
    .temperature = 20.0f,
    .sequence = STAND_DEV_LONG_STAND_SEQUENCE,
    .sequence_length = 1};

// Stand Development Static Steps
static const AgitationStepStatic STAND_DEV_STEPS[] = {
    STAND_DEV_INITIAL_STEP,
    STAND_DEV_LONG_STAND_STEP};

/**
 * @brief Stand Development Process
 */
static const AgitationProcessStatic STAND_DEV_STATIC = {
    .process_name = "Black and White Stand Development",
    .film_type = "Black and White Negative",
    .tank_type = "Developing Tank",
    .chemistry = "B&W Developer",
    .temperature = 20.0f,
    .steps = STAND_DEV_STEPS,
    .steps_length = 2};
