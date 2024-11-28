#pragma once

#include "base_dialog.hpp"

class RuntimeSettingsDialog : public BaseDialog {
protected:
    void configure(const char* custom_message = nullptr) override {
        UNUSED(custom_message);
        
        set_header("Runtime Settings", 64, 10, AlignCenter, AlignCenter);
        set_text("Adjust current step:\n"
                 "↑ Duration (+30s)\n"
                 "↓ Duration (-30s)\n"
                 "← Back to process\n"
                 "→ Skip step", 
                 64, 32, AlignCenter, AlignCenter);
        
        set_left_button_text("Back");
        set_right_button_text("Apply");
        
        set_result_callback(dialog_callback);
    }

private:
    static void dialog_callback(DialogExResult result, void* context) {
        auto dialog = static_cast<RuntimeSettingsDialog*>(context);
        if(result == DialogExResultLeft) {
            dialog->send_dialog_event(FilmDeveloperEvent::ExitRuntimeSettings);
        } else if(result == DialogExResultRight) {
            dialog->send_dialog_event(FilmDeveloperEvent::StepDurationChanged);
        }
    }
}; 