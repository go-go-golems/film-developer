#pragma once

#include "view_cpp.hpp"
#include "../models/main_view_model.hpp"
#include "../agitation_process_interpreter.hpp"
#include "../motor_controller.hpp"
#include <gui/elements.h>
#include <furi.h>
#include <furi_hal_resources.h>

class MainDevelopmentView : public flipper::ViewCpp {
private:
    MainViewModel* model{nullptr};
    AgitationProcessInterpreter* process_interpreter{nullptr};
    MotorController* motor_controller{nullptr};
    FuriEventLoopTimer* timer{nullptr};
    FuriEventLoop* event_loop{nullptr};

    static void timer_callback(void* context) {
        auto view = static_cast<MainDevelopmentView*>(context);
        view->update();
    }

    void update() {
        if(model->process_active && !model->paused) {
            bool still_active = process_interpreter->tick();

            // Update status texts
            const AgitationStepStatic* current_step =
                &model->current_process->steps[process_interpreter->getCurrentStepIndex()];

            model->update_step_text(current_step->name);
            model->update_status(
                process_interpreter->getCurrentMovementTimeElapsed(),
                process_interpreter->getCurrentMovementDuration());
            model->update_movement_text(motor_controller->getDirectionString());

            model->process_active = still_active;
            if(!still_active) {
                motor_controller->stop();
            }
        }

        view_commit_model(view, true);
    }

protected:
    void draw(Canvas* canvas, void* model) override {
        auto m = static_cast<MainViewModel*>(model);

        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);

        // Draw title
        canvas_draw_str(canvas, 2, 12, "C41 Process");

        // Draw current step info
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 24, m->step_text);

        // Draw status or user message
        if(process_interpreter && process_interpreter->isWaitingForUser()) {
            canvas_draw_str(canvas, 2, 36, process_interpreter->getUserMessage());
        } else {
            canvas_draw_str(canvas, 2, 36, m->status_text);
        }

        // Draw movement state if not waiting for user
        if(!process_interpreter || !process_interpreter->isWaitingForUser()) {
            canvas_draw_str(canvas, 2, 48, m->movement_text);
        }

        // Draw pin states
        canvas_draw_str(canvas, 2, 60, "CW:");
        canvas_draw_str(canvas, 50, 60, motor_controller->isRunning() ? "ON" : "OFF");

        canvas_draw_str(canvas, 2, 70, "CCW:");
        canvas_draw_str(canvas, 50, 70, motor_controller->isRunning() ? "ON" : "OFF");

        // Draw control hints
        if(m->process_active) {
            if(process_interpreter->isWaitingForUser()) {
                elements_button_center(canvas, "Continue");
            } else if(m->paused) {
                elements_button_center(canvas, "Resume");
            } else {
                elements_button_center(canvas, "Pause");
            }
            elements_button_right(canvas, "Skip");
            elements_button_left(canvas, "Restart");
        } else {
            elements_button_center(canvas, "Start");
        }
    }

    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk) {
                if(!model->process_active) {
                    // Start new process
                    process_interpreter->init(model->current_process, motor_controller);
                    model->process_active = true;
                    model->paused = false;
                } else if(process_interpreter->isWaitingForUser()) {
                    // Handle user confirmation
                    process_interpreter->confirm();
                } else {
                    // Toggle pause
                    model->paused = !model->paused;
                    if(model->paused) {
                        motor_controller->stop();
                    }
                }
                return true;
            } else if(model->process_active && event->key == InputKeyRight) {
                // Skip to next step (only if not waiting for user)
                if(!process_interpreter->isWaitingForUser()) {
                    motor_controller->stop();
                    process_interpreter->skipToNextStep();
                    if(process_interpreter->getCurrentStepIndex() >=
                       model->current_process->steps_length) {
                        model->process_active = false;
                    }
                }
                return true;
            } else if(model->process_active && event->key == InputKeyLeft) {
                // Restart current step
                motor_controller->stop();
                process_interpreter->reset();
                return true;
            }
        }
        return false;
    }

public:
    void init() override {
        ViewCpp::init();
        model = new MainViewModel();
        process_interpreter = new AgitationProcessInterpreter();
        set_model(model);
        event_loop = furi_event_loop_alloc();

        // Move this to the app itself
        // Create timer for periodic updates
        timer = furi_event_loop_timer_alloc(
            event_loop, timer_callback, FuriEventLoopTimerTypePeriodic, this);
        furi_event_loop_timer_start(timer, 1000); // 1 second intervals

        model->reset();
    }

    ~MainDevelopmentView() {
        if(timer) {
            furi_event_loop_timer_free(timer);
        }
        delete process_interpreter;
    }

    void set_motor_controller(MotorController* controller) {
        motor_controller = controller;
    }
};
