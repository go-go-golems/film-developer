#pragma once

#include "../../film_developer_events.hpp"
#include "../common/submenu_cpp.hpp"

class DispatchMenuView : public flipper::SubMenuCpp {
public:
  void init() override {
    SubMenuCpp::init();
    set_header("Process Control");

    add_item("Pause/Resume", 0, process_callback, this);
    add_item("Runtime Settings", 1, process_callback, this);
    add_item("Restart Step", 2, process_callback, this);
    add_item("Skip Step", 3, process_callback, this);
  }

private:
  static void process_callback(void *context, uint32_t index) {
    auto view = static_cast<DispatchMenuView *>(context);

    switch (index) {
    case 0: // Pause/Resume
      view->send_custom_event(
          static_cast<uint32_t>(FilmDeveloperEvent::PauseRequested));
      break;
    case 1: // Runtime Settings
      view->send_custom_event(
          static_cast<uint32_t>(FilmDeveloperEvent::EnterRuntimeSettings));
      break;
    case 2: // Restart Step
      view->send_custom_event(
          static_cast<uint32_t>(FilmDeveloperEvent::RestartRequested));
      break;
    case 3: // Skip Step
      view->send_custom_event(
          static_cast<uint32_t>(FilmDeveloperEvent::SkipRequested));
      break;
    }
  }
};