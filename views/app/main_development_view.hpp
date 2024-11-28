#pragma once

#include "../../agitation_process_interpreter.hpp"
#include "../../models/main_view_model.hpp"
#include "../../motor_controller.hpp"
#include "../common/view_cpp.hpp"
#include <furi.h>
#include <furi_hal_resources.h>
#include <gui/elements.h>

class MainDevelopmentView : public flipper::ViewCpp {
public:
    MainDevelopmentView(ProtectedModel& model)
        : model(model) {
    }

private:
    ProtectedModel& model;

protected:
    void draw(Canvas* canvas, void*) override {
        UNUSED(model);
        auto m = model.lock();
        auto process_interpreter = m->process_interpreter;
        auto motor_controller = m->motor_controller;

        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);

        // Draw title
        canvas_draw_str(canvas, 2, 12, m->current_process->process_name);

        // Draw current step info
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 24, m->step_text);

        // Draw status or user message
        if(process_interpreter.isWaitingForUser()) {
            canvas_draw_str(canvas, 2, 36, process_interpreter.getUserMessage());
        } else {
            canvas_draw_str(canvas, 2, 36, m->status_text);
        }

        // Draw movement state if not waiting for user
        if(!process_interpreter.isWaitingForUser()) {
            canvas_draw_str(canvas, 2, 48, m->movement_text);
        }

        // Draw pin states
        if(motor_controller->isRunning()) {
            if(motor_controller->isClockwise()) {
                canvas_draw_str(canvas, 2, 60, "CW:");
            } else {
                canvas_draw_str(canvas, 2, 60, "CCW:");
            }
            canvas_draw_str(canvas, 50, 60, "ON");
        } else {
            canvas_draw_str(canvas, 2, 60, "CW/CCW:");
            canvas_draw_str(canvas, 50, 60, "OFF");
        }

        // Draw control hints
        if(m->process_active) {
            if(process_interpreter.isWaitingForUser()) {
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
        auto model = this->model.lock();
        auto process_interpreter = model->process_interpreter;
        auto motor_controller = model->motor_controller;

        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk) {
                // XXX should move to the main
                if(!model->process_active) {
                    // Start new process
                    process_interpreter.init(model->current_process, motor_controller);
                    model->process_active = true;
                    model->paused = false;
                } else if(process_interpreter.isWaitingForUser()) {
                    // Handle user confirmation
                    process_interpreter.confirm();
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
                if(!process_interpreter.isWaitingForUser()) {
                    motor_controller->stop();
                    process_interpreter.skipToNextStep();
                    if(process_interpreter.getCurrentStepIndex() >=
                       model->current_process->steps_length) {
                        model->process_active = false;
                    }
                }
                return true;
            } else if(model->process_active && event->key == InputKeyLeft) {
                // Restart current step
                motor_controller->stop();
                process_interpreter.reset();
                return true;
            }
        }
        return false;
    }
};
