#pragma once

#include <gui/modules/byte_input.h>
#include "view_cpp.hpp"

namespace flipper {

class ByteInputCpp : public ViewCpp {
public:
    ByteInputCpp() {
    }

    ~ByteInputCpp() {
        if(byte_input) {
            byte_input_free(byte_input);
            view = nullptr;
        }
    }

    void init() override {
        byte_input = byte_input_alloc();
        view = byte_input_get_view(byte_input);
    }

    void set_result_callback(
        ByteInputCallback input_callback,
        ByteChangedCallback changed_callback,
        void* callback_context,
        uint8_t* bytes,
        uint8_t bytes_count) {
        byte_input_set_result_callback(
            byte_input, input_callback, changed_callback, callback_context, bytes, bytes_count);
    }

    void set_header_text(const char* text) {
        byte_input_set_header_text(byte_input, text);
    }

protected:
    // These are handled by the byte input internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    ByteInput* byte_input = nullptr;
};

} // namespace flipper 