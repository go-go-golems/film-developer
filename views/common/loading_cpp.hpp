#pragma once

#include <gui/modules/loading.h>
#include "view_cpp.hpp"

namespace flipper {

class LoadingCpp : public ViewCpp {
public:
    LoadingCpp() {
    }

    ~LoadingCpp() {
        if(loading) {
            loading_free(loading);
            view = nullptr;
        }
    }

    void init() override {
        loading = loading_alloc();
        view = loading_get_view(loading);
    }

protected:
    // These are handled by the loading internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }
    bool input(InputEvent* event) override {
        UNUSED(event);
        return false;
    }

private:
    Loading* loading = nullptr;
};

} // namespace flipper 