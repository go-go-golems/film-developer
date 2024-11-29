# Film Developer View Specification

## Architecture Overview

The application will follow the architecture pattern shown in example_cpp_view, using:

- A main application class (`FilmDeveloperApp`) managing view dispatch
- Individual view classes for each screen
- A shared view model for state management
- Clear separation between UI and business logic

## View Classes

### 1. Main Development View (ViewCpp)

Primary interface showing:

- Process title
- Current step information
- Status/timer information
- Movement state
- Pin states
- Control buttons:
  - Center: Menu
  - Back: Exit confirmation
  - Left: Restart step
  - Right: Skip step

### 2. Process Selection View (SubMenuCpp)

Simple menu with:

- C41 Color Process
- B&W Process
- E6 Process
- Back: Exit app

### 3. Settings View (VariableItemListCpp)

Single scrollable list with adjustable parameters:

- Push/Pull (-2 to +2 stops)
  - Fixed values: "-2", "-1", "0", "+1", "+2"
  - Default: "0"
- Roll Count (1-100)
  - Default: "1"
  - Long press left/right for faster adjustment
- Confirm: Start process
- Back: Return to process selection

### 4. Paused View (ViewCpp)

Dedicated pause screen showing:

- PAUSED header
- Current step information
- Elapsed time
- Control buttons:
  - Center: Menu
  - Right: Resume
  - Left: Settings
  - Back: Exit confirmation

### 5. Dialog System

#### a. Confirmation Dialog (DialogExCpp)
- Process start/abort confirmation
- Step completion requiring user intervention
- Exit confirmation during active process
- Step restart confirmation
- Step skip confirmation

#### b. Runtime Settings Dialog (ViewCpp)
- Adjust current step duration
- Back: Return to previous view

#### c. Dispatch Menu (SubMenuCpp)
- Pause/Resume process
- Runtime Settings
- Restart Step
- Skip Step
- Exit Process
- Back: Return to previous view

### 6. Waiting For User View (ViewCpp)

Dedicated user confirmation screen showing:
- WAITING FOR CONFIRMATION header
- Current step information
- Action required description
- Control buttons:
  - Center: Menu (opens dispatch dialog)
  - Right: Confirm and continue
  - Back: Exit confirmation

## View Navigation Flow

```mermaid
graph TD
    PS[Process Selection View] -->|Select Process| S[Settings View]
    S -->|Confirm Settings| MD[Main Development View]
    MD -->|Menu| DD[Dispatch Menu]
    MD -->|Back| CD1[Confirmation Dialog]
    MD -->|Pause| PV[Paused View]
    
    PV -->|Menu| DD
    PV -->|Resume| MD
    PV -->|Settings| RS[Runtime Settings]
    PV -->|Back| CD1
    
    DD -->|Back| MD
    DD -->|Pause| PV
    DD -->|Settings| RS
    DD -->|Restart| CD2[Confirmation Dialog]
    DD -->|Skip| CD3[Confirmation Dialog]
    DD -->|Exit| CD1
    
    RS -->|Back| MD
    
    CD1 -->|Confirm| PS
    CD1 -->|Cancel| MD
    CD2 -->|Confirm| MD
    CD2 -->|Cancel| DD
    CD3 -->|Confirm| MD
    CD3 -->|Cancel| DD
```

## State Machines

### Process State Machine (Model Layer)

```mermaid
stateDiagram-v2
    [*] --> NotStarted
    NotStarted --> Running: Start
    Running --> Paused: Pause
    Paused --> Running: Resume
    Running --> WaitingForUser: UserActionNeeded
    WaitingForUser --> Running: UserConfirmed
    Running --> [*]: Complete
    Paused --> [*]: Abort
    WaitingForUser --> [*]: Abort
```

### Application State Machine (Updated)

```mermaid
stateDiagram-v2
    [*] --> ProcessSelection
    ProcessSelection --> Settings: ProcessSelected
    Settings --> MainView: SettingsConfirmed
    
    state MainView {
        [*] --> ShowingMainDevelopment
        ShowingMainDevelopment --> ShowingWaitingConfirmation: ProcessState.WaitingForUser
        ShowingWaitingConfirmation --> ShowingMainDevelopment: ProcessState.Running
        ShowingMainDevelopment --> ShowingPaused: ProcessState.Paused
        ShowingPaused --> ShowingMainDevelopment: ProcessState.Running
    }
    
    MainView --> DispatchDialog: DispatchRequested
    DispatchDialog --> MainView: DialogDismissed
    
    MainView --> RuntimeSettings: EnterRuntimeSettings
    RuntimeSettings --> MainView: ExitRuntimeSettings
    
    MainView --> ConfirmRestart: RestartRequested
    ConfirmRestart --> MainView: DialogDismissed
    
    MainView --> ConfirmSkip: SkipRequested
    ConfirmSkip --> MainView: DialogDismissed
    
    MainView --> ConfirmExit: ExitRequested
    ConfirmExit --> ProcessSelection: DialogConfirmed
    ConfirmExit --> MainView: DialogDismissed
    
    MainView --> ProcessSelection: ProcessCompleted
    ProcessSelection --> [*]: Exit
```

