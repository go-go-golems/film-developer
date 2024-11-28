#pragma once

#include "../agitation/agitation_sequence.hpp"

//------------------------------------------------------------------------------
// Common Base Sequences
//------------------------------------------------------------------------------

/**
 * @brief Basic inversion sequence (CW -> Pause -> CCW -> Pause)
 */
static const AgitationMovementStatic STANDARD_INVERSION[] = {
    {.type = AgitationMovementTypeCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
};
static const size_t STANDARD_INVERSION_LENGTH = 4;

/**
 * @brief Gentle continuous base sequence
 */
static const AgitationMovementStatic CONTINUOUS_GENTLE_SEQ[] = {
    {.type = AgitationMovementTypeCW, .duration = 2},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 2},
    {.type = AgitationMovementTypePause, .duration = 1},
};
static const size_t CONTINUOUS_GENTLE_SEQ_LENGTH = 4;
