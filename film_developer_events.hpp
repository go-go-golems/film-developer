#pragma once

#include <inttypes.h>

enum class FilmDeveloperEvent : uint32_t {
    // Navigation Events
    ProcessSelected = 0,
    SettingsConfirmed = 1,
    ProcessAborted = 2,
    ProcessCompleted = 3,

    // Process Control Events
    StartProcess = 10,
    PauseProcess = 11,
    ResumeProcess = 12,

    // User Intervention Events
    UserActionRequired = 20,
    UserActionConfirmed = 21,

    // Timer Events
    TimerTick = 30,
    StepComplete = 31,

    // Motor Control Events
    MotorStateChanged = 40,
    AgitationComplete = 41,

    // Settings Events
    PushPullChanged = 50,
    RollCountChanged = 51,
    
    // Runtime Control Events
    EnterRuntimeSettings = 60,
    ExitRuntimeSettings = 61,
    StepDurationChanged = 62,
    SkipStep = 63,
    RestartStep = 64,
    
    // Dialog Events
    DialogDismissed = 70,
    DialogConfirmed = 71,
    
    // State Machine Events
    StateChanged = 80,
    
    // Dispatch Events
    DispatchRequested = 90,
    
    // Pause Control Events
    PauseRequested = 100,
    ResumeRequested = 101
}; 