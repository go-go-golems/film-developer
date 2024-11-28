#pragma once

#include <gui/modules/popup.h>
#include "view_cpp.hpp"

namespace flipper {

class PopupCpp : public ViewCpp {
public:
    PopupCpp() {
    }

    ~PopupCpp() {
        if(popup) {
            popup_free(popup);
            view = nullptr;
        }
    }

    void init() override {
        popup = popup_alloc();
        view = popup_get_view(popup);
    }

    void set_callback(PopupCallback callback) {
        popup_set_callback(popup, callback);
    }

    void set_context(void* context) {
        popup_set_context(popup, context);
    }

    void set_header(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) {
        popup_set_header(popup, text, x, y, horizontal, vertical);
    }

    void set_text(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) {
        popup_set_text(popup, text, x, y, horizontal, vertical);
    }

    void set_icon(uint8_t x, uint8_t y, const Icon* icon) {
        popup_set_icon(popup, x, y, icon);
    }

    void set_timeout(uint32_t timeout_in_ms) {
        popup_set_timeout(popup, timeout_in_ms);
    }

    void enable_timeout() {
        popup_enable_timeout(popup);
    }

    void disable_timeout() {
        popup_disable_timeout(popup);
    }

    void reset() {
        popup_reset(popup);
    }

protected:
    // These are handled by the popup internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    Popup* popup = nullptr;
};

} // namespace flipper 