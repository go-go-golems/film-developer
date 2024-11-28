#pragma once

#include <gui/modules/text_box.h>
#include "view_cpp.hpp"

namespace flipper {

class TextBoxCpp : public ViewCpp {
public:
    TextBoxCpp() {
    }

    ~TextBoxCpp() {
        if(text_box) {
            text_box_free(text_box);
            view = nullptr;
        }
    }

    void init() override {
        text_box = text_box_alloc();
        view = text_box_get_view(text_box);
    }

    void set_text(const char* text) {
        text_box_set_text(text_box, text);
    }

    void set_font(TextBoxFont font) {
        text_box_set_font(text_box, font);
    }

    void set_focus(TextBoxFocus focus) {
        text_box_set_focus(text_box, focus);
    }

    void reset() {
        text_box_reset(text_box);
    }

protected:
    // These are handled by the text_box internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    TextBox* text_box = nullptr;
};

} // namespace flipper 