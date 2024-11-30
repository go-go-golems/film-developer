#pragma once

#include "common_sequences.hpp"

//------------------------------------------------------------------------------
// B&W Development Sequences as Steps
//------------------------------------------------------------------------------

/**
 * @brief Standard B&W Initial Agitation Step
 */
static const AgitationMovementStatic INITIAL_AGITATION[] = {
    {.type = AgitationMovementTypeLoop,
     .loop =
         {
             .count = 4,
             .max_duration = 0,
             .sequence = (const struct AgitationMovementStatic*)STANDARD_INVERSION,
             .sequence_length = STANDARD_INVERSION_LENGTH,
         }},
    {.type = AgitationMovementTypePause, .duration = 24},
};
static const size_t INITIAL_AGITATION_LENGTH = 2;

static const AgitationStepStatic BW_INITIAL_AGITATION_STEP = {
    .name = "Initial Agitation",
    .description = "First round of agitation to ensure even development",
    .temperature = 20.0f,
    .sequence = INITIAL_AGITATION,
    .sequence_length = INITIAL_AGITATION_LENGTH};

/**
 * @brief Standard B&W Periodic Agitation Step
 */
static const AgitationMovementStatic BW_PERIODIC_AGITATION_SEQUENCE[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 2,
         .max_duration = 0,
         .sequence = (const struct AgitationMovementStatic*)STANDARD_INVERSION,
         .sequence_length = STANDARD_INVERSION_LENGTH}}};

static const AgitationStepStatic BW_PERIODIC_AGITATION_STEP = {
    .name = "Periodic Agitation",
    .description = "Continued agitation during development",
    .temperature = 20.0f,
    .sequence = BW_PERIODIC_AGITATION_SEQUENCE,
    .sequence_length = 1};

// B&W Standard Development Static Steps
static const AgitationStepStatic BW_STANDARD_DEV_STEPS[] = {
    BW_INITIAL_AGITATION_STEP,
    BW_PERIODIC_AGITATION_STEP};

/**
 * @brief Standard B&W Development Process
 */
static const AgitationProcessStatic BW_STANDARD_DEV_STATIC = {
    .process_name = "Black and White Standard Development",
    .film_type = "Black and White Negative",
    .tank_type = "Developing Tank",
    .chemistry = "B&W Developer",
    .temperature = 20.0f,
    .steps = BW_STANDARD_DEV_STEPS,
    .steps_length = 2};
