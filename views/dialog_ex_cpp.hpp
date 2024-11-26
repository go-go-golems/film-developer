#pragma once

#include <gui/modules/dialog_ex.h>
#include "view_cpp.hpp"

namespace flipper {

class DialogExCpp : public ViewCpp {
public:
    DialogExCpp() {
    }

    ~DialogExCpp() {
        if(dialog) {
            dialog_ex_free(dialog);
            view = nullptr;
        }
    }

    void init() override {
        // Instead of ViewCpp::init(), we'll use the dialog's view
        dialog = dialog_ex_alloc();
        view = dialog_ex_get_view(dialog);
        dialog_ex_set_context(dialog, this);
    }

    void set_header(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) {
        dialog_ex_set_header(dialog, text, x, y, horizontal, vertical);
    }

    void set_text(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) {
        dialog_ex_set_text(dialog, text, x, y, horizontal, vertical);
    }

    void set_left_button_text(const char* text) {
        dialog_ex_set_left_button_text(dialog, text);
    }

    void set_center_button_text(const char* text) {
        dialog_ex_set_center_button_text(dialog, text);
    }

    void set_right_button_text(const char* text) {
        dialog_ex_set_right_button_text(dialog, text);
    }

    void set_result_callback(DialogExResultCallback callback) {
        dialog_ex_set_result_callback(dialog, callback);
    }

    void reset() {
        dialog_ex_reset(dialog);
    }

protected:
    // These are handled by the dialog internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    DialogEx* dialog = nullptr;
};

} // namespace flipper
