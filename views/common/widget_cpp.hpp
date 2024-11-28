#pragma once

#include <gui/modules/widget.h>
#include "view_cpp.hpp"

namespace flipper {

class WidgetCpp : public ViewCpp {
public:
    WidgetCpp() {
    }

    ~WidgetCpp() {
        if(widget) {
            widget_free(widget);
            view = nullptr;
        }
    }

    void init() override {
        // Instead of ViewCpp::init(), we'll use the widget's view
        widget = widget_alloc();
        view = widget_get_view(widget);
    }

    void reset() {
        widget_reset(widget);
    }

    void add_string_multiline(
        uint8_t x,
        uint8_t y,
        Align horizontal,
        Align vertical,
        Font font,
        const char* text) {
        widget_add_string_multiline_element(widget, x, y, horizontal, vertical, font, text);
    }

    void add_string(
        uint8_t x,
        uint8_t y,
        Align horizontal,
        Align vertical,
        Font font,
        const char* text) {
        widget_add_string_element(widget, x, y, horizontal, vertical, font, text);
    }

    void add_text_box(
        uint8_t x,
        uint8_t y,
        uint8_t width,
        uint8_t height,
        Align horizontal,
        Align vertical,
        const char* text,
        bool strip_to_dots = false) {
        widget_add_text_box_element(
            widget, x, y, width, height, horizontal, vertical, text, strip_to_dots);
    }

    void add_text_scroll(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char* text) {
        widget_add_text_scroll_element(widget, x, y, width, height, text);
    }

    void add_button(
        GuiButtonType button_type,
        const char* text,
        ButtonCallback callback,
        void* context) {
        widget_add_button_element(widget, button_type, text, callback, context);
    }

    void add_icon(uint8_t x, uint8_t y, const Icon* icon) {
        widget_add_icon_element(widget, x, y, icon);
    }

    void add_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius) {
        widget_add_frame_element(widget, x, y, width, height, radius);
    }

protected:
    // These are handled by the widget internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    Widget* widget = nullptr;
};

} // namespace flipper
