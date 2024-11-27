#include "models/main_view_model.hpp"
#include "views/app/confirmation_dialog_view.hpp"
#include "views/app/main_development_view.hpp"
#include "views/app/process_selection_view.hpp"
#include "views/app/settings_view.hpp"
#ifndef HOST
#include "embedded/motor_controller_embedded.hpp"
#endif
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>

class FilmDeveloperApp {
public:
    enum ViewId {
        ViewProcessSelection,
        // ViewSettings,
        // ViewMainDevelopment,
        // ViewConfirmationDialog,
        ViewCount,
    };

    struct ViewMap {
        ViewId id;
        flipper::ViewCpp* view;
        ViewId next_view;
    };

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

        if(timer) {
            furi_event_loop_timer_free(timer);
        }

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
        // view_map[ViewSettings].view = &settings_view;
        // view_map[ViewMainDevelopment].view = &main_view;
        // view_map[ViewConfirmationDialog].view = &dialog_view;

        // Set up view transitions
        view_map[ViewProcessSelection].next_view = ViewProcessSelection;
        // view_map[ViewProcessSelection].next_view = ViewSettings;
        // view_map[ViewSettings].next_view = ViewMainDevelopment;
        // view_map[ViewMainDevelopment].next_view = ViewProcessSelection;
        // view_map[ViewConfirmationDialog].next_view = ViewProcessSelection;

        // Initialize all views
        for(size_t i = 0; i < ViewCount; i++) {
            view_map[i].view->set_view_dispatcher(view_dispatcher);
            view_map[i].view->init();
            view_dispatcher_add_view(
                view_dispatcher, static_cast<ViewId>(i), view_map[i].view->get_view());
        }

        event_loop = furi_event_loop_alloc();

        // Move this to the app itself
        // Create timer for periodic updates
        timer = furi_event_loop_timer_alloc(
            event_loop, timer_callback, FuriEventLoopTimerTypePeriodic, this);
        furi_event_loop_timer_start(timer, 1000); // 1 second intervals
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

    void update() {
        // auto model = this->model.lock();
        // if (model->process_active && !model->paused) {
        //   bool still_active = process_interpreter->tick();

        //   // Update status texts
        //   const AgitationStepStatic *current_step =
        //       &model->current_process
        //            ->steps[process_interpreter->getCurrentStepIndex()];

        //   model->update_step_text(current_step->name);
        //   model->update_status(process_interpreter->getCurrentMovementTimeElapsed(),
        //                        process_interpreter->getCurrentMovementDuration());
        //   model->update_movement_text(motor_controller->getDirectionString());

        //   model->process_active = still_active;
        //   if (!still_active) {
        //     motor_controller->stop();
        //   }
        // }
    }

private:
    static ViewMap view_map[ViewCount];
    Gui* gui = nullptr;
    ViewDispatcher* view_dispatcher = nullptr;
    ViewId current_view = ViewProcessSelection;
    ViewId previous_view = ViewProcessSelection;

    ProtectedMainViewModel model;
    AgitationProcessInterpreter* process_interpreter{nullptr};
    MotorController* motor_controller{nullptr};

    FuriEventLoopTimer* timer{nullptr};
    FuriEventLoop* event_loop{nullptr};

    // Views
    // MainDevelopmentView main_view{model};
    ProcessSelectionView process_view;
    // SettingsView settings_view{model};
    // ConfirmationDialogView dialog_view;

    static bool custom_callback(void* context, uint32_t event) {
        auto app = static_cast<FilmDeveloperApp*>(context);
        return app->handle_custom_event(static_cast<FilmDeveloperEvent>(event));
    }

    static bool navigation_callback(void* context) {
        auto app = static_cast<FilmDeveloperApp*>(context);
        return app->handle_back_event();
    }

    bool handle_custom_event(FilmDeveloperEvent event) {
        // auto model = this->model.lock();

        switch(event) {
            // case FilmDeveloperEvent::ProcessSelected:
            //   return switch_to_view(ViewSettings);

            // case FilmDeveloperEvent::SettingsConfirmed:
            //   model->set_process(process_view.get_selected_process());
            //   return switch_to_view(ViewMainDevelopment);

        case FilmDeveloperEvent::ProcessAborted:
            return switch_to_view(ViewProcessSelection);

            // case FilmDeveloperEvent::UserActionRequired:
            //   return switch_to_view(ViewConfirmationDialog);

            // case FilmDeveloperEvent::UserActionConfirmed:
            //   return switch_to_view(previous_view);

        default:
            return false;
        }
    }

    bool handle_back_event() {
        view_dispatcher_stop(view_dispatcher);
        return true;
        // if(current_view == ViewProcessSelection) {
        //     // Exit app from process selection
        //     view_dispatcher_stop(view_dispatcher);
        //     return true;
        // } else if(current_view == ViewMainDevelopment) {
        //     // Show confirmation dialog before exiting development
        //     dialog_view.show_dialog(ConfirmationDialogView::DialogType::ProcessAbort);
        //     return switch_to_view(ViewConfirmationDialog);
        // }

        // // Default: go back to process selection
        // return switch_to_view(ViewProcessSelection);
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
};

FilmDeveloperApp::ViewMap FilmDeveloperApp::view_map[ViewCount] = {
    {ViewProcessSelection, nullptr, ViewProcessSelection},
    // {ViewProcessSelection, nullptr, ViewSettings},
    // {ViewSettings, nullptr, ViewMainDevelopment},
    // {ViewMainDevelopment, nullptr, ViewProcessSelection},
    // {ViewConfirmationDialog, nullptr, ViewProcessSelection},
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
