#include "gui/view_dispatcher_i.h"
#include "models/main_view_model.hpp"
#include "views/app/confirmation_dialog_view.hpp"
#include "views/app/main_development_view.hpp"
#include "views/app/process_selection_view.hpp"
#include "views/app/settings_view.hpp"
#include "views/app/dispatch_menu_view.hpp"
#include "views/app/runtime_settings_view.hpp"
#ifndef HOST
#include "embedded/motor_controller_embedded.hpp"
#endif

extern "C" {
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
}

class FilmDeveloperApp {
public:
    enum ViewId {
        ViewProcessSelection,
        ViewSettings,
        ViewMainDevelopment,
        ViewConfirmationDialog,
        ViewDispatchMenu,
        ViewRuntimeSettings,
        ViewCount,
    };

    enum class AppState {
        ProcessSelection,
        Settings,
        Running,
        Paused,
        WaitingConfirmation,
        RuntimeSettings,
        DispatchDialog,
        ConfirmRestart,
        ConfirmSkip,
        ConfirmExit
    };

    struct ViewMap {
        ViewId id;
        flipper::ViewCpp* view;
        ViewId next_view;
    };

    static const char* get_event_name(FilmDeveloperEvent event) {
        switch(event) {
            case FilmDeveloperEvent::ProcessSelected: return "ProcessSelected";
            case FilmDeveloperEvent::SettingsConfirmed: return "SettingsConfirmed";
            case FilmDeveloperEvent::ProcessAborted: return "ProcessAborted";
            case FilmDeveloperEvent::ProcessCompleted: return "ProcessCompleted";
            case FilmDeveloperEvent::StartProcess: return "StartProcess";
            case FilmDeveloperEvent::PauseProcess: return "PauseProcess";
            case FilmDeveloperEvent::ResumeProcess: return "ResumeProcess";
            case FilmDeveloperEvent::UserActionRequired: return "UserActionRequired";
            case FilmDeveloperEvent::UserActionConfirmed: return "UserActionConfirmed";
            case FilmDeveloperEvent::TimerTick: return "TimerTick";
            case FilmDeveloperEvent::StepComplete: return "StepComplete";
            case FilmDeveloperEvent::MotorStateChanged: return "MotorStateChanged";
            case FilmDeveloperEvent::AgitationComplete: return "AgitationComplete";
            case FilmDeveloperEvent::PushPullChanged: return "PushPullChanged";
            case FilmDeveloperEvent::RollCountChanged: return "RollCountChanged";
            case FilmDeveloperEvent::EnterRuntimeSettings: return "EnterRuntimeSettings";
            case FilmDeveloperEvent::ExitRuntimeSettings: return "ExitRuntimeSettings";
            case FilmDeveloperEvent::StepDurationChanged: return "StepDurationChanged";
            case FilmDeveloperEvent::SkipStep: return "SkipStep";
            case FilmDeveloperEvent::RestartStep: return "RestartStep";
            case FilmDeveloperEvent::DialogDismissed: return "DialogDismissed";
            case FilmDeveloperEvent::DialogConfirmed: return "DialogConfirmed";
            case FilmDeveloperEvent::StateChanged: return "StateChanged";
            case FilmDeveloperEvent::DispatchRequested: return "DispatchRequested";
            case FilmDeveloperEvent::PauseRequested: return "PauseRequested";
            case FilmDeveloperEvent::ResumeRequested: return "ResumeRequested";
            default: return "Unknown";
        }
    }

    static const char* get_state_name(AppState state) {
        switch(state) {
            case AppState::ProcessSelection: return "ProcessSelection";
            case AppState::Settings: return "Settings";
            case AppState::Running: return "Running";
            case AppState::Paused: return "Paused";
            case AppState::WaitingConfirmation: return "WaitingConfirmation";
            case AppState::RuntimeSettings: return "RuntimeSettings";
            case AppState::DispatchDialog: return "DispatchDialog";
            case AppState::ConfirmRestart: return "ConfirmRestart";
            case AppState::ConfirmSkip: return "ConfirmSkip";
            case AppState::ConfirmExit: return "ConfirmExit";
            default: return "Unknown";
        }
    }

