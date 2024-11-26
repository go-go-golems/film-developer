#pragma once

#include "variable_item_list_cpp.hpp"
#include "../models/process_settings_model.hpp"
#include "../film_developer_events.hpp"

class SettingsView : public flipper::VariableItemListCpp {
public:
    void init() override {
        VariableItemListCpp::init();
        model = new ProcessSettingsModel();

        // Add push/pull setting
        push_pull_item = add_item(
            "Push/Pull",
            ProcessSettingsModel::PUSH_PULL_COUNT,
            push_pull_change_callback,
            this);
        set_current_value_index(push_pull_item, 2); // Default to 0 stops (middle position)
        set_current_value_text(push_pull_item, ProcessSettingsModel::PUSH_PULL_VALUES[2]);

        // Add roll count setting
        roll_count_item = add_item(
            "Roll Count",
            ProcessSettingsModel::MAX_ROLL_COUNT,
            roll_count_change_callback,
            this);
        set_current_value_index(roll_count_item, 0); // Default to 1 roll
        update_roll_count_text(1);

        // Set enter callback for confirmation
        set_enter_callback(enter_callback, this);
    }

    ~SettingsView() {
        delete model;
    }

    const ProcessSettingsModel* get_model() const {
        return model;
    }

private:
    ProcessSettingsModel* model = nullptr;
    VariableItem* push_pull_item = nullptr;
    VariableItem* roll_count_item = nullptr;

    static void push_pull_change_callback(VariableItem* item) {
        auto view = static_cast<SettingsView*>(get_context(item));
        uint8_t index = get_current_value_index(item);
        view->model->push_pull_stops = index - 2; // Convert from 0-4 to -2 to +2
        set_current_value_text(item, ProcessSettingsModel::PUSH_PULL_VALUES[index]);
    }

    static void roll_count_change_callback(VariableItem* item) {
        auto view = static_cast<SettingsView*>(get_context(item));
        uint8_t index = get_current_value_index(item);
        view->model->roll_count = index + 1; // Convert from 0-based to 1-based
        view->update_roll_count_text(index + 1);
    }

    static void enter_callback(void* context, uint32_t index) {
        UNUSED(index);
        auto view = static_cast<SettingsView*>(context);
        view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::SettingsConfirmed));
    }

    void update_roll_count_text(uint8_t count) {
        char text[8];
        snprintf(text, sizeof(text), "%u", count);
        set_current_value_text(roll_count_item, text);
    }
}; 