#pragma once

#include "common_sequences.hpp"

//------------------------------------------------------------------------------
// Continuous Gentle Agitation Process
//------------------------------------------------------------------------------

/**
 * @brief Continuous gentle agitation (for C41/E6)
 */
static const AgitationMovementStatic CONTINUOUS_GENTLE[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 0, // Continuous
         .max_duration = 0,
         .sequence = (const struct AgitationMovementStatic*)CONTINUOUS_GENTLE_SEQ,
         .sequence_length = CONTINUOUS_GENTLE_SEQ_LENGTH}}};
static const size_t CONTINUOUS_GENTLE_LENGTH = 1;

/**
 * @brief Continuous Gentle Agitation Step
 */
static const AgitationStepStatic CONTINUOUS_GENTLE_STEP = {
    .name = "Continuous Gentle Agitation",
    .description = "Gentle, continuous movement for consistent development",
    .temperature = 38.0f, // Typical color development temperature
    .sequence = CONTINUOUS_GENTLE,
    .sequence_length = CONTINUOUS_GENTLE_LENGTH};

// Continuous Gentle Static Steps
static const AgitationStepStatic CONTINUOUS_GENTLE_STEPS[] = {CONTINUOUS_GENTLE_STEP};

/**
 * @brief Continuous Gentle Agitation Process
 */
static const AgitationProcessStatic CONTINUOUS_GENTLE_STATIC = {
    .process_name = "Continuous Gentle Agitation",
    .film_type = "Various",
    .tank_type = "Developing Tank",
    .chemistry = "Various",
    .temperature = 38.0f,
    .steps = CONTINUOUS_GENTLE_STEPS,
    .steps_length = 1};
