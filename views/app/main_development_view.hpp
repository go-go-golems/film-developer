#pragma once

#include "../../agitation/agitation_process_interpreter.hpp"
#include "../../models/main_view_model.hpp"
#include "../../motor_controller.hpp"
#include "../../film_developer_events.hpp"
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
        FURI_LOG_D("MainView", "Drawing");
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
            FURI_LOG_D("MainView", "Input received: key=%d", event->key);

            if(event->key == InputKeyOk) {
                if(!model->process_active) {
                    FURI_LOG_I(
                        "MainView",
                        "Starting new process: %s",
                        model->current_process->process_name);
                    process_interpreter.init(model->current_process, motor_controller);
                    model->process_active = true;
                    model->paused = false;
                } else if(process_interpreter.isWaitingForUser()) {
                    FURI_LOG_I("MainView", "User confirmed action");
                    process_interpreter.confirm();
                } else {
                    model->paused = !model->paused;
                    FURI_LOG_I("MainView", "Process %s", model->paused ? "paused" : "resumed");
                    if(model->paused) {
                        motor_controller->stop();
                    }
                }
                return true;
            } else if(model->process_active && event->key == InputKeyRight) {
                if(!process_interpreter.isWaitingForUser()) {
                    FURI_LOG_I("MainView", "Skipping to next step");
                    motor_controller->stop();
                    process_interpreter.skipToNextStep();
                    if(process_interpreter.getCurrentStepIndex() >=
                       model->current_process->steps_length) {
                        FURI_LOG_I("MainView", "Process completed via skip");
                        model->process_active = false;
                    }
                }
                return true;
            } else if(model->process_active && event->key == InputKeyLeft) {
                FURI_LOG_I("MainView", "Restarting current step");
                motor_controller->stop();
                process_interpreter.reset();
                return true;
            }
        }
        return false;
    }

    bool custom(uint32_t event) override {
        FURI_LOG_D("MainView", "Custom event received: %lu", static_cast<unsigned long>(event));
        if(event == static_cast<uint32_t>(FilmDeveloperEvent::TimerTick)) {
            // trigger a redraw
            auto model = this->get_model<Model>();
            UNUSED(model);
            return true;
        }
        return false;
    }
};
