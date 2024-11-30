#pragma once

#include "../../film_developer_events.hpp"
#include "../../models/main_view_model.hpp"
#include "../../motor_controller.hpp"
#include "../common/view_cpp.hpp"
#include <furi.h>
#include <furi_hal_resources.h>
#include <gui/elements.h>

#define MAIN_VIEW_TAG "MainView"
class MainDevelopmentView : public flipper::ViewCpp {
public:
    MainDevelopmentView(ProtectedModel& model)
        : model(model) {
    }

private:
    ProtectedModel& model;

protected:
    void draw(Canvas* canvas, void*) override {
        FURI_LOG_T(MAIN_VIEW_TAG, "Drawing");
        auto m = model.lock();
        auto* process_interpreter = m->process_interpreter;
        auto* motor_controller = m->motor_controller;

        if(!process_interpreter || !motor_controller) {
            return;
        }

        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);

        // Draw title
        char process_name[32];
        process_interpreter->getProcessName(
            process_interpreter->getCurrentProcessIndex(), process_name, sizeof(process_name));
        canvas_draw_str(canvas, 2, 12, process_name);

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
        FURI_LOG_T(MAIN_VIEW_TAG, "Drawing done");
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
                send_custom_event(
                    static_cast<uint32_t>(FilmDeveloperEvent::DispatchDialogRequested));
            }
            return true;

        case InputKeyBack:
            if(m->is_process_active()) {
                // Show abort confirmation
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::StopProcessRequested));
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
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::SkipRequested));
            }
            return true;

        default:
            return false;
        }
    }

    bool custom(uint32_t event) override {
        if(event == static_cast<uint32_t>(FilmDeveloperEvent::TimerTick)) {
            FURI_LOG_T(MAIN_VIEW_TAG, "Timer tick event");
            redraw();
            return true;
        }
        return false;
    }

    void redraw() {
        // DO NOT REMOVE THIS!
        // This triggers a redraw when the model is updated.
        // trigger a redraw because when the handle is deleted, the model is committed and the view is updated
        auto model = this->get_model<Model>();
        UNUSED(model);
    }

    void enter() override {
        FURI_LOG_D(MAIN_VIEW_TAG, "Entering");
        redraw();
        FURI_LOG_D(MAIN_VIEW_TAG, "Entering done");
    }
};
