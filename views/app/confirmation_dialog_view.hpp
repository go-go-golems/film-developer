#pragma once

#include "../common/dialog_ex_cpp.hpp"
#include "../../film_developer_events.hpp"

class ConfirmationDialogView : public flipper::DialogExCpp {
public:
    enum class DialogType {
        ProcessStart,
        ProcessAbort,
        StepComplete,
        StepRestart,
        StepSkip
    };

    void init() override {
        DialogExCpp::init();
        reset();
        configure_for_start();
    }

    void show_dialog(DialogType type) {
        reset();
        current_type = type;
        switch(type) {
        case DialogType::ProcessStart:
            configure_for_start();
            break;
        case DialogType::ProcessAbort:
            configure_for_abort();
            break;
        case DialogType::StepComplete:
            configure_for_step();
            break;
        case DialogType::StepRestart:
            configure_for_restart();
            break;
        case DialogType::StepSkip:
            configure_for_skip();
            break;
        }
    }

private:
    DialogType current_type{DialogType::ProcessStart};

    void configure_for_start() {
        set_header("Start Process", 64, 10, AlignCenter, AlignCenter);
        set_text(
            "Begin development process?\nSettings will be locked.",
            64,
            32,
            AlignCenter,
            AlignCenter);
        set_left_button_text("Cancel");
        set_right_button_text("Start");
        set_result_callback(dialog_callback);
    }

    void configure_for_abort() {
        set_header("Abort Process", 64, 10, AlignCenter, AlignCenter);
        set_text(
            "Stop current development?\nProgress will be lost.", 64, 32, AlignCenter, AlignCenter);
        set_left_button_text("Continue");
        set_right_button_text("Stop");
        set_result_callback(dialog_callback);
    }

    void configure_for_step() {
        set_header("Step Complete", 64, 10, AlignCenter, AlignCenter);
        set_text(
            "Current step finished.\nProceed to next step?", 64, 32, AlignCenter, AlignCenter);
        set_left_button_text("Wait");
        set_right_button_text("Next");
        set_result_callback(dialog_callback);
    }

    void configure_for_restart() {
        set_header("Restart Step", 64, 10, AlignCenter, AlignCenter);
        set_text("Restart current step?\nTimer will reset.", 64, 32, AlignCenter, AlignCenter);
        set_left_button_text("Cancel");
        set_right_button_text("Restart");
        set_result_callback(dialog_callback);
    }

    void configure_for_skip() {
        set_header("Skip Step", 64, 10, AlignCenter, AlignCenter);
        set_text("Skip current step?\nThis may affect results.", 64, 32, AlignCenter, AlignCenter);
        set_left_button_text("Cancel");
        set_right_button_text("Skip");
        set_result_callback(dialog_callback);
    }

    static void dialog_callback(DialogExResult result, void* context) {
        auto view = static_cast<ConfirmationDialogView*>(context);
        if(result == DialogExResultRight) {
            switch(view->current_type) {
            case DialogType::ProcessStart:
                view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::StartProcess));
                break;
            case DialogType::ProcessAbort:
                view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::ProcessAborted));
                break;
            case DialogType::StepComplete:
                view->send_custom_event(
                    static_cast<uint32_t>(FilmDeveloperEvent::UserActionConfirmed));
                break;
            case DialogType::StepRestart:
            case DialogType::StepSkip:
                view->send_custom_event(
                    static_cast<uint32_t>(FilmDeveloperEvent::DialogConfirmed));
                break;
            }
        } else {
            view->send_custom_event(static_cast<uint32_t>(FilmDeveloperEvent::DialogDismissed));
        }
    }
};
