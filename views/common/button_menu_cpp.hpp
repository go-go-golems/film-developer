#pragma once

#include <gui/modules/button_menu.h>
#include "view_cpp.hpp"

namespace flipper {

class ButtonMenuCpp : public ViewCpp {
public:
    ButtonMenuCpp() {
    }

    ~ButtonMenuCpp() {
        if(button_menu) {
            button_menu_free(button_menu);
            view = nullptr;
        }
    }

    void init() override {
        button_menu = button_menu_alloc();
        view = button_menu_get_view(button_menu);
    }

    ButtonMenuItem* add_item(
        const char* label,
        int32_t index,
        ButtonMenuItemCallback callback,
        ButtonMenuItemType type,
        void* callback_context) {
        return button_menu_add_item(button_menu, label, index, callback, type, callback_context);
    }

    void set_selected_item(uint32_t index) {
        button_menu_set_selected_item(button_menu, index);
    }

    void set_header(const char* header) {
        button_menu_set_header(button_menu, header);
    }

    void reset() {
        button_menu_reset(button_menu);
    }

protected:
    // These are handled by the button menu internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    ButtonMenu* button_menu = nullptr;
};

} // namespace flipper 