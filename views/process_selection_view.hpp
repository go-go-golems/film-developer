#pragma once

#include "submenu_cpp.hpp"
#include "../agitation_processes.hpp"
#include "../film_developer_events.hpp"

class ProcessSelectionView : public flipper::SubMenuCpp {
public:
    void init() override {
        SubMenuCpp::init();
        set_header("Select Process");

        // Add process options
        for(size_t i = 0; i < PROCESS_COUNT; ++i) {
            add_item(available_processes[i]->process_name, i, process_callback, this);
        }
    }

    const AgitationProcessStatic* get_selected_process() const {
        return available_processes[get_selected_item()];
    }

private:
    static constexpr const AgitationProcessStatic* available_processes[] = {
        &C41_FULL_PROCESS_STATIC,
        &BW_STANDARD_DEV_STATIC,
        &STAND_DEV_STATIC,
        &CONTINUOUS_GENTLE_STATIC};
    static constexpr size_t PROCESS_COUNT =
        sizeof(available_processes) / sizeof(available_processes[0]);

    static void process_callback(void* context, uint32_t index) {
        auto view = static_cast<ProcessSelectionView*>(context);
        if(index < PROCESS_COUNT) {
            view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessSelected));
        }
    }
};
