#include <furi.h>
#include <furi_hal_gpio.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view_port.h>
#include "agitation_sequence.hpp"
#include "agitation_processes.hpp"
#include "agitation_process_interpreter.hpp"
#include "motor_controller.hpp"

#ifdef HOST
#include "test-film_developer/mock_controller.hpp"
#else
#include "embedded/motor_controller_embedded.hpp"
#endif

typedef struct {
    FuriEventLoop* event_loop;
    ViewPort* view_port;
    Gui* gui;
    FuriEventLoopTimer* state_timer;

    // Add motor controller
    MotorController* motor_controller;

    // Process state
    AgitationProcessInterpreterState process_state;
    const AgitationProcessStatic* current_process;
    bool process_active;

    // Display info
    char status_text[64];
    char step_text[32];
    char movement_text[32];

    // Additional state tracking
    bool paused;
} FilmDeveloperApp;

// Add motor control callback wrappers
static void motor_cw_callback(bool enable) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(furi_record_open("film_developer"));
    app->motor_controller->clockwise(enable);
    furi_record_close("film_developer");
}

static void motor_ccw_callback(bool enable) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(furi_record_open("film_developer"));
    app->motor_controller->counterClockwise(enable);
    furi_record_close("film_developer");
}

static void draw_callback(Canvas* canvas, void* context) {
    FilmDeveloperApp* app = (FilmDeveloperApp*)context;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    // Draw title
    canvas_draw_str(canvas, 2, 12, "C41 Process");

    // Draw current step info
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 24, app->step_text);

    // Draw status or user message
    if(app->process_state.waiting_for_user) {
        canvas_draw_str(
            canvas,
            2,
            36,
            app->process_state.user_message ? app->process_state.user_message :
                                              "Press OK to continue");
    } else {
        canvas_draw_str(canvas, 2, 36, app->status_text);
    }

    // Draw movement state if not waiting for user
    if(!app->process_state.waiting_for_user) {
        canvas_draw_str(canvas, 2, 48, app->movement_text);
    }

    // Draw pin states
    canvas_draw_str(canvas, 2, 60, "CW:");
    canvas_draw_str(canvas, 50, 60, app->motor_controller->isRunning() ? "ON" : "OFF");

    canvas_draw_str(canvas, 2, 70, "CCW:");
    canvas_draw_str(canvas, 50, 70, app->motor_controller->isRunning() ? "ON" : "OFF");

    // Draw control hints
    if(app->process_active) {
        if(app->process_state.waiting_for_user) {
            elements_button_center(canvas, "Continue");
        } else if(app->paused) {
            elements_button_center(canvas, "Resume");
        } else {
            elements_button_center(canvas, "Pause");
        }
        elements_button_right(canvas, "SkIP");
        elements_button_left(canvas, "Restart");
    } else {
        elements_button_center(canvas, "Start");
    }
}

static void timer_callback(void* context) {
    FilmDeveloperApp* app = (FilmDeveloperApp*)context;

    if(app->process_active && !app->paused) {
        bool still_active = agitation_process_interpreter_tick(&app->process_state);

        // Update status texts
        const AgitationStepStatic* current_step =
            &app->current_process->steps[app->process_state.current_step_index];

        snprintf(app->step_text, sizeof(app->step_text), "Step: %s", current_step->name);

        // Get current loop info if in a loop
        if(app->process_state.movement_interpreter.loop_depth > 0) {
            // Show elapsed time in current loop
            uint32_t elapsed =
                app->process_state.movement_interpreter
                    .loop_stack[app->process_state.movement_interpreter.loop_depth - 1]
                    .elapsed_duration;
            uint32_t max_duration =
                app->process_state.movement_interpreter
                    .loop_stack[app->process_state.movement_interpreter.loop_depth - 1]
                    .max_duration;

            if(max_duration > 0) {
                snprintf(
                    app->status_text,
                    sizeof(app->status_text),
                    "%s Time: %lus/%lus",
                    app->paused ? "[PAUSED]" : "",
                    elapsed,
                    max_duration);
            } else {
                snprintf(
                    app->status_text,
                    sizeof(app->status_text),
                    "%s Time: %lus",
                    app->paused ? "[PAUSED]" : "",
                    elapsed);
            }
        } else {
            // Show remaining time for current movement
            snprintf(
                app->status_text,
                sizeof(app->status_text),
                "%s Time left: %lus",
                app->paused ? "[PAUSED]" : "",
                app->process_state.movement_interpreter.time_remaining);
        }

        // Update movement text based on motor controller state
        const char* movement_str = "Idle";
        if(app->motor_controller->isRunning()) {
            movement_str = "Running"; // You may want to add direction detection to MotorController
        }
        snprintf(app->movement_text, sizeof(app->movement_text), "Movement: %s", movement_str);

        app->process_active = still_active;
        if(!still_active) {
            app->motor_controller->stop();
        }
    }

    view_port_update(app->view_port);
}

