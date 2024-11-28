#pragma once

#include <gui/view.h>
#include <gui/view_dispatcher.h>

namespace flipper {

struct ViewContext {
  void *instance;
  void *model;
};

template <typename T> class ModelHandle {
private:
  View *view;
  T *model;
  bool should_update;

public:
  ModelHandle(View *v, bool update = true) : view(v), should_update(update) {
    model = static_cast<T *>(view_get_model(v));
    FURI_LOG_D("ModelHandle", "ModelHandle created for view: %p", view);
  }

  ~ModelHandle() {
    FURI_LOG_D("ModelHandle", "ModelHandle destroyed for view: %p", view);
    view_commit_model(view, should_update);
  }

  T *operator->() { return model; }
  const T *operator->() const { return model; }
  T &operator*() { return *model; }
  const T &operator*() const { return *model; }
};

class ViewCpp {
public:
  ViewCpp() {}

  virtual void init() {
    view = view_alloc();
    view_allocate_model(view, ViewModelTypeLocking, sizeof(ViewContext));
    with_view_model_cpp(
        view, ViewContext *, model,
        {
          model->instance = this;
          model->model = nullptr;
        },
        true);
    view_set_context(view, this);
    view_set_draw_callback(view, &ViewCpp::drawWrapper);
    view_set_input_callback(view, &ViewCpp::inputWrapper);
    view_set_custom_callback(view, &ViewCpp::customWrapper);
    view_set_enter_callback(view, &ViewCpp::enterWrapper);
    view_set_exit_callback(view, &ViewCpp::exitWrapper);
  }

  virtual ~ViewCpp() {
    if (view) {
      view_free(view);
    }
  }

  // Non-copyable
  ViewCpp(const ViewCpp &) = delete;
  ViewCpp &operator=(const ViewCpp &) = delete;

  // Get raw View* for C API compatibility
  View *get_view() { return view; }

  // Set the view dispatcher that this view is added to
  void set_view_dispatcher(ViewDispatcher *dispatcher) {
    view_dispatcher = dispatcher;
  }

  // Send custom event through the view dispatcher
  void send_custom_event(uint32_t event) {
    if (view_dispatcher) {
      view_dispatcher_send_custom_event(view_dispatcher, event);
    }
  }

  virtual void enter() {}
  virtual void exit() {}
  virtual bool custom(uint32_t event) {
    UNUSED(event);
    return false;
  }
  // Virtual methods to be implemented by derived classes
  virtual void draw(Canvas *canvas, void *model) = 0;
  virtual bool input(InputEvent *event) {
    UNUSED(event);
    return false;
  }

  // Model access helpers
  template <typename T> void set_model(T *model) {
    // Update ViewContext with new model
    ViewContext *ctx = static_cast<ViewContext *>(view_get_model(view));
    if (ctx == nullptr) {
      return;
    }
    if (ctx->model != nullptr) {
      delete static_cast<T *>(ctx->model);
    }
    ctx->model = model;
    view_commit_model(view, true);
  }

  template <typename T> ModelHandle<T> get_model() {
    return ModelHandle<T>(view);
  }

protected:
  View *view = nullptr;
  ViewDispatcher *view_dispatcher = nullptr;

  // Static wrapper functions that route callbacks to instance methods
  static void drawWrapper(Canvas *canvas, void *model) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 39, 31, "Main Screen");
    ViewContext *ctx = static_cast<ViewContext *>(model);
    ViewCpp *instance = static_cast<ViewCpp *>(ctx->instance);
    instance->draw(canvas, ctx->model);
  }

  static bool inputWrapper(InputEvent *event, void *context) {
    ViewCpp *instance = static_cast<ViewCpp *>(context);
    return instance->input(event);
  }

  static bool customWrapper(uint32_t event, void *context) {
    ViewCpp *instance = static_cast<ViewCpp *>(context);
    return instance->custom(event);
  }

  static void enterWrapper(void *context) {
    ViewCpp *instance = static_cast<ViewCpp *>(context);
    instance->enter();
  }

  static void exitWrapper(void *context) {
    ViewCpp *instance = static_cast<ViewCpp *>(context);
    instance->exit();
  }
};

} // namespace flipper