    FilmDeveloperApp()
        : motor_controller(
#ifdef HOST
              new MockController()
#else
              new MotorControllerEmbedded()
#endif
          ) {
        gui = static_cast<Gui*>(furi_record_open(RECORD_GUI));
        view_dispatcher = view_dispatcher_alloc();
        view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
        view_dispatcher_set_event_callback_context(view_dispatcher, this);
        view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);
        view_dispatcher_set_navigation_event_callback(view_dispatcher, navigation_callback);
        view_dispatcher_set_tick_event_callback(view_dispatcher, timer_callback, 1000);

#ifndef HOST
        static_cast<MotorControllerEmbedded*>(motor_controller)->initGpio();
#endif

        auto model = this->model.lock();
        model->motor_controller = motor_controller;
        model->init();
    }

    ~FilmDeveloperApp() {
#ifndef HOST
        static_cast<MotorControllerEmbedded*>(motor_controller)->deinitGpio();
#endif
        delete motor_controller;

        if(view_dispatcher != nullptr) {
            for(size_t i = 0; i < ViewCount; i++) {
                view_dispatcher_remove_view(view_dispatcher, static_cast<ViewId>(i));
            }
            view_dispatcher_free(view_dispatcher);
            furi_record_close(RECORD_GUI);
        }
    }

    void init() {
        // Initialize view map with actual view pointers
        view_map[ViewProcessSelection].view = &process_view;
        view_map[ViewSettings].view = &settings_view;
        view_map[ViewMainDevelopment].view = &main_view;
        view_map[ViewConfirmationDialog].view = &dialog_view;
        view_map[ViewDispatchMenu].view = &dispatch_menu_view;
        view_map[ViewRuntimeSettings].view = &runtime_settings_view;

        // Set up view transitions
        view_map[ViewProcessSelection].next_view = ViewSettings;
        view_map[ViewSettings].next_view = ViewMainDevelopment;
        view_map[ViewMainDevelopment].next_view = ViewProcessSelection;
        view_map[ViewConfirmationDialog].next_view = ViewProcessSelection;
        view_map[ViewDispatchMenu].next_view = ViewMainDevelopment;
        view_map[ViewRuntimeSettings].next_view = ViewMainDevelopment;

        // Initialize all views
        for(size_t i = 0; i < ViewCount; i++) {
            view_map[i].view->set_view_dispatcher(view_dispatcher);
            view_map[i].view->init();
            view_dispatcher_add_view(
                view_dispatcher, static_cast<ViewId>(i), view_map[i].view->get_view());
        }
    }

    void run() {
        // Start with process selection
        view_dispatcher_switch_to_view(view_dispatcher, ViewProcessSelection);
        view_dispatcher_run(view_dispatcher);
    }

    static void timer_callback(void* context) {
        auto app = static_cast<FilmDeveloperApp*>(context);
        app->update();
    }

    int ticks = 0;

    void update() {
        auto model = this->model.lock();
        if(model->process_active && !model->paused) {
            auto& process_interpreter = model->process_interpreter;
            bool still_active = process_interpreter.tick();

            FURI_LOG_D("FilmDev", 
                "Process tick - Step: %d, Time: %ld/%ld", 
                process_interpreter.getCurrentStepIndex(),
                process_interpreter.getCurrentMovementTimeElapsed(),
                process_interpreter.getCurrentMovementDuration());

            const AgitationStepStatic* current_step =
                &model->current_process->steps[process_interpreter.getCurrentStepIndex()];

            model->update_step_text(current_step->name);
            model->update_status(
                process_interpreter.getCurrentMovementTimeElapsed(),
                process_interpreter.getCurrentMovementDuration());
            model->update_movement_text(motor_controller->getDirectionString());

            if(model->process_active != still_active) {
                FURI_LOG_I("FilmDev", "Process completed");
            }
            model->process_active = still_active;
            if(!still_active) {
                motor_controller->stop();
            }
        } else {
            // ticks = 0;
        }

        view_dispatcher_send_custom_event(
            view_dispatcher, static_cast<uint32_t>(FilmDeveloperEvent::TimerTick));
        // // ticks++;
        // // char status_text[32];
        // // snprintf(status_text, sizeof(status_text), "ticks: %d", ticks);
        // // model->update_step_text(status_text);

        // // kind of a hack to update the main view...
        // main_view.get_view()->update_callback(main_view.get_view(), view_dispatcher);
    }

