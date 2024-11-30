#include "models/main_view_model.hpp"
#include "views/app/confirmation_dialog_view.hpp"
#include "views/app/dispatch_menu_view.hpp"
#include "views/app/main_development_view.hpp"
#include "views/app/paused_view.hpp"
#include "views/app/process_selection_view.hpp"
#include "views/app/runtime_settings_view.hpp"
#include "views/app/settings_view.hpp"
#ifndef HOST
#include "embedded/motor_controller_embedded.hpp"
#endif

#include "agitation/cinestill_process_interpreter.hpp"

extern "C" {
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
}

#define APP_TAG "FilmDev"

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
    ConfirmStop,
    ConfirmExit
  };

  struct ViewMap {
    ViewId id;
    flipper::ViewCpp *view;
  };

  static const char *get_state_name(AppState state) {
    switch (state) {
    case AppState::ProcessSelection:
      return "ProcessSelection";
    case AppState::Settings:
      return "Settings";
    case AppState::MainView:
      return "MainView";
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
    case AppState::ConfirmStop:
      return "ConfirmStop";
    }
    return "Unknown";
  }

  FilmDeveloperApp()
      : motor_controller(
#ifdef HOST
            new MockController()
#else
            new MotorControllerEmbedded()
#endif
                )
        // , process_interpreter(new AgitationProcessInterpreter())
        ,
        process_interpreter(new CineStillProcessInterpreter(motor_controller)),
        process_view(process_interpreter) {
    gui = static_cast<Gui *>(furi_record_open(RECORD_GUI));
    view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(view_dispatcher, gui,
                                  ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(view_dispatcher, this);
    view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);
    view_dispatcher_set_navigation_event_callback(view_dispatcher,
                                                  navigation_callback);
    view_dispatcher_set_tick_event_callback(view_dispatcher, timer_callback,
                                            1000);

#ifndef HOST
    static_cast<MotorControllerEmbedded *>(motor_controller)->initGpio();
#endif

    auto model = this->model.lock();
    model->motor_controller = motor_controller;

    process_interpreter->init();
    model->process_interpreter = process_interpreter;
    model->init();
  }

  ~FilmDeveloperApp() {
    if (view_dispatcher != nullptr) {
      FURI_LOG_D(APP_TAG, "Freeing views");
      for (size_t i = 0; i < ViewCount; i++) {
        view_dispatcher_remove_view(view_dispatcher, static_cast<ViewId>(i));
        FURI_LOG_D(APP_TAG, "Removed view %d", i);
      }
      view_dispatcher_free(view_dispatcher);
      FURI_LOG_D(APP_TAG, "View dispatcher freed");
      furi_record_close(RECORD_GUI);
      FURI_LOG_D(APP_TAG, "GUI freed");

      delete process_interpreter;
      FURI_LOG_D(APP_TAG, "Process interpreter freed");
#ifndef HOST
      static_cast<MotorControllerEmbedded *>(motor_controller)->deinitGpio();
#endif
      delete motor_controller;
      FURI_LOG_D(APP_TAG, "Motor controller freed");
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

    // Initialize all views
    for (size_t i = 0; i < ViewCount; i++) {
      view_map[i].view->set_view_dispatcher(view_dispatcher);
      view_map[i].view->init();
      view_dispatcher_add_view(view_dispatcher, static_cast<ViewId>(i),
                               view_map[i].view->get_view());
    }
  }

  void send_custom_event(FilmDeveloperEvent event) {
    if (event == FilmDeveloperEvent::TimerTick) {
      FURI_LOG_T(APP_TAG, "Sending timer tick event");
    } else {
      FURI_LOG_D(APP_TAG, "Sending custom event: %s", get_event_name(event));
    }
    view_dispatcher_send_custom_event(view_dispatcher,
                                      static_cast<uint32_t>(event));
  }

  void run() {
    // Start with process selection
    view_dispatcher_switch_to_view(view_dispatcher, ViewProcessSelection);
    view_dispatcher_run(view_dispatcher);
  }

  static void timer_callback(void *context) {
    auto app = static_cast<FilmDeveloperApp *>(context);
    app->update();
  }

  int ticks = 0;

  void update() {
    auto model = this->model.lock();
    if (model->is_process_active() && !model->is_process_paused()) {
      auto *process_interpreter = model->process_interpreter;
      const bool wasWaitingForUser = process_interpreter->isWaitingForUser();
      bool still_active = process_interpreter->tick();
      const bool isWaitingForUser = process_interpreter->isWaitingForUser();
      if (!wasWaitingForUser && isWaitingForUser) {
        send_custom_event(FilmDeveloperEvent::UserActionRequired);
      }

      FURI_LOG_T(
          APP_TAG, "Process tick - Step: %d, Time: %ld/%ld",
          process_interpreter->getCurrentStepIndex(),
          static_cast<long>(
              process_interpreter->getCurrentMovementTimeElapsed()),
          static_cast<long>(process_interpreter->getCurrentMovementDuration()));

      model->update_step_text(process_interpreter->getCurrentStepName());
      model->update_status(process_interpreter->getCurrentMovementTimeElapsed(),
                           process_interpreter->getCurrentMovementDuration());
      model->update_movement_text(motor_controller->getDirectionString());

      if (!still_active && process_interpreter->isComplete()) {
        model->complete_process();
        FURI_LOG_I(APP_TAG, "Process completed");
      }
    }

    send_custom_event(FilmDeveloperEvent::TimerTick);
  }

private:
  static ViewMap view_map[ViewCount];
  Gui *gui = nullptr;
  ViewDispatcher *view_dispatcher = nullptr;
  ViewId current_view = ViewProcessSelection;

  ProtectedModel model;
  MotorController *motor_controller{nullptr};
  ProcessInterpreterInterface *process_interpreter{nullptr};

  // Views
  MainDevelopmentView main_view{model};
  ProcessSelectionView process_view{process_interpreter};
  SettingsView settings_view{model};
  ConfirmationDialogView dialog_view;
  DispatchMenuView dispatch_menu_view;
  RuntimeSettingsView runtime_settings_view;
  PausedView paused_view{model};

  AppState current_state{AppState::ProcessSelection};
  AppState before_confirmation_state{AppState::ProcessSelection};
  ViewId before_confirmation_view{ViewProcessSelection};

  static bool custom_callback(void *context, uint32_t event) {
    auto app = static_cast<FilmDeveloperApp *>(context);
    return app->handle_custom_event(static_cast<FilmDeveloperEvent>(event));
  }

  static bool navigation_callback(void *context) {
    FURI_LOG_D(APP_TAG, "Navigation callback");
    auto app = static_cast<FilmDeveloperApp *>(context);
    return app->handle_back_event();
  }

  void enter_state(AppState new_state) {
    FURI_LOG_D(APP_TAG, "State transition: %s -> %s",
               get_state_name(current_state), get_state_name(new_state));

    current_state = new_state;
  }

  bool switch_to_view(ViewId new_view_id) {
    flipper::ViewCpp *current = get_view(current_view);
    flipper::ViewCpp *next = get_view(new_view_id);

    if (current == next) {
      FURI_LOG_W(APP_TAG, "Switching to same view, ignoring");
      return false;
    }

    // if (current) {
    //   current->exit();
    // }

    // if (next) {
    //   next->enter();
    // }

    current_view = new_view_id;
    view_dispatcher_switch_to_view(view_dispatcher, new_view_id);
    return true;
  }

  void
  show_confirmation_dialog(ConfirmationDialogView::DialogType dialog_type) {
    before_confirmation_state = current_state;
    before_confirmation_view = current_view;
    dialog_view.show_dialog(dialog_type);
    switch_to_view(ViewConfirmationDialog);
  }

  void show_exit_confirmation_dialog() {
    show_confirmation_dialog(ConfirmationDialogView::DialogType::AppExit);
    enter_state(AppState::ConfirmExit);
  }

  void show_restart_confirmation_dialog() {
    show_confirmation_dialog(ConfirmationDialogView::DialogType::StepRestart);
    enter_state(AppState::ConfirmRestart);
  }

  void show_waiting_confirmation_dialog() {
    show_confirmation_dialog(ConfirmationDialogView::DialogType::StepComplete);
    enter_state(AppState::WaitingConfirmation);
  }

  void show_skip_confirmation_dialog() {
    show_confirmation_dialog(ConfirmationDialogView::DialogType::StepSkip);
    enter_state(AppState::ConfirmSkip);
  }

  void show_stop_confirmation_dialog() {
    show_confirmation_dialog(ConfirmationDialogView::DialogType::ProcessAbort);
    enter_state(AppState::ConfirmStop);
  }

  void show_main_view(bool paused = false) {
    if (paused) {
      enter_state(AppState::Paused);
      switch_to_view(ViewPaused);
    }
    enter_state(AppState::MainView);
    switch_to_view(ViewMainDevelopment);
  }

  bool handle_back_event() {
    FURI_LOG_D(APP_TAG, "Back event");
    auto model = this->model.lock();
    FURI_LOG_D(APP_TAG, "Model locked, Process active: %d",
               model->is_process_active());
    switch (current_state) {
    case AppState::ProcessSelection:
      show_exit_confirmation_dialog();
      //   // Only allow exit to menu if no process is active
      //   if (!model->is_process_active()) {
      //     FURI_LOG_D(APP_TAG, "No process active, stopping view
      //     dispatcher"); view_dispatcher_stop(view_dispatcher); return true;
      //   }
      //   FURI_LOG_D(APP_TAG, "Process active, staying in state");
      //   return false;
      return true;

    case AppState::Paused:
      // Resume process when back is pressed from paused view
      // XXX should show stop confirmation dialog
      if (model->resume_process()) {
        enter_state(AppState::MainView);
        return switch_to_view(ViewMainDevelopment);
      }
      return true;

    case AppState::MainView:
      if (model->is_process_active()) {
        show_stop_confirmation_dialog();
      } else {
        enter_state(AppState::ProcessSelection);
        return switch_to_view(ViewProcessSelection);
      }
      return true;

    case AppState::RuntimeSettings:
    case AppState::DispatchDialog:
      if (model->is_process_paused()) {
        enter_state(AppState::Paused);
        return switch_to_view(ViewPaused);
      }
      enter_state(AppState::MainView);
      return switch_to_view(ViewMainDevelopment);

    case AppState::ConfirmExit:
    case AppState::ConfirmRestart:
    case AppState::ConfirmSkip:
    case AppState::ConfirmStop:
      if (model->is_process_paused()) {
        enter_state(AppState::Paused);
        return switch_to_view(ViewPaused);
      }
      enter_state(before_confirmation_state);
      return switch_to_view(before_confirmation_view);

    case AppState::Settings:
      enter_state(AppState::ProcessSelection);
      return switch_to_view(ViewProcessSelection);

    case AppState::WaitingConfirmation:
      enter_state(before_confirmation_state);
      return switch_to_view(before_confirmation_view);
    }

    furi_assert(false);
    return false;
  }

  flipper::ViewCpp *get_view(ViewId id) { return view_map[id].view; }

  bool handle_custom_event(FilmDeveloperEvent event) {
    auto model = this->model.lock();
    if (event == FilmDeveloperEvent::TimerTick) {
      FURI_LOG_T(APP_TAG, "Timer tick event, current state: %s",
                 get_state_name(current_state));
    } else {
      FURI_LOG_D(APP_TAG, "Custom event received: %s, current state: %s",
                 get_event_name(event), get_state_name(current_state));
    }

    switch (event) {
    case FilmDeveloperEvent::ProcessSelected:
      enter_state(AppState::Settings);
      return switch_to_view(ViewSettings);

    case FilmDeveloperEvent::SettingsConfirmed:
      model->set_process(process_view.get_selected_process());
      enter_state(AppState::MainView);
      return switch_to_view(ViewMainDevelopment);

    case FilmDeveloperEvent::PauseRequested:
      if (current_state == AppState::MainView ||
          current_state == AppState::DispatchDialog) {
        if (model->pause_process()) {
          enter_state(AppState::Paused);
          return switch_to_view(ViewPaused);
        }
      }
      return true;

    case FilmDeveloperEvent::ResumeRequested:
      if (current_state == AppState::Paused ||
          current_state == AppState::DispatchDialog) {
        if (model->resume_process()) {
          enter_state(AppState::MainView);
          return switch_to_view(ViewMainDevelopment);
        }
      }
      return true;

    case FilmDeveloperEvent::EnterRuntimeSettings:
      enter_state(AppState::RuntimeSettings);
      return switch_to_view(ViewRuntimeSettings);

    case FilmDeveloperEvent::ExitRuntimeSettings:
      if (model->is_process_paused()) {
        enter_state(AppState::Paused);
        return switch_to_view(ViewPaused);
      }
      enter_state(AppState::MainView);
      return switch_to_view(ViewMainDevelopment);

    case FilmDeveloperEvent::UserActionRequired:
      // XXX not the cleanest way to do this, we should delegate entirely to
      // the process interpreter
      if (model->wait_for_user()) {
        show_waiting_confirmation_dialog();
        return true;
      }
      return true;

    case FilmDeveloperEvent::UserActionConfirmed:
      if (model->confirm_user_action()) {
        enter_state(before_confirmation_state);
        return switch_to_view(before_confirmation_view);
      }
      return true;

    case FilmDeveloperEvent::RestartRequested:
      // NOTE not sure we need to check for the current state here, or maybe
      // check for it negatively.
      if (current_state == AppState::MainView ||
          current_state == AppState::Paused ||
          current_state == AppState::DispatchDialog) {
        show_restart_confirmation_dialog();
      } else {
        FURI_LOG_I(APP_TAG,
                   "Restart requested, but not in a valid state, ignoring");
      }
      return true;

    case FilmDeveloperEvent::SkipRequested:
      if (current_state == AppState::MainView ||
          current_state == AppState::Paused ||
          current_state == AppState::DispatchDialog) {
        show_skip_confirmation_dialog();
      }
      return true;

    case FilmDeveloperEvent::StopProcessRequested:
      show_stop_confirmation_dialog();
      return true;

    case FilmDeveloperEvent::ExitRequested:
      show_exit_confirmation_dialog();
      return true;

    case FilmDeveloperEvent::DispatchDialogRequested:
      enter_state(AppState::DispatchDialog);
      return switch_to_view(ViewDispatchMenu);

    case FilmDeveloperEvent::RestartStep:
      model->process_interpreter->reset();
      if (model->is_process_paused()) {
        model->resume_process();
      }
      enter_state(AppState::MainView);
      return switch_to_view(ViewMainDevelopment);

    case FilmDeveloperEvent::SkipStep:
      model->process_interpreter->advanceToNextStep();
      if (model->is_process_paused()) {
        model->resume_process();
      }
      // XXX ideally we want to trigger a tick here, and restart the timer
      enter_state(AppState::MainView);
      return switch_to_view(ViewMainDevelopment);

    case FilmDeveloperEvent::StopProcess:
      model->stop_process();
      enter_state(AppState::ProcessSelection);
      return switch_to_view(ViewProcessSelection);

    case FilmDeveloperEvent::DispatchDialogConfirmed:
      switch (current_state) {
      case AppState::ConfirmRestart: {
        send_custom_event(FilmDeveloperEvent::RestartStep);
        return true;
      }
      case AppState::ConfirmSkip: {
        send_custom_event(FilmDeveloperEvent::SkipStep);
        return true;
      }
      case AppState::ConfirmExit: {
        send_custom_event(FilmDeveloperEvent::ExitRequested);
        return true;
      }
      default:
        return false;
      }

    case FilmDeveloperEvent::DispatchDialogDismissed:
      show_main_view(model->is_process_paused());
      return true;

    case FilmDeveloperEvent::StartProcess:
      if (model->start_process()) {
        enter_state(AppState::MainView);
        return switch_to_view(ViewMainDevelopment);
      }
      return true;

    case FilmDeveloperEvent::ProcessCompleted:
      if (model->complete_process()) {
        enter_state(AppState::ProcessSelection);
        return switch_to_view(ViewProcessSelection);
      }
      return true;

    case FilmDeveloperEvent::ExitApp:
      view_dispatcher_stop(view_dispatcher);
      return true;
    case FilmDeveloperEvent::PauseProcess:
    case FilmDeveloperEvent::ResumeProcess:
    case FilmDeveloperEvent::StepComplete:
      return false;

    case FilmDeveloperEvent::TimerTick:
    case FilmDeveloperEvent::MotorStateChanged:
    case FilmDeveloperEvent::AgitationComplete:
    case FilmDeveloperEvent::PushPullChanged:
    case FilmDeveloperEvent::RollCountChanged:
    case FilmDeveloperEvent::StepDurationChanged:
    case FilmDeveloperEvent::StateChanged:
      return false;
    }

    furi_assert(false);
    return false;
  }
};

FilmDeveloperApp::ViewMap FilmDeveloperApp::view_map[ViewCount] = {
    {
        ViewProcessSelection,
        nullptr,
    },
    {
        ViewSettings,
        nullptr,
    },
    {
        ViewMainDevelopment,
        nullptr,
    },
    {
        ViewConfirmationDialog,
        nullptr,
    },
    {
        ViewDispatchMenu,
        nullptr,
    },
    {
        ViewRuntimeSettings,
        nullptr,
    },
    {
        ViewPaused,
        nullptr,
    },
};

#ifdef __cplusplus
extern "C" {
#endif

int32_t film_developer_app(void *p) {
  UNUSED(p);
  FilmDeveloperApp app;
  app.init();
  app.run();
  return 0;
}

#ifdef __cplusplus
}
#endif
