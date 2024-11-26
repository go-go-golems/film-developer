#pragma once

#include <gui/modules/number_input.h>
#include "view_cpp.hpp"

namespace flipper {

class NumberInputCpp : public ViewCpp {
public:
    NumberInputCpp() {
    }

    ~NumberInputCpp() {
        if(number_input) {
            number_input_free(number_input);
            view = nullptr;
        }
    }

    void init() override {
        number_input = number_input_alloc();
        view = number_input_get_view(number_input);
    }

    void set_result_callback(
        NumberInputCallback input_callback,
        void* callback_context,
        int32_t current_number,
        int32_t min_value,
        int32_t max_value) {
        number_input_set_result_callback(
            number_input, input_callback, callback_context, current_number, min_value, max_value);
    }

    void set_header_text(const char* text) {
        number_input_set_header_text(number_input, text);
    }

protected:
    // These are handled by the number input internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    NumberInput* number_input = nullptr;
};

} // namespace flipper 