private:
    static ViewMap view_map[ViewCount];
    Gui* gui = nullptr;
    ViewDispatcher* view_dispatcher = nullptr;
    ViewId current_view = ViewProcessSelection;
    ViewId previous_view = ViewProcessSelection;

    ProtectedModel model;
    MotorController* motor_controller{nullptr};

    // Views
    MainDevelopmentView main_view{model};
    ProcessSelectionView process_view;
    SettingsView settings_view{model};
    ConfirmationDialogView dialog_view;
    DispatchMenuView dispatch_menu_view;
    RuntimeSettingsView runtime_settings_view;

    AppState current_state{AppState::ProcessSelection};
    AppState previous_state{AppState::ProcessSelection};

    static bool custom_callback(void* context, uint32_t event) {
        auto app = static_cast<FilmDeveloperApp*>(context);
        return app->handle_custom_event(static_cast<FilmDeveloperEvent>(event));
    }

    static bool navigation_callback(void* context) {
        auto app = static_cast<FilmDeveloperApp*>(context);
        return app->handle_back_event();
    }

    bool handle_custom_event(FilmDeveloperEvent event) {
        auto model = this->model.lock();
        FURI_LOG_D("FilmDev", "Custom event received: %s", get_event_name(event));

        switch(event) {
            case FilmDeveloperEvent::ProcessSelected:
                transition_to(AppState::Settings);
                return switch_to_view(ViewSettings);

            case FilmDeveloperEvent::SettingsConfirmed:
                model->set_process(process_view.get_selected_process());
                transition_to(AppState::Running);
                return switch_to_view(ViewMainDevelopment);

            case FilmDeveloperEvent::ProcessAborted:
                transition_to(AppState::ProcessSelection);
                return switch_to_view(ViewProcessSelection);

            case FilmDeveloperEvent::PauseRequested:
                if(current_state == AppState::Running) {
                    transition_to(AppState::Paused);
                }
                return true;

            case FilmDeveloperEvent::ResumeRequested:
                if(current_state == AppState::Paused) {
                    transition_to(AppState::Running);
                }
                return true;

            case FilmDeveloperEvent::EnterRuntimeSettings:
                transition_to(AppState::RuntimeSettings);
                return true;

            case FilmDeveloperEvent::ExitRuntimeSettings:
                transition_to(previous_state);
                return switch_to_view(ViewMainDevelopment);

            case FilmDeveloperEvent::DispatchRequested:
                transition_to(AppState::DispatchDialog);
                return true;

            case FilmDeveloperEvent::DialogDismissed:
                transition_to(previous_state);
                return switch_to_view(ViewMainDevelopment);

            case FilmDeveloperEvent::UserActionRequired:
                transition_to(AppState::WaitingConfirmation);
                return switch_to_view(ViewConfirmationDialog);

            case FilmDeveloperEvent::UserActionConfirmed:
                transition_to(AppState::Running);
                return switch_to_view(ViewMainDevelopment);

            case FilmDeveloperEvent::RestartStep:
                if(current_state == AppState::Running || current_state == AppState::Paused) {
                    transition_to(AppState::ConfirmRestart);
                    dialog_view.show_dialog(ConfirmationDialogView::DialogType::StepRestart);
                    return switch_to_view(ViewConfirmationDialog);
                }
                return true;

            case FilmDeveloperEvent::SkipStep:
                if(current_state == AppState::Running || current_state == AppState::Paused) {
                    transition_to(AppState::ConfirmSkip);
                    dialog_view.show_dialog(ConfirmationDialogView::DialogType::StepSkip);
                    return switch_to_view(ViewConfirmationDialog);
                }
                return true;

            case FilmDeveloperEvent::DialogConfirmed:
                switch(current_state) {
                    case AppState::ConfirmRestart:
                        {
                            auto model = this->model.lock();
                            model->process_interpreter.reset();
                            transition_to(AppState::Running);
                            return switch_to_view(ViewMainDevelopment);
                        }
                    case AppState::ConfirmSkip:
                        {
                            auto model = this->model.lock();
                            model->process_interpreter.skipToNextStep();
                            transition_to(AppState::Running);
                            return switch_to_view(ViewMainDevelopment);
                        }
                    default:
                        return false;
                }

            case FilmDeveloperEvent::StartProcess:
                {
                    auto model = this->model.lock();
                    model->process_active = true;
                    model->paused = false;
                    transition_to(AppState::Running);
                    return switch_to_view(ViewMainDevelopment);
                }

            case FilmDeveloperEvent::ProcessCompleted:
                {
                    auto model = this->model.lock();
                    model->process_active = false;
                    model->paused = false;
                    motor_controller->stop();
                    transition_to(AppState::ProcessSelection);
                    return switch_to_view(ViewProcessSelection);
                }

            default:
                return false;
        }
    }

    bool handle_back_event() {
        switch(current_state) {
            case AppState::ProcessSelection:
                view_dispatcher_stop(view_dispatcher);
                return true;
                
            case AppState::Running:
                transition_to(AppState::ConfirmExit);
                dialog_view.show_dialog(ConfirmationDialogView::DialogType::ProcessAbort);
                return switch_to_view(ViewConfirmationDialog);
                
            case AppState::RuntimeSettings:
                transition_to(previous_state);
                return switch_to_view(ViewMainDevelopment);
                
            case AppState::DispatchDialog:
                transition_to(previous_state);
                return switch_to_view(ViewMainDevelopment);
                
            default:
                transition_to(AppState::ProcessSelection);
                return switch_to_view(ViewProcessSelection);
        }
    }

    bool switch_to_view(ViewId new_view_id) {
        flipper::ViewCpp* current = get_view(current_view);
        flipper::ViewCpp* next = get_view(new_view_id);

        if(current) {
            current->exit();
        }

        if(next) {
            next->enter();
        }

        previous_view = current_view;
        current_view = new_view_id;
        view_dispatcher_switch_to_view(view_dispatcher, new_view_id);
        return true;
    }

    flipper::ViewCpp* get_view(ViewId id) {
        return view_map[id].view;
    }

    ViewId get_next_view(ViewId current) {
        return view_map[current].next_view;
    }

    void transition_to(AppState new_state) {
        FURI_LOG_D("FilmDev", "State transition: %s -> %s", 
                   get_state_name(current_state),
                   get_state_name(new_state));
                   
        previous_state = current_state;
        current_state = new_state;
        
        // Handle state entry actions
        switch(new_state) {
            case AppState::Running:
                if(previous_state == AppState::Paused) {
                    auto model = this->model.lock();
                    model->paused = false;
                }
                break;
                
            case AppState::Paused:
                {
                    auto model = this->model.lock();
                    model->paused = true;
                    motor_controller->stop();
                }
                break;
                
            case AppState::RuntimeSettings:
                view_dispatcher_switch_to_view(view_dispatcher, ViewRuntimeSettings);
                break;
                
            case AppState::DispatchDialog:
                view_dispatcher_switch_to_view(view_dispatcher, ViewDispatchMenu);
                break;
                
            default:
                break;
        }
        
        view_dispatcher_send_custom_event(
            view_dispatcher, 
            static_cast<uint32_t>(FilmDeveloperEvent::StateChanged));
    }
};

FilmDeveloperApp::ViewMap FilmDeveloperApp::view_map[ViewCount] = {
    {ViewProcessSelection, nullptr, ViewSettings},
    {ViewSettings, nullptr, ViewMainDevelopment},
    {ViewMainDevelopment, nullptr, ViewProcessSelection},
    {ViewConfirmationDialog, nullptr, ViewProcessSelection},
    {ViewDispatchMenu, nullptr, ViewMainDevelopment},
    {ViewRuntimeSettings, nullptr, ViewMainDevelopment},
};

#ifdef __cplusplus
extern "C" {
#endif

int32_t film_developer_app(void* p) {
    UNUSED(p);
    FilmDeveloperApp app;
    app.init();
    app.run();
    return 0;
}

#ifdef __cplusplus
}
#endif
