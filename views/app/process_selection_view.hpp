#pragma once

#include "../common/submenu_cpp.hpp"
#include "../../agitation/process_interpreter_interface.hpp"
#include "../../film_developer_events.hpp"

class ProcessSelectionView : public flipper::SubMenuCpp {
public:
    ProcessSelectionView(ProcessInterpreterInterface* interpreter)
        : interpreter(interpreter) {
    }

    void init() override {
        SubMenuCpp::init();
        set_header("Select Process");

        // Clear existing process names
        process_count = 0;

        // Add process options using the interpreter
        for(size_t i = 0; i < interpreter->getProcessCount() && i < MAX_PROCESSES; ++i) {
            if(interpreter->getProcessName(i, process_names[i], sizeof(process_names[i]))) {
                add_item(process_names[i], i, process_callback, this);
                process_count++;
            }
        }
    }

    const char* get_selected_process() const {
        size_t index = get_selected_item();
        if(index < process_count) {
            return process_names[index];
        }
        return nullptr;
    }

private:
    static constexpr size_t MAX_PROCESSES = 16;
    static constexpr size_t MAX_NAME_LENGTH = 32;

    ProcessInterpreterInterface* interpreter;
    char process_names[MAX_PROCESSES][MAX_NAME_LENGTH];
    size_t process_count = 0;

    static void process_callback(void* context, uint32_t index) {
        auto view = static_cast<ProcessSelectionView*>(context);
        if(index < view->interpreter->getProcessCount()) {
            view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessSelected));
        }
    }
};
