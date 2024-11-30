## UX / State Machine

- [x] Separate Running/Paused/WaitingForUser from the view states, since running process influences the state transitions
- [x] Transition for confirmation of waiting user state
- [x] Add a StopRequested event + Stop confirmation dialog (for example when exiting the app)
- [x] Add a ResumeProcess event + Resume confirmation dialog (for example when back from the paused view)
- [ ] Add a AcknowledgeCompleted event + AcknowledgeCompleted confirmation dialog
- [ ] Go to runtime settings from paused / user confirmation view
- [ ] Confirm restart hangs

## UI / pretty

- [ ] Make the UI pretty
- [ ] Add animations

## Process Interpreter

- [x] Add developer exhaustion scaling to the process itself
- [ ] Processes should be able to provide their own settings menu, instead of having an API for push/pull/roll count
- [ ] Implement adding additional time to the current process step (which is a separate thing in the model)
- [ ] Add possibility to inject events? things like ProcessCompleted, UserActionRequired? the way we currently do it is a bit messy in FimlDeveloperApp::update()
