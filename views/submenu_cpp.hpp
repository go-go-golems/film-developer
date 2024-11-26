#pragma once

#include <gui/modules/submenu.h>
#include "view_cpp.hpp"

namespace flipper {

class SubMenuCpp : public ViewCpp {
public:
    SubMenuCpp() {
    }

    ~SubMenuCpp() {
        if(submenu) {
            submenu_free(submenu);
            view = nullptr;
        }
    }

    void init() override {
        // Instead of ViewCpp::init(), we'll use the submenu's view
        submenu = submenu_alloc();
        view = submenu_get_view(submenu);
    }

    void add_item(const char* label, uint32_t index, SubmenuItemCallback callback, void* context) {
        submenu_add_item(submenu, label, index, callback, context);
    }

    void set_selected_item(uint32_t index) {
        submenu_set_selected_item(submenu, index);
    }

    uint32_t get_selected_item() {
        return submenu_get_selected_item(submenu);
    }

    void set_header(const char* header) {
        submenu_set_header(submenu, header);
    }

    void reset() {
        submenu_reset(submenu);
    }

protected:
    // These are handled by the submenu internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    Submenu* submenu = nullptr;
};

} // namespace flipper