### View Navigation with Process States

```mermaid
stateDiagram-v2
    [*] --> ViewProcessSelection
    ViewProcessSelection --> ViewSettings: ProcessSelected
    ViewSettings --> ViewMainDevelopment: SettingsConfirmed
    
    state ViewMainDevelopment {
        [*] --> ShowingMain
        ShowingMain --> ShowingWaitingUser: ProcessState.WaitingForUser
        ShowingWaitingUser --> ShowingMain: ProcessState.Running
        ShowingMain --> ShowingPaused: ProcessState.Paused
        ShowingPaused --> ShowingMain: ProcessState.Running
    }
    
    ViewMainDevelopment --> ViewDispatchMenu: DispatchRequested
    ViewDispatchMenu --> ViewMainDevelopment: DialogDismissed
    
    ViewMainDevelopment --> ViewRuntimeSettings: EnterRuntimeSettings
    ViewRuntimeSettings --> ViewMainDevelopment: ExitRuntimeSettings
    
    ViewMainDevelopment --> ViewConfirmationDialog: RestartRequested
    ViewMainDevelopment --> ViewConfirmationDialog: SkipRequested
    ViewConfirmationDialog --> ViewMainDevelopment: DialogDismissed
    
    ViewMainDevelopment --> ViewConfirmationDialog: ExitRequested
    ViewConfirmationDialog --> ViewProcessSelection: DialogConfirmed
    ViewConfirmationDialog --> ViewMainDevelopment: DialogDismissed
    
    ViewMainDevelopment --> ViewProcessSelection: ProcessCompleted
    ViewProcessSelection --> [*]: Exit
```

## View-State-Process Mapping

| View ID | Application State | Valid Process States |
|---------|------------------|---------------------|
| ViewProcessSelection | ProcessSelection | NotStarted |
| ViewSettings | Settings | NotStarted |
| ViewMainDevelopment | ShowingMainDevelopment | Running |
| ViewPaused | ShowingPaused | Paused |
| ViewWaitingConfirmation | ShowingWaitingConfirmation | WaitingForUser |
| ViewConfirmationDialog | ConfirmRestart/ConfirmSkip/ConfirmExit | Any |
| ViewDispatchMenu | DispatchDialog | Any |
| ViewRuntimeSettings | RuntimeSettings | Any |

## Button Behavior Matrix

| View | Process State | Center Button | Right Button | Left Button | Back Button |
|------|--------------|---------------|--------------|-------------|-------------|
| Main | Running | Dispatch Menu | Skip Step | Restart Step | Exit Confirm |
| Main | Paused | Dispatch Menu | Resume | Settings | Exit Confirm |
| Main | WaitingForUser | Dispatch Menu | Confirm | - | Exit Confirm |
| Paused | Paused | Dispatch Menu | Resume | Settings | Exit Confirm |
| WaitingConfirmation | WaitingForUser | Dispatch Menu | Confirm | - | Exit Confirm |

## View-State Mapping

| View ID | Associated States |
|---------|------------------|
| ViewProcessSelection | ProcessSelection |
| ViewSettings | Settings |
| ViewMainDevelopment | MainView |
| ViewPaused | Paused |
| ViewConfirmationDialog | WaitingConfirmation, ConfirmRestart, ConfirmSkip, ConfirmExit |
| ViewDispatchMenu | DispatchDialog |
| ViewRuntimeSettings | RuntimeSettings |

## View Navigation Rules

1. **Main Development View**
   - Menu → Dispatch Menu
   - Back → Exit Confirmation Dialog
   - Left → Restart Confirmation
   - Right → Skip Confirmation

2. **Paused View**
   - Menu → Dispatch Menu
   - Resume → Main Development View
   - Settings → Runtime Settings
   - Back → Exit Confirmation Dialog

3. **Dispatch Menu**
   - Back → Previous View (Main or Paused)
   - Actions trigger appropriate transitions

4. **Runtime Settings**
   - Back → Previous View (Main or Paused)

5. **Confirmation Dialogs**
   - Confirm → Appropriate action
   - Cancel → Previous View
