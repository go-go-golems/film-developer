#pragma once

#include <gui/modules/text_input.h>
#include "view_cpp.hpp"

namespace flipper {

class TextInputCpp : public ViewCpp {
public:
    TextInputCpp() {
    }

    ~TextInputCpp() {
        if(text_input) {
            text_input_free(text_input);
            view = nullptr;
        }
    }

    void init() override {
        text_input = text_input_alloc();
        view = text_input_get_view(text_input);
    }

    void set_result_callback(
        TextInputCallback callback,
        void* callback_context,
        char* text_buffer,
        size_t text_buffer_size,
        bool clear_default_text) {
        text_input_set_result_callback(
            text_input, callback, callback_context, text_buffer, text_buffer_size, clear_default_text);
    }

    void set_header_text(const char* text) {
        text_input_set_header_text(text_input, text);
    }

    void set_minimum_length(size_t minimum_length) {
        text_input_set_minimum_length(text_input, minimum_length);
    }

    void set_validator(TextInputValidatorCallback callback, void* callback_context) {
        text_input_set_validator(text_input, callback, callback_context);
    }

    void reset() {
        text_input_reset(text_input);
    }

protected:
    // These are handled by the text input internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    TextInput* text_input = nullptr;
};

} // namespace flipper 