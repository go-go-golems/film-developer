#include "gui/view_dispatcher_i.h"
#include "models/main_view_model.hpp"
#include "views/app/confirmation_dialog_view.hpp"
#include "views/app/main_development_view.hpp"
#include "views/app/process_selection_view.hpp"
#include "views/app/settings_view.hpp"
#include "views/app/dispatch_menu_view.hpp"
#include "views/app/runtime_settings_view.hpp"
#include "views/app/paused_view.hpp"
#include "views/app/waiting_confirmation_view.hpp"
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
        ViewPaused,
        ViewWaitingConfirmation,
        ViewCount,
    };

    enum class AppState {
        ProcessSelection,
        Settings,
        MainView,
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
        case FilmDeveloperEvent::SkipStep:
            return "SkipStep";
        case FilmDeveloperEvent::RestartStep:
            return "RestartStep";
        case FilmDeveloperEvent::DialogDismissed:
            return "DialogDismissed";
        case FilmDeveloperEvent::DialogConfirmed:
            return "DialogConfirmed";
        case FilmDeveloperEvent::StateChanged:
            return "StateChanged";
        case FilmDeveloperEvent::DispatchRequested:
            return "DispatchRequested";
        case FilmDeveloperEvent::PauseRequested:
            return "PauseRequested";
        case FilmDeveloperEvent::ResumeRequested:
            return "ResumeRequested";
        default:
            return "Unknown";
        }
    }

    static const char* get_state_name(AppState state) {
        switch(state) {
        case AppState::ProcessSelection:
            return "ProcessSelection";
        case AppState::Settings:
            return "Settings";
        case AppState::MainView:
            return "Running";
        case AppState::Paused:
            return "Paused";
        case AppState::WaitingConfirmation:
            return "WaitingConfirmation";
        case AppState::RuntimeSettings:
            return "RuntimeSettings";
        case AppState::DispatchDialog:
            return "DispatchDialog";
        case AppState::ConfirmRestart:
            return "ConfirmRestart";
        case AppState::ConfirmSkip:
            return "ConfirmSkip";
        case AppState::ConfirmExit:
            return "ConfirmExit";
        default:
            return "Unknown";
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
        view_map[ViewPaused].view = &paused_view;
        view_map[ViewWaitingConfirmation].view = &waiting_confirmation_view;

        // Set up view transitions
        view_map[ViewProcessSelection].next_view = ViewSettings;
        view_map[ViewSettings].next_view = ViewMainDevelopment;
        view_map[ViewMainDevelopment].next_view = ViewProcessSelection;
        view_map[ViewConfirmationDialog].next_view = ViewProcessSelection;
        view_map[ViewDispatchMenu].next_view = ViewMainDevelopment;
        view_map[ViewRuntimeSettings].next_view = ViewMainDevelopment;
        view_map[ViewPaused].next_view = ViewMainDevelopment;
        view_map[ViewWaitingConfirmation].next_view = ViewMainDevelopment;

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
        if(model->is_process_active() && !model->is_process_paused()) {
            auto& process_interpreter = model->process_interpreter;
            bool still_active = process_interpreter.tick();

            FURI_LOG_T(
                "FilmDev",
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

            if(!still_active) {
                model->complete_process();
                FURI_LOG_I("FilmDev", "Process completed");
            }
        }

        view_dispatcher_send_custom_event(
            view_dispatcher, static_cast<uint32_t>(FilmDeveloperEvent::TimerTick));
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
    PausedView paused_view{model};
    WaitingConfirmationView waiting_confirmation_view{model};

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

    bool handle_back_event() {
        auto model = this->model.lock();
        switch(current_state) {
        case AppState::ProcessSelection:
            view_dispatcher_stop(view_dispatcher);
            return true;

        case AppState::MainView:
        case AppState::Paused:
            FURI_LOG_D(
                "FilmDev",
                "State transition: %s -> %s",
                get_state_name(current_state),
                get_state_name(AppState::ConfirmExit));

            previous_state = current_state;
            current_state = AppState::ConfirmExit;

            dialog_view.show_dialog(ConfirmationDialogView::DialogType::ProcessAbort);
            return switch_to_view(ViewConfirmationDialog);

        case AppState::RuntimeSettings:
        case AppState::DispatchDialog:
            FURI_LOG_D(
                "FilmDev",
                "State transition: %s -> %s",
                get_state_name(current_state),
                get_state_name(previous_state));

            current_state = previous_state;

            if(model->is_process_paused()) {
                return switch_to_view(ViewPaused);
            }
            return switch_to_view(ViewMainDevelopment);

        case AppState::ConfirmExit:
        case AppState::ConfirmRestart:
        case AppState::ConfirmSkip:
            FURI_LOG_D(
                "FilmDev",
                "State transition: %s -> %s",
                get_state_name(current_state),
                get_state_name(previous_state));

            current_state = previous_state;
            return switch_to_view(ViewMainDevelopment);

        default:
            FURI_LOG_D(
                "FilmDev",
                "State transition: %s -> %s",
                get_state_name(current_state),
                get_state_name(AppState::ProcessSelection));

            previous_state = current_state;
            current_state = AppState::ProcessSelection;

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

    bool handle_custom_event(FilmDeveloperEvent event) {
        auto model = this->model.lock();
        FURI_LOG_D(
            "FilmDev",
            "Custom event received: %s, current state: %s",
            get_event_name(event),
            get_state_name(current_state));

        previous_state = current_state;

        switch(event) {
        case FilmDeveloperEvent::ProcessSelected:
            current_state = AppState::Settings;
            return switch_to_view(ViewSettings);

        case FilmDeveloperEvent::SettingsConfirmed:
            model->set_process(process_view.get_selected_process());
            current_state = AppState::MainView;
            return switch_to_view(ViewMainDevelopment);

        case FilmDeveloperEvent::ProcessAborted:
            current_state = AppState::ProcessSelection;
            return switch_to_view(ViewProcessSelection);

        case FilmDeveloperEvent::PauseRequested:
            if(current_state == AppState::MainView || current_state == AppState::DispatchDialog) {
                if(model->pause_process()) {
                    current_state = AppState::Paused;
                    if(current_state == AppState::DispatchDialog) {
                        return switch_to_view(ViewMainDevelopment);
                    }
                    return switch_to_view(ViewPaused);
                }
            }
            return true;

        case FilmDeveloperEvent::ResumeRequested:
            if(current_state == AppState::Paused || current_state == AppState::DispatchDialog) {
                if(model->resume_process()) {
                    current_state = AppState::MainView;
                    if(current_state == AppState::DispatchDialog) {
                        return switch_to_view(ViewMainDevelopment);
                    }
                    return switch_to_view(ViewMainDevelopment);
                }
            }
            return true;

        case FilmDeveloperEvent::EnterRuntimeSettings:
            current_state = AppState::RuntimeSettings;
            return switch_to_view(ViewRuntimeSettings);

        case FilmDeveloperEvent::ExitRuntimeSettings:
            if(model->is_process_paused()) {
                current_state = AppState::Paused;
                return switch_to_view(ViewPaused);
            }
            current_state = AppState::MainView;
            return switch_to_view(ViewMainDevelopment);

        case FilmDeveloperEvent::DispatchRequested:
            current_state = AppState::DispatchDialog;
            return switch_to_view(ViewDispatchMenu);

        case FilmDeveloperEvent::DialogDismissed:
            if(model->is_process_paused()) {
                current_state = AppState::Paused;
                return switch_to_view(ViewPaused);
            }
            current_state = AppState::MainView;
            return switch_to_view(ViewMainDevelopment);

        case FilmDeveloperEvent::UserActionRequired:
            if(model->wait_for_user()) {
                current_state = AppState::WaitingConfirmation;
                return switch_to_view(ViewWaitingConfirmation);
            }
            return true;

        case FilmDeveloperEvent::UserActionConfirmed:
            if(model->confirm_user_action()) {
                current_state = AppState::MainView;
                return switch_to_view(ViewMainDevelopment);
            }
            return true;

        case FilmDeveloperEvent::RestartStep:
            if(current_state == AppState::MainView || current_state == AppState::Paused ||
               current_state == AppState::DispatchDialog) {
                current_state = AppState::ConfirmRestart;
                dialog_view.show_dialog(ConfirmationDialogView::DialogType::StepRestart);
                return switch_to_view(ViewConfirmationDialog);
            }
            return true;

        case FilmDeveloperEvent::SkipStep:
            if(current_state == AppState::MainView || current_state == AppState::Paused ||
               current_state == AppState::DispatchDialog) {
                current_state = AppState::ConfirmSkip;
                dialog_view.show_dialog(ConfirmationDialogView::DialogType::StepSkip);
                return switch_to_view(ViewConfirmationDialog);
            }
            return true;

        case FilmDeveloperEvent::DialogConfirmed:
            switch(current_state) {
            case AppState::ConfirmRestart: {
                model->process_interpreter.reset();
                if(model->is_process_paused()) {
                    model->resume_process();
                }
                current_state = AppState::MainView;
                return switch_to_view(ViewMainDevelopment);
            }
            case AppState::ConfirmSkip: {
                model->process_interpreter.skipToNextStep();
                if(model->is_process_paused()) {
                    model->resume_process();
                }
                current_state = AppState::MainView;
                return switch_to_view(ViewMainDevelopment);
            }
            default:
                return false;
            }

        case FilmDeveloperEvent::StartProcess:
            if(model->start_process()) {
                current_state = AppState::MainView;
                return switch_to_view(ViewMainDevelopment);
            }
            return true;

        case FilmDeveloperEvent::ProcessCompleted:
            if(model->complete_process()) {
                current_state = AppState::ProcessSelection;
                return switch_to_view(ViewProcessSelection);
            }
            return true;

        default:
            return false;
        }
    }
};

FilmDeveloperApp::ViewMap FilmDeveloperApp::view_map[ViewCount] = {
    {ViewProcessSelection, nullptr, ViewSettings},
    {ViewSettings, nullptr, ViewMainDevelopment},
    {ViewMainDevelopment, nullptr, ViewProcessSelection},
    {ViewConfirmationDialog, nullptr, ViewProcessSelection},
    {ViewDispatchMenu, nullptr, ViewMainDevelopment},
    {ViewRuntimeSettings, nullptr, ViewMainDevelopment},
    {ViewPaused, nullptr, ViewMainDevelopment},
    {ViewWaitingConfirmation, nullptr, ViewMainDevelopment},
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
