# Film Developer Implementation Guide

## View Class Implementations

### 1. MainDevelopmentView

```cpp
class MainDevelopmentView : public flipper::ViewCpp {
private:
    MainViewModel* model;
    AgitationProcessInterpreter* process_interpreter;
    MotorController* motor_controller;
    ConfirmationDialogView* dialog;

    // Current implementation from film_developer.cpp:draw_callback
    void draw(Canvas* canvas) override {
        // Draw title, step info, status, movement state, pin states
        // Use model-> instead of direct access
    }

    // Current implementation from film_developer.cpp:input_callback
    bool handle_input(InputEvent* event) override {
        // Handle OK, Left, Right buttons
        // Delegate to process_interpreter and motor_controller
    }

    static void dialog_callback(DialogExResult result, void* context) {
        auto* view = static_cast<MainDevelopmentView*>(context);
        
        switch(result) {
            case DialogExResultRight:
                // Handle confirmation
                if (view->pending_action == PendingAction::Start) {
                    view->start_process();
                } else if (view->pending_action == PendingAction::Abort) {
                    view->abort_process();
                } else if (view->pending_action == PendingAction::StepComplete) {
                    view->advance_step();
                }
                break;
                
            case DialogExResultLeft:
                // Handle cancellation - return to previous state
                break;
        }
    }
};
```

### 2. ProcessSelectionView

```cpp
class ProcessSelectionView : public flipper::SubMenuCpp {
private:
    const std::vector<AgitationProcessStatic*> available_processes = {
        &C41_FULL_PROCESS_STATIC,
        &BW_PROCESS_STATIC,
        &E6_PROCESS_STATIC
    };

    void init() override {
        add_item("C41 Color Process", process_callback, this);
        add_item("B&W Process", process_callback, this);
        add_item("E6 Process", process_callback, this);
    }
};
```

### 3. SettingsView

```cpp
class SettingsView : public flipper::VariableItemListCpp {
private:
    ProcessSettingsModel* settings;

    void init() override {
        add_variable_item(
            "Push/Pull",
            5,  // Number of options (-2 to +2)
            settings_callback,
            this);

        add_variable_item(
            "Roll Count",
            100,  // Max value
            roll_count_callback,
            this);
    }
};
```

### 4. ConfirmationDialogView

```cpp
class ConfirmationDialogView : public flipper::DialogExCpp {
private:
    DialogExResultCallback callback;
    void* callback_context;

    void init() override {
        DialogExCpp::init();
        // Default initialization - specific text set when shown
    }

    void configure_for_start() {
        set_header("Start Process", 64, 10, AlignCenter, AlignCenter);
        set_text("Begin development process?\nSettings will be locked.", 
                64, 32, AlignCenter, AlignCenter);
        set_left_button_text("Cancel");
        set_right_button_text("Start");
    }

    void configure_for_abort() {
        set_header("Abort Process", 64, 10, AlignCenter, AlignCenter);
        set_text("Stop current development?\nProgress will be lost.", 
                64, 32, AlignCenter, AlignCenter);
        set_left_button_text("Continue");
        set_right_button_text("Stop");
    }

    void configure_for_step() {
        set_header("Step Complete", 64, 10, AlignCenter, AlignCenter);
        set_text("Current step finished.\nProceed to next step?", 
                64, 32, AlignCenter, AlignCenter);
        set_left_button_text("Wait");
        set_right_button_text("Next");
    }

public:
    enum class DialogType {
        ProcessStart,
        ProcessAbort,
        StepComplete
    };

    void show_dialog(DialogType type, DialogExResultCallback cb, void* context) {
        callback = cb;
        callback_context = context;

        switch(type) {
            case DialogType::ProcessStart:
                configure_for_start();
                break;
            case DialogType::ProcessAbort:
                configure_for_abort();
                break;
            case DialogType::StepComplete:
                configure_for_step();
                break;
        }

        set_result_callback(callback);
    }
};
```

## Model Implementations

### 1. MainViewModel

```cpp
class MainViewModel {
public:
    // Current process state (from film_developer.cpp)
    const AgitationProcessStatic* current_process{nullptr};
    bool process_active{false};
    bool paused{false};

    // Display information (from film_developer.cpp)
    char status_text[64]{};
    char step_text[32]{};
    char movement_text[32]{};

    // New fields for enhanced functionality
    int8_t push_pull_stops{0};
    uint8_t roll_count{1};

    void update_status(uint32_t elapsed, uint32_t duration) {
        snprintf(status_text, sizeof(status_text),
                "%s Time: %us/%us",
                paused ? "[PAUSED]" : "",
                elapsed, duration);
    }
};
```

## Main Application Class

```cpp
class FilmDeveloperApp {
private:
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    MainViewModel model;
    ProcessSettingsModel settings;

    // Views
    MainDevelopmentView main_view;
    ProcessSelectionView process_view;
    SettingsView settings_view;

    // Current implementation from film_developer_app()
    void init() {
        // Initialize ViewDispatcher
        // Setup views
        // Configure navigation
    }
};
```

## Code Migration Guide

### Phase 1: View Separation

1. Create header files for each view class
2. Move relevant code from film_developer.cpp:
   - draw_callback -> MainDevelopmentView::draw()
   - input_callback -> MainDevelopmentView::handle_input()
   - timer_callback -> MainDevelopmentView::timer_callback()

### Phase 2: Model Implementation

1. Create model header files
2. Move state from FilmDeveloperApp struct:

   ```cpp
   // Before:
   typedef struct {
     bool process_active;
     char status_text[64];
     // ...
   } FilmDeveloperApp;

   // After:
   class MainViewModel {
     bool process_active;
     char status_text[64];
     // ...
   };
   ```

