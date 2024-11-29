#pragma once

#include "../../models/main_view_model.hpp"
#include "../common/view_cpp.hpp"
#include "../../film_developer_events.hpp"
#include <gui/canvas.h>
#include <gui/elements.h>

class PausedView : public flipper::ViewCpp {
public:
    PausedView(ProtectedModel& model)
        : model(model) {
    }

protected:
    void draw(Canvas* canvas, void*) override {
        auto m = model.lock();

        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);

        // Draw PAUSED header
        canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignCenter, "PAUSED");

        // Draw current step info
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 24, m->step_text);

        // Draw elapsed time
        canvas_draw_str(canvas, 2, 36, m->status_text);

        // Draw control hints
        elements_button_center(canvas, "Menu");
        elements_button_right(canvas, "Resume");
        elements_button_left(canvas, "Settings");
    }

    bool input(InputEvent* event) override {
        if(event->type != InputTypeShort) {
            return false;
        }

        switch(event->key) {
        case InputKeyOk:
            send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::DispatchRequested));
            return true;

        case InputKeyRight:
            send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ResumeRequested));
            return true;

        case InputKeyLeft:
            send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::EnterRuntimeSettings));
            return true;

        case InputKeyBack:
            send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessAborted));
            return true;

        default:
            return false;
        }
    }

private:
    ProtectedModel& model;
};
