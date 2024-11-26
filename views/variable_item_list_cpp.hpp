#pragma once

#include <gui/modules/variable_item_list.h>
#include "view_cpp.hpp"

namespace flipper {

class VariableItemListCpp : public ViewCpp {
public:
    VariableItemListCpp() {
    }

    ~VariableItemListCpp() {
        if(variable_item_list) {
            variable_item_list_free(variable_item_list);
            view = nullptr;
        }
    }

    void init() override {
        // Instead of ViewCpp::init(), we'll use the variable_item_list's view
        variable_item_list = variable_item_list_alloc();
        view = variable_item_list_get_view(variable_item_list);
    }

    VariableItem* add_item(
        const char* label,
        uint8_t values_count,
        VariableItemChangeCallback change_callback,
        void* context) {
        return variable_item_list_add(variable_item_list, label, values_count, change_callback, context);
    }

    void set_enter_callback(VariableItemListEnterCallback callback, void* context) {
        variable_item_list_set_enter_callback(variable_item_list, callback, context);
    }

    void set_selected_item(uint8_t index) {
        variable_item_list_set_selected_item(variable_item_list, index);
    }

    uint8_t get_selected_item_index() {
        return variable_item_list_get_selected_item_index(variable_item_list);
    }

    void reset() {
        variable_item_list_reset(variable_item_list);
    }

    // Helper methods for working with items
    static void set_current_value_index(VariableItem* item, uint8_t current_value_index) {
        variable_item_set_current_value_index(item, current_value_index);
    }

    static void set_values_count(VariableItem* item, uint8_t values_count) {
        variable_item_set_values_count(item, values_count);
    }

    static void set_current_value_text(VariableItem* item, const char* current_value_text) {
        variable_item_set_current_value_text(item, current_value_text);
    }

    static uint8_t get_current_value_index(VariableItem* item) {
        return variable_item_get_current_value_index(item);
    }

    static void* get_context(VariableItem* item) {
        return variable_item_get_context(item);
    }

protected:
    // These are handled by the variable_item_list internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    VariableItemList* variable_item_list = nullptr;
};

} // namespace flipper 