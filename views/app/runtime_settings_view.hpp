#pragma once

#include "../common/view_cpp.hpp"
#include "../../film_developer_events.hpp"
#include <gui/canvas.h>
#include <gui/elements.h>

class RuntimeSettingsView : public flipper::ViewCpp {
public:
    void draw(Canvas* canvas, void*) override {
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        
        // Draw title
        canvas_draw_str(canvas, 2, 12, "Runtime Settings");
        
        // Draw placeholder message
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 32, "Press Back to return");
        canvas_draw_str(canvas, 2, 44, "Settings coming soon...");
        
        // Draw back button hint
        elements_button_left(canvas, "Back");
    }

    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyBack || event->key == InputKeyLeft) {
                send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ExitRuntimeSettings));
                return true;
            }
        }
        return false;
    }
}; 