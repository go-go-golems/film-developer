#pragma once

#include <gui/modules/file_browser.h>
#include "view_cpp.hpp"

namespace flipper {

class FileBrowserCpp : public ViewCpp {
public:
    FileBrowserCpp(FuriString* result_path)
        : result_path(result_path) {
    }

    ~FileBrowserCpp() {
        if(browser) {
            file_browser_free(browser);
            view = nullptr;
        }
    }

    void init() override {
        browser = file_browser_alloc(result_path);
        view = file_browser_get_view(browser);
    }

    void configure(
        const char* extension,
        const char* base_path,
        bool skip_assets,
        bool hide_dot_files,
        const Icon* file_icon,
        bool hide_ext) {
        file_browser_configure(
            browser, extension, base_path, skip_assets, hide_dot_files, file_icon, hide_ext);
    }

    void start(FuriString* path) {
        file_browser_start(browser, path);
    }

    void stop() {
        file_browser_stop(browser);
    }

    void set_callback(FileBrowserCallback callback, void* context) {
        file_browser_set_callback(browser, callback, context);
    }

    void set_item_callback(FileBrowserLoadItemCallback callback, void* context) {
        file_browser_set_item_callback(browser, callback, context);
    }

protected:
    // These are handled by the file browser internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

    FileBrowser* browser = nullptr;
    FuriString* result_path = nullptr;
};

} // namespace flipper
