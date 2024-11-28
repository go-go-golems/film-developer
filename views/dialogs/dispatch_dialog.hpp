#pragma once

#include "base_dialog.hpp"

class DispatchDialog : public BaseDialog {
protected:
    void configure(const char* custom_message = nullptr) override {
        UNUSED(custom_message);
        
        set_header("Process Control", 64, 10, AlignCenter, AlignCenter);
        set_text("↑ Pause/Resume\n"
                 "↓ Runtime Settings\n"
                 "← Restart Step\n"
                 "→ Skip Step\n"
                 "OK to exit", 
                 64, 32, AlignCenter, AlignCenter);
        
        set_left_button_text("Back");
        set_right_button_text("Select");
        
        set_result_callback(dialog_callback);
    }

private:
    static void dialog_callback(DialogExResult result, void* context) {
        auto dialog = static_cast<DispatchDialog*>(context);
        if(result == DialogExResultLeft) {
            dialog->send_dialog_event(FilmDeveloperEvent::DialogDismissed);
        }
    }
}; 