static void input_callback(InputEvent* input_event, void* context) {
    FilmDeveloperApp* app = (FilmDeveloperApp*)context;

    if(input_event->type == InputTypeShort) {
        if(input_event->key == InputKeyOk) {
            if(!app->process_active) {
                // Start new process
                agitation_process_interpreter_init(
                    &app->process_state,
                    &C41_FULL_PROCESS_STATIC,
                    app->motor_controller, // Pass motor controller
                    motor_cw_callback,
                    motor_ccw_callback);
                app->process_active = true;
                app->paused = false;
            } else if(app->process_state.waiting_for_user) {
                // Handle user confirmation
                agitation_process_interpreter_confirm(&app->process_state);
            } else {
                // Toggle pause
                app->paused = !app->paused;
                if(app->paused) {
                    app->motor_controller->stop();
                }
            }
        } else if(app->process_active && input_event->key == InputKeyRight) {
            // Skip to next step (only if not waiting for user)
            if(!app->process_state.waiting_for_user) {
                app->motor_controller->stop();
                app->process_state.current_step_index++;
                app->process_state.process_state = AgitationProcessStateIdle;
                if(app->process_state.current_step_index >= app->current_process->steps_length) {
                    app->process_active = false;
                }
            }
        } else if(app->process_active && input_event->key == InputKeyLeft) {
            // Restart current step
            app->motor_controller->stop();
            agitation_process_interpreter_init(
                &app->process_state, app->current_process, motor_cw_callback, motor_ccw_callback);
            app->process_state.current_step_index =
                app->process_state.current_step_index; // Stay on current step
        }
    } else if(input_event->type == InputTypeShort && input_event->key == InputKeyBack) {
        if(app->process_active) {
            // Stop process
            app->process_active = false;
            app->paused = false;
            app->motor_controller->stop();
        } else {
            furi_event_loop_stop(app->event_loop);
        }
    }
}

#ifdef __cplusplus
extern "C" {
#endif

int32_t film_developer_app(void* p) {
    UNUSED(p);
    FilmDeveloperApp* app = (FilmDeveloperApp*)malloc(sizeof(FilmDeveloperApp));

    // Create appropriate motor controller
#ifdef HOST
    app->motor_controller = new MockController();
#else
    app->motor_controller = new MotorControllerEmbedded();
#endif

    // Register app instance for callbacks
    furi_record_create("film_developer", app);

    // Create event loop
    app->event_loop = furi_event_loop_alloc();

    // Create GUI
    app->gui = (Gui*)furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // Create timer
    app->state_timer = furi_event_loop_timer_alloc(
        app->event_loop, timer_callback, FuriEventLoopTimerTypePeriodic, app);
    furi_event_loop_timer_start(app->state_timer, 1000); // 1 second intervals

    // Set initial state
    app->current_process = &C41_FULL_PROCESS_STATIC;
    app->process_active = false;
    app->paused = false;
    snprintf(app->status_text, sizeof(app->status_text), "Press OK to start");
    snprintf(app->step_text, sizeof(app->step_text), "Ready");
    snprintf(app->movement_text, sizeof(app->movement_text), "Movement: Idle");

    // furi_assert(false, "Hello");

    // Run event loop
    furi_event_loop_run(app->event_loop);

    // Cleanup
    furi_event_loop_timer_free(app->state_timer);
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_event_loop_free(app->event_loop);

    // Clean up motor controller
    delete app->motor_controller;
    furi_record_destroy("film_developer");

    free(app);

    return 0;
}

#ifdef __cplusplus
}
#endif
