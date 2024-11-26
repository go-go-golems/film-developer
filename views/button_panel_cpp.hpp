#pragma once

#include <gui/modules/button_panel.h>
#include "view_cpp.hpp"

namespace flipper {

class ButtonPanelCpp : public ViewCpp {
public:
    ButtonPanelCpp() {
    }

    ~ButtonPanelCpp() {
        if(button_panel) {
            button_panel_free(button_panel);
            view = nullptr;
        }
    }

    void init() override {
        // Instead of ViewCpp::init(), we'll use the button panel's view
        button_panel = button_panel_alloc();
        view = button_panel_get_view(button_panel);
    }

    void reserve(size_t reserve_x, size_t reserve_y) {
        button_panel_reserve(button_panel, reserve_x, reserve_y);
    }

    void add_item(
        uint32_t index,
        uint16_t matrix_place_x,
        uint16_t matrix_place_y,
        uint16_t x,
        uint16_t y,
        const Icon* icon_name,
        const Icon* icon_name_selected,
        ButtonItemCallback callback,
        void* callback_context) {
        button_panel_add_item(
            button_panel,
            index,
            matrix_place_x,
            matrix_place_y,
            x,
            y,
            icon_name,
            icon_name_selected,
            callback,
            callback_context);
    }

    void add_label(uint16_t x, uint16_t y, Font font, const char* label_str) {
        button_panel_add_label(button_panel, x, y, font, label_str);
    }

    void add_icon(uint16_t x, uint16_t y, const Icon* icon_name) {
        button_panel_add_icon(button_panel, x, y, icon_name);
    }

    void reset() {
        button_panel_reset(button_panel);
    }

protected:
    // These are handled by the button panel internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    ButtonPanel* button_panel = nullptr;
};

} // namespace flipper 