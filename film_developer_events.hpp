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
    SkipStep = 13,
    RestartStep = 14,
    StopProcess = 15,
    ExitApp = 16,

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

    // Dispatch Dialog Events
    DispatchDialogDismissed = 70,
    DispatchDialogConfirmed = 71,
    DispatchDialogRequested = 90,

    // State Machine Events
    StateChanged = 80,

    // Pause Control Events
    PauseRequested = 100,
    ResumeRequested = 101,
    SkipRequested = 102,
    RestartRequested = 103,
    ExitRequested = 104,
    StopProcessRequested = 105,
};

inline const char* get_event_name(FilmDeveloperEvent event) {
    switch(event) {
    case FilmDeveloperEvent::ProcessSelected:
        return "ProcessSelected";
    case FilmDeveloperEvent::SettingsConfirmed:
        return "SettingsConfirmed";
    case FilmDeveloperEvent::ProcessAborted:
        return "ProcessAborted";
    case FilmDeveloperEvent::ProcessCompleted:
        return "ProcessCompleted";
    case FilmDeveloperEvent::StartProcess:
        return "StartProcess";
    case FilmDeveloperEvent::PauseProcess:
        return "PauseProcess";
    case FilmDeveloperEvent::ResumeProcess:
        return "ResumeProcess";
    case FilmDeveloperEvent::SkipStep:
        return "SkipStep";
    case FilmDeveloperEvent::RestartStep:
        return "RestartStep";
    case FilmDeveloperEvent::StopProcess:
        return "StopProcess";
    case FilmDeveloperEvent::ExitApp:
        return "ExitApp";
    case FilmDeveloperEvent::UserActionRequired:
        return "UserActionRequired";
    case FilmDeveloperEvent::UserActionConfirmed:
        return "UserActionConfirmed";
    case FilmDeveloperEvent::TimerTick:
        return "TimerTick";
    case FilmDeveloperEvent::StepComplete:
        return "StepComplete";
    case FilmDeveloperEvent::MotorStateChanged:
        return "MotorStateChanged";
    case FilmDeveloperEvent::AgitationComplete:
        return "AgitationComplete";
    case FilmDeveloperEvent::PushPullChanged:
        return "PushPullChanged";
    case FilmDeveloperEvent::RollCountChanged:
        return "RollCountChanged";
    case FilmDeveloperEvent::EnterRuntimeSettings:
        return "EnterRuntimeSettings";
    case FilmDeveloperEvent::ExitRuntimeSettings:
        return "ExitRuntimeSettings";
    case FilmDeveloperEvent::StepDurationChanged:
        return "StepDurationChanged";
    case FilmDeveloperEvent::StateChanged:
        return "StateChanged";

    case FilmDeveloperEvent::DispatchDialogRequested:
        return "DispatchDialogRequested";
    case FilmDeveloperEvent::DispatchDialogDismissed:
        return "DispatchDialogDismissed";
    case FilmDeveloperEvent::DispatchDialogConfirmed:
        return "DialogConfirmed";
    case FilmDeveloperEvent::PauseRequested:
        return "PauseRequested";
    case FilmDeveloperEvent::ResumeRequested:
        return "ResumeRequested";
    case FilmDeveloperEvent::SkipRequested:
        return "SkipRequested";
    case FilmDeveloperEvent::RestartRequested:
        return "RestartRequested";
    case FilmDeveloperEvent::ExitRequested:
        return "ExitRequested";
    case FilmDeveloperEvent::StopProcessRequested:
        return "StopProcessRequested";
    }

    return "Unknown";
}
