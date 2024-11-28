#pragma once

#include "base_dialog.hpp"

class PauseDialog : public BaseDialog {
protected:
    void configure(const char* custom_message = nullptr) override {
        UNUSED(custom_message);
        
        set_header("Process Paused", 64, 10, AlignCenter, AlignCenter);
        set_text("Process is paused.\n"
                 "↑ Runtime Settings\n"
                 "↓ Exit Process\n"
                 "OK to resume", 
                 64, 32, AlignCenter, AlignCenter);
        
        set_left_button_text("Settings");
        set_right_button_text("Resume");
        
        set_result_callback(dialog_callback);
    }

private:
    static void dialog_callback(DialogExResult result, void* context) {
        auto dialog = static_cast<PauseDialog*>(context);
        if(result == DialogExResultLeft) {
            dialog->send_dialog_event(FilmDeveloperEvent::EnterRuntimeSettings);
        } else if(result == DialogExResultRight) {
            dialog->send_dialog_event(FilmDeveloperEvent::ResumeRequested);
        }
    }
}; 