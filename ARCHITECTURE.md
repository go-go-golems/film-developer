# Film Developer Application Architecture

## Overview

The Film Developer application is built using a modern C++ architecture with proper separation of concerns. It follows a view-based architecture with centralized navigation and state management.

## Core Components

### 1. Application Class (`FilmDeveloperApp`)

The main application class that manages:
- View lifecycle and transitions
- Event handling and routing
- Resource management
- Motor controller initialization

Key features:
- View mapping system for navigation
- Previous view tracking for back navigation
- Centralized event handling
- Proper resource cleanup

### 2. Views

All views inherit from `flipper::ViewCpp` through specialized base classes:

#### ProcessSelectionView (inherits from `SubMenuCpp`)
- Purpose: Select film development process
- Features:
  - Static list of available processes
  - Process selection callback
  - Navigation to settings on selection
- State:
  - Currently selected process
  - Available processes list

#### SettingsView (inherits from `VariableItemListCpp`)
- Purpose: Configure development settings
- Features:
  - Push/Pull adjustment (-2 to +2 stops)
  - Roll count setting (1-100)
  - Settings confirmation
- State:
  - Current push/pull value
  - Current roll count
  - Settings model

#### MainDevelopmentView (inherits from `ViewCpp`)
- Purpose: Main development process screen
- Features:
  - Process status display
  - Timer management
  - Motor control
  - Step progression
- State:
  - Current process step
  - Timer state
  - Motor controller state
  - Process status

#### ConfirmationDialogView (inherits from `DialogExCpp`)
- Purpose: User confirmations
- Features:
  - Process start confirmation
  - Process abort confirmation
  - Step completion confirmation
- State:
  - Current dialog type
  - Dialog result handling

### 3. Models

#### MainViewModel
- Purpose: Main development screen state
- Properties:
  - `current_process`: Current development process
  - `process_active`: Process running state
  - `paused`: Pause state
  - Status text fields:
    - `status_text`: Current status
    - `step_text`: Current step
    - `movement_text`: Movement state
- Methods:
  - `update_status()`: Update timer display
  - `update_step_text()`: Update step information
  - `update_movement_text()`: Update motor state
  - `reset()`: Reset to initial state

#### ProcessSettingsModel
- Purpose: Development settings state
- Properties:
  - `push_pull_stops`: Push/pull adjustment (-2 to +2)
  - `roll_count`: Number of rolls (1-100)
  - Static process definitions
- Methods:
  - `get_time_adjustment()`: Calculate time factor
  - `get_push_pull_text()`: Get formatted push/pull value
  - `reset()`: Reset to defaults

### 4. Events (`FilmDeveloperEvent`)

Navigation and state change events:
```cpp
enum class FilmDeveloperEvent : uint32_t {
    // Navigation Events
    ProcessSelected = 0,      // Process chosen in selection view
    SettingsConfirmed = 1,    // Settings confirmed, ready to start
    ProcessAborted = 2,       // Development process aborted
    ProcessCompleted = 3,     // Development process finished

    // Process Control Events
    StartProcess = 10,        // Begin development
    PauseProcess = 11,        // Pause current process
    ResumeProcess = 12,       // Resume paused process

    // User Intervention Events
    UserActionRequired = 20,  // User action needed
    UserActionConfirmed = 21, // User confirmed action

    // Timer Events
    TimerTick = 30,          // Timer update
    StepComplete = 31,       // Process step completed

    // Motor Control Events
    MotorStateChanged = 40,   // Motor state update
    AgitationComplete = 41,   // Agitation cycle completed

    // Settings Events
    PushPullChanged = 50,     // Push/pull setting changed
    RollCountChanged = 51     // Roll count changed
};
```

### 5. View Navigation Flow

1. Application Start
   - Opens ProcessSelectionView
   - User selects development process
   - Triggers `ProcessSelected` event

2. Settings Configuration
   - Transitions to SettingsView
   - User adjusts push/pull and roll count
   - Confirms with `SettingsConfirmed` event

3. Development Process
   - Moves to MainDevelopmentView
   - Process runs with timer updates
   - User can pause/resume/abort
   - Step completions trigger confirmations

4. Dialog Interactions
   - ConfirmationDialogView shown as needed
   - Returns to previous view on confirmation
   - Aborts to process selection on cancellation

### 6. Resource Management

The application handles several types of resources:

1. View Resources
   - Each view manages its own UI elements
   - Views are initialized in sequence
   - Proper cleanup in view destructors

2. Motor Controller
   - Initialized in app constructor
   - Platform-specific implementation (Host/Embedded)
   - Proper GPIO management
   - Cleanup in destructor

3. Timer Management
   - Process timer in MainDevelopmentView
   - Automatic cleanup on view destruction

4. Memory Management
   - Static allocations preferred
   - Limited dynamic allocation
   - RAII principles followed
   - Proper cleanup in destructors

## Best Practices

1. View Management
   - Always use view transitions through ViewMap
   - Handle enter/exit callbacks properly
   - Clean up resources in destructors

2. Event Handling
   - Use typed events (FilmDeveloperEvent)
   - Handle all event cases
   - Provide appropriate feedback

3. State Management
   - Keep state in appropriate models
   - Use RAII for resource management
   - Maintain clear state transitions

4. Error Handling
   - Validate all user inputs
   - Provide clear error feedback
   - Handle edge cases gracefully

## Future Considerations

1. Potential Improvements
   - Additional development processes
   - Temperature compensation
   - Process history
   - Custom process creation

2. Extension Points
   - New view types
   - Additional settings
   - Enhanced motor control
   - Process customization 