### Phase 3: ViewDispatcher Integration

1. Update main app initialization:

   ```cpp
   void FilmDeveloperApp::init() {
       gui = static_cast<Gui*>(furi_record_open(RECORD_GUI));
       view_dispatcher = view_dispatcher_alloc();
       view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

       // Initialize views
       main_view.init();
       process_view.init();
       settings_view.init();

       // Add views to dispatcher
       view_dispatcher_add_view(view_dispatcher, ViewMain, main_view.get_view());
       view_dispatcher_add_view(view_dispatcher, ViewProcess, process_view.get_view());
       view_dispatcher_add_view(view_dispatcher, ViewSettings, settings_view.get_view());
   }
   ```

### Phase 4: Timer Management

1. Move timer logic to MainDevelopmentView:

   ```cpp
   class MainDevelopmentView {
   private:
       FuriTimer* timer;

       void init() override {
           timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, this);
           furi_timer_start(timer, 1000);
       }

       static void timer_callback(void* context) {
           auto* view = static_cast<MainDevelopmentView*>(context);
           view->update();
       }
   };
   ```

### Phase 4: Dialog Integration

1. Add dialog handling to MainDevelopmentView:

   ```cpp:applications_user/film_developer/views/main_development_view.hpp
   class MainDevelopmentView : public flipper::ViewCpp {
   private:
       MainViewModel* model;
       ConfirmationDialogView* dialog;

       static void dialog_callback(DialogExResult result, void* context) {
           auto* view = static_cast<MainDevelopmentView*>(context);
           
           switch(result) {
               case DialogExResultRight:
                   // Handle confirmation
                   if (view->pending_action == PendingAction::Start) {
                       view->start_process();
                   } else if (view->pending_action == PendingAction::Abort) {
                       view->abort_process();
                   } else if (view->pending_action == PendingAction::StepComplete) {
                       view->advance_step();
                   }
                   break;
                   
               case DialogExResultLeft:
                   // Handle cancellation - return to previous state
                   break;
           }
       }

       bool handle_input(InputEvent* event) override {
           if (event->type == InputTypeShort) {
               if (event->key == InputKeyOk && !model->process_active) {
                   // Show start confirmation
                   dialog->show_dialog(
                       ConfirmationDialogView::DialogType::ProcessStart,
                       dialog_callback, this);
                   return true;
               } else if (event->key == InputKeyBack && model->process_active) {
                   // Show abort confirmation
                   dialog->show_dialog(
                       ConfirmationDialogView::DialogType::ProcessAbort,
                       dialog_callback, this);
                   return true;
               }
           }
           return false;
       }
   };
   ```

## Custom Events Implementation

### 1. Event Definitions

```cpp
// film_developer_events.hpp
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
    RollCountChanged = 51
};
```

### 2. Application Event Handling

```cpp
class FilmDeveloperApp {
private:
    static bool custom_event_callback(void* context, uint32_t event) {
        auto* app = static_cast<FilmDeveloperApp*>(context);
        auto evt = static_cast<FilmDeveloperEvent>(event);
        
        switch(evt) {
            case FilmDeveloperEvent::ProcessSelected:
                return app->handle_process_selected();
            case FilmDeveloperEvent::SettingsConfirmed:
                return app->handle_settings_confirmed();
            case FilmDeveloperEvent::ProcessAborted:
                return app->handle_process_aborted();
            // ... handle other events
        }
        return false;
    }

    bool handle_process_selected() {
        // Save selected process
        // Switch to settings view
        view_dispatcher_switch_to_view(view_dispatcher, ViewId::Settings);
        return true;
    }

    bool handle_settings_confirmed() {
        // Apply settings
        // Switch to main development view
        view_dispatcher_switch_to_view(view_dispatcher, ViewId::MainDevelopment);
        return true;
    }

    void init() {
        // ... existing initialization ...
        view_dispatcher_set_event_callback_context(view_dispatcher, this);
        view_dispatcher_set_custom_event_callback(view_dispatcher, custom_event_callback);
    }
};
```

### 3. View Event Implementation

#### MainDevelopmentView

```cpp
class MainDevelopmentView : public flipper::ViewCpp {
protected:
    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort) {
            switch(event->key) {
                case InputKeyOk:
                    if(!model->process_active) {
                        send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::StartProcess));
                    } else {
                        send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::PauseProcess));
                    }
                    return true;
                case InputKeyBack:
                    if(model->process_active) {
                        send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessAborted));
                    }
                    return true;
            }
        }
        return false;
    }

    void timer_callback() {
        send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::TimerTick));
    }
};
```

#### ProcessSelectionView

```cpp
class ProcessSelectionView : public flipper::SubMenuCpp {
private:
    static void process_callback(void* context, uint32_t index) {
        auto* view = static_cast<ProcessSelectionView*>(context);
        view->selected_process = view->available_processes[index];
        view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessSelected));
    }
};
```

#### SettingsView

```cpp
class SettingsView : public flipper::VariableItemListCpp {
private:
    static void settings_callback(void* context, uint32_t index) {
        auto* view = static_cast<SettingsView*>(context);
        view->settings->push_pull_stops = index - 2; // Convert 0-4 to -2 to +2
        view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::PushPullChanged));
    }

    static void enter_callback(void* context) {
        auto* view = static_cast<SettingsView*>(context);
        view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::SettingsConfirmed));
    }
};
```

### 4. Process Controller Events

```cpp
class ProcessController {
public:
    void tick() {
        if(current_step_complete()) {
            app->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::StepComplete));
        } else {
            app->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::TimerTick));
        }
    }

    void handle_motor_state_change() {
        app->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::MotorStateChanged));
    }
};
```
