#pragma once

#include "../../film_developer_events.hpp"
#include "../../models/main_view_model.hpp"
#include "../common/variable_item_list_cpp.hpp"

class SettingsView : public flipper::VariableItemListCpp {
public:
    SettingsView(ProtectedModel& model)
        : flipper::VariableItemListCpp()
        , model(model) {
    }

    void init() override {
        flipper::VariableItemListCpp::init();

        auto m = model.lock();

        // Add push/pull setting
        push_pull_item =
            add_item("Push/Pull", Model::PUSH_PULL_COUNT, push_pull_change_callback, this);
        set_current_value_index(push_pull_item,
                                2); // Default to 0 stops (middle position)
        set_current_value_text(push_pull_item, Model::PUSH_PULL_VALUES[2]);

        // Add roll count setting
        roll_count_item =
            add_item("Roll Count", Model::MAX_ROLL_COUNT, roll_count_change_callback, this);
        set_current_value_index(roll_count_item, 0); // Default to 1 roll
        update_roll_count_text(1);

        add_item("Confirm", 0, nullptr, nullptr);
        add_item("Back", 0, nullptr, nullptr);

        // Set enter callback for confirmation
        set_enter_callback(enter_callback, this);
    }

private:
    ProtectedModel& model;
    VariableItem* push_pull_item = nullptr;
    VariableItem* roll_count_item = nullptr;

    static void push_pull_change_callback(VariableItem* item) {
        auto view = static_cast<SettingsView*>(get_context(item));
        uint8_t index = get_current_value_index(item);
        auto m = view->model.lock();
        m->push_pull_stops = index - 1; // Convert from 0-4 to -1 to +3
        if(m->process_interpreter) {
            m->process_interpreter->setProcessPushPull(m->push_pull_stops);
        }
        set_current_value_text(item, Model::PUSH_PULL_VALUES[index]);
    }

    static void roll_count_change_callback(VariableItem* item) {
        auto view = static_cast<SettingsView*>(get_context(item));
        uint8_t index = get_current_value_index(item);
        auto m = view->model.lock();
        m->roll_count = index + 1; // Convert from 0-based to 1-based
        if(m->process_interpreter) {
            m->process_interpreter->setRolls(m->roll_count);
        }
        view->update_roll_count_text(index + 1);
    }

    static void enter_callback(void* context, uint32_t index) {
        if(index == 2) {
            auto view = static_cast<SettingsView*>(context);
            view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::SettingsConfirmed));
        } else if(index == 3) {
            auto view = static_cast<SettingsView*>(context);
            view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessAborted));
        }
    }

    void update_roll_count_text(uint8_t count) {
        char text[8];
        snprintf(text, sizeof(text), "%u", count);
        set_current_value_text(roll_count_item, text);
    }
};
