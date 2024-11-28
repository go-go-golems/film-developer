#pragma once

#include "../common/dialog_ex_cpp.hpp"
#include "../../film_developer_events.hpp"

class BaseDialog : public flipper::DialogExCpp {
public:
    virtual void init() override {
        DialogExCpp::init();
        configure();
    }

    virtual void show(const char* custom_message = nullptr) {
        configure(custom_message);
    }

protected:
    virtual void configure(const char* custom_message = nullptr) = 0;

    void send_dialog_event(FilmDeveloperEvent event) {
        send_custom_event(static_cast<uint32_t>(event));
    }
}; 