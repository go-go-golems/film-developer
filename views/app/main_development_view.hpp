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
        FURI_LOG_T("MainView", "Drawing");
        auto m = model.lock();
        auto* process_interpreter = m->process_interpreter;
        auto* motor_controller = m->motor_controller;

        if (!process_interpreter || !motor_controller) {
            return;
        }

        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);

        // Draw title
        canvas_draw_str(canvas, 2, 12, m->current_process->process_name);

        // Draw current step info
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 24, m->step_text);

        // Draw status or user message
        if(m->is_waiting_for_user()) {
            canvas_draw_str(canvas, 2, 36, process_interpreter->getUserMessage());
        } else {
            canvas_draw_str(canvas, 2, 36, m->status_text);
        }

        // Draw movement state if not waiting for user
        if(!m->is_waiting_for_user()) {
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

        // Draw control hint - only show OK button hint
        if(m->is_process_active()) {
            if(m->is_waiting_for_user()) {
                elements_button_center(canvas, "Continue");
            } else if(m->is_process_paused()) {
                elements_button_center(canvas, "Resume");
            } else {
                elements_button_center(canvas, "Menu");
            }
        } else {
            elements_button_center(canvas, "Start");
        }
    }

    bool input(InputEvent* event) override {
        if(event->type != InputTypeShort) {
            return false;
        }

        auto m = model.lock();

        switch(event->key) {
        case InputKeyOk:
            if(!m->is_process_active()) {
                // Start new process
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::StartProcess));
            } else if(m->is_waiting_for_user()) {
                // Confirm user action
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::UserActionConfirmed));
            } else {
                // Show dispatch menu
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::DispatchRequested));
            }
            return true;

        case InputKeyBack:
            if(m->is_process_active()) {
                // Show abort confirmation
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessAborted));
            }
            return true;

        case InputKeyLeft:
            if(m->is_process_active()) {
                // Show restart confirmation
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::RestartStep));
            }
            return true;

        case InputKeyRight:
            if(m->is_process_active()) {
                // Show skip confirmation
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::SkipStep));
            }
            return true;

        default:
            return false;
        }
    }

    bool custom(uint32_t event) override {
        // DO NOT REMOVE THIS!
        // This triggers a redraw when the model is updated.
        if(event == static_cast<uint32_t>(FilmDeveloperEvent::TimerTick)) {
            // trigger a redraw
            auto model = this->get_model<Model>();
            UNUSED(model);
            return true;
        }

        return false;
    }
};
