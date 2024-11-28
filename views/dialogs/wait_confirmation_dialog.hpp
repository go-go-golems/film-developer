#pragma once

#include "base_dialog.hpp"

class WaitConfirmationDialog : public BaseDialog {
protected:
    void configure(const char* custom_message = nullptr) override {
        set_header("Action Required", 64, 10, AlignCenter, AlignCenter);
        
        if(custom_message) {
            set_text(custom_message, 64, 32, AlignCenter, AlignCenter);
        } else {
            set_text("User action required.\nPress OK when ready.", 
                     64, 32, AlignCenter, AlignCenter);
        }
        
        set_left_button_text("Settings");
        set_right_button_text("Continue");
        
        set_result_callback(dialog_callback);
    }

private:
    static void dialog_callback(DialogExResult result, void* context) {
        auto dialog = static_cast<WaitConfirmationDialog*>(context);
        if(result == DialogExResultLeft) {
            dialog->send_dialog_event(FilmDeveloperEvent::EnterRuntimeSettings);
        } else if(result == DialogExResultRight) {
            dialog->send_dialog_event(FilmDeveloperEvent::UserActionConfirmed);
        }
    }
}; 