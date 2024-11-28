#pragma once

#include "runtime_settings_dialog.hpp"
#include "dispatch_dialog.hpp"
#include "wait_confirmation_dialog.hpp"
#include "pause_dialog.hpp"

class DialogFactory {
public:
    enum class DialogType {
        RuntimeSettings,
        Dispatch,
        WaitConfirmation,
        Pause
    };

    static BaseDialog* create_dialog(DialogType type) {
        switch(type) {
            case DialogType::RuntimeSettings:
                return new RuntimeSettingsDialog();
            case DialogType::Dispatch:
                return new DispatchDialog();
            case DialogType::WaitConfirmation:
                return new WaitConfirmationDialog();
            case DialogType::Pause:
                return new PauseDialog();
            default:
                return nullptr;
        }
    }
}; 