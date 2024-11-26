# View Classes Reference

This document provides a reference for all view classes in the Example C++ View application.

## Base Class: ViewCpp

All view classes inherit from `flipper::ViewCpp`, which provides:

- `void init()` - Initialize the view
- `View* get_view()` - Get the underlying C view
- `void set_view_dispatcher(ViewDispatcher* dispatcher)`
- Protected methods: `draw(Canvas*, void*)`, `input(InputEvent*)`

Example usage:

```cpp
class MyView : public flipper::ViewCpp {
public:
    void init() override {
        ViewCpp::init();
        // Initialize your view
    }

protected:
    void draw(Canvas* canvas, void* model) override {
        canvas_clear(canvas);
        canvas_draw_str(canvas, 39, 31, "My View");
    }

    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort && event->key == InputKeyOk) {
            send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
            return true;
        }
        return false;
    }
};
```

## Custom Events Communication

Views can communicate with the main application and other views through custom events. This is achieved through the ViewDispatcher's event system. Here's how it works:

### 1. Sending Custom Events from Views

Views can send custom events using the `send_custom_event()` method inherited from `ViewCpp`:

```cpp
// Inside your view class
void my_action() {
    send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
}
```

### 2. Handling Custom Events in the Main Application

The main application sets up a custom event callback through the ViewDispatcher:

```cpp
class MyApp {
    void init() {
        view_dispatcher = view_dispatcher_alloc();
        view_dispatcher_set_event_callback_context(view_dispatcher, this);
        view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);
    }

    static bool custom_callback(void* context, uint32_t event) {
        auto* app = static_cast<MyApp*>(context);
        if(event == static_cast<uint32_t>(CustomEvent::ToggleScene)) {
            // Handle the event
            return true;  // Event was handled
        }
        return false;  // Event was not handled
    }
};
```

### 3. Defining Custom Events

Define your custom events using an enum class for type safety:

```cpp
enum class CustomEvent : uint32_t {
    ToggleScene = 0,
    SaveSettings = 1,
    LoadData = 2,
    // Add more events as needed
};
```

### 4. Event Flow

1. View triggers an action (e.g., button press)
2. View sends custom event via `send_custom_event()`
3. ViewDispatcher receives the event
4. If the current view doesn't handle the event, it's passed to the main application's custom event callback
5. The callback processes the event and returns true if handled

### Best Practices

1. Use enum class for type-safe event definitions
2. Handle events at the appropriate level (view or application)
3. Return true from callbacks when events are handled
4. Keep event handling logic clean and focused
5. Document the purpose and expected handling of each custom event

## Working with Models

The `ViewCpp` base class provides a powerful model management system that allows views to maintain state. Models are managed through smart handles that automatically handle locking and unlocking of the view model.

### Model Basics

1. Define your model structure:
```cpp
struct ViewModel {
    int counter{0};
    const char* message{"Hello World"};
};
```

2. Initialize the model in your view:
```cpp
void init() override {
    ViewCpp::init();
    ViewModel* initial_model = new ViewModel();
    set_model(initial_model);
}
```

3. Access the model in your view methods:
```cpp
void draw(Canvas* canvas, void* model) override {
    auto m = get_model<ViewModel>();
    canvas_draw_str(canvas, 33, 41, m->message);
    
    char str[16];
    snprintf(str, sizeof(str), "%d", m->counter);
    canvas_draw_str(canvas, 78, 41, str);
}
```

### Model Handling Examples

#### Basic Counter View
```cpp
class CounterView : public flipper::ViewCpp {
public:
    void init() override {
        ViewCpp::init();
        auto* initial_model = new ViewModel();
        initial_model->counter = 0;
        set_model(initial_model);
    }

protected:
    void draw(Canvas* canvas, void* model) override {
        auto m = get_model<ViewModel>();
        canvas_draw_str(canvas, 33, 41, m->message);
        
        char str[16];
        snprintf(str, sizeof(str), "%d", m->counter);
        canvas_draw_str(canvas, 78, 41, str);
    }
};
```

## View Classes

### ButtonPanelCpp

Grid-based button panel with icons and labels.

Interface:

```cpp
void init()
void reserve(uint8_t columns, uint8_t rows)
void add_label(uint8_t x, uint8_t y, Font font, const char* text)
void add_item(uint8_t index, uint8_t column, uint8_t row, uint8_t x, uint8_t y,
              const Icon* icon_idle, const Icon* icon_active,
              ButtonCallback callback, void* context)
```

Example usage:

```cpp
class MyButtonPanel : public flipper::ButtonPanelCpp {
public:
    void init() override {
        ButtonPanelCpp::init();
        reserve(2, 2);  // 2x2 grid

        add_label(5, 8, FontPrimary, "Button Panel");
        add_item(0, 0, 0, 8, 32,
                &I_ButtonLeft_4x7, &I_ButtonLeftPressed_4x7,
                button_callback, this);
    }

private:
    static void button_callback(void* context, uint32_t index) {
        auto* instance = static_cast<MyButtonPanel*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
```

### ByteInputCpp

Hexadecimal byte input interface.

Interface:

```cpp
void init()
void set_header_text(const char* text)
void set_result_callback(ByteInputCallback callback, ByteChangedCallback changed_cb,
                        void* context, uint8_t* bytes, uint8_t bytes_count)
```

Example usage:

```cpp
class MyByteInput : public flipper::ByteInputCpp {
public:
    void init() override {
        ByteInputCpp::init();
        set_header_text("Enter Bytes");

        bytes[0] = 0xAB;
        set_result_callback(byte_input_callback, byte_changed_callback,
                          this, bytes, 1);
    }

private:
    uint8_t bytes[1];
    static void byte_input_callback(void* context) {
        auto* instance = static_cast<MyByteInput*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
    static void byte_changed_callback(void* context) {
        // Handle byte changes
    }
};
```

### ButtonMenuCpp

Menu with different types of buttons.

Interface:

```cpp
void init()
void set_header(const char* header)
void add_item(const char* label, uint32_t index, ButtonMenuItemCallback callback,
              ButtonMenuItemType type, void* context)
```

Example usage:

```cpp
class MyButtonMenu : public flipper::ButtonMenuCpp {
public:
    void init() override {
        ButtonMenuCpp::init();
        set_header("Menu Options");

        add_item("Option 1", 0, button_callback, ButtonMenuItemTypeCommon, this);
        add_item("Exit", 1, button_callback, ButtonMenuItemTypeControl, this);
    }

private:
    static void button_callback(void* context, int32_t index, InputType type) {
        if(index == 1 && type == InputTypeShort) {
            auto* instance = static_cast<MyButtonMenu*>(context);
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
```

### DialogExCpp

Extended dialog with customizable buttons.

Interface:

```cpp
void init()
void set_header(const char* text, uint8_t x, uint8_t y, Align h, Align v)
void set_text(const char* text, uint8_t x, uint8_t y, Align h, Align v)
void set_left_button_text(const char* text)
void set_center_button_text(const char* text)
void set_right_button_text(const char* text)
void set_result_callback(DialogExResultCallback callback)
```

Example usage:

```cpp
class MyDialog : public flipper::DialogExCpp {
public:
    void init() override {
        DialogExCpp::init();
        set_header("Confirm", 64, 10, AlignCenter, AlignCenter);
        set_text("Are you sure?", 64, 32, AlignCenter, AlignCenter);

        set_left_button_text("No");
        set_right_button_text("Yes");
        set_result_callback(dialog_callback);
    }

private:
    static void dialog_callback(DialogExResult result, void* context) {
        auto* instance = static_cast<MyDialog*>(context);
        if(result == DialogExResultRight) {
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
```

### FileBrowserCpp

File system browser with customizable filters.

Interface:

```cpp
void init()
void configure(const char* extension, const char* base_path, bool skip_assets,
               bool hide_dot_files, const Icon* file_icon, bool hide_ext)
void set_callback(FileBrowserCallback callback, void* context)
void start(FuriString* result_path)
```

Example usage:

```cpp
class MyFileBrowser : public flipper::FileBrowserCpp {
public:
    MyFileBrowser() : FileBrowserCpp(furi_string_alloc()) {}

    ~MyFileBrowser() {
        if(result_path) furi_string_free(result_path);
    }

    void init() override {
        FileBrowserCpp::init();
        configure(".txt", "/", false, true, &I_icon_10px, true);
        set_callback(file_browser_callback, this);
        start(result_path);
    }

private:
    static void file_browser_callback(void* context) {
        auto* instance = static_cast<MyFileBrowser*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
```

### LoadingCpp

Simple loading animation view.

Interface:

```cpp
void init()
```

Example usage:

```cpp
class MyLoading : public flipper::LoadingCpp {
public:
    void init() override {
        LoadingCpp::init();
        // Loading animation starts automatically
    }
};
```

### NumberInputCpp

Numeric value input interface.

Interface:

```cpp
void init()
void set_header_text(const char* text)
void set_result_callback(NumberInputCallback callback, void* context,
                        int32_t start_value, int32_t min_value, int32_t max_value)
```

Example usage:

```cpp
class MyNumberInput : public flipper::NumberInputCpp {
public:
    void init() override {
        NumberInputCpp::init();
        set_header_text("Enter Value");
        set_result_callback(number_callback, this, 50, 0, 100);
    }

private:
    static void number_callback(void* context, int32_t number) {
        auto* instance = static_cast<MyNumberInput*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
```

### PopupCpp

Popup dialog with optional timeout.

Interface:

```cpp
void init()
void set_header(const char* text, uint8_t x, uint8_t y, Align h, Align v)
void set_text(const char* text, uint8_t x, uint8_t y, Align h, Align v)
void set_icon(uint8_t x, uint8_t y, const Icon* icon)
void set_callback(PopupCallback callback)
void set_context(void* context)
void set_timeout(uint32_t timeout_ms)
void enable_timeout()
```

Example usage:

```cpp
class MyPopup : public flipper::PopupCpp {
public:
    void init() override {
        PopupCpp::init();
        set_header("Notice", 64, 10, AlignCenter, AlignCenter);
        set_text("Auto-closing...", 64, 32, AlignCenter, AlignCenter);

        set_callback(popup_callback);
        set_context(this);
        set_timeout(3000);
        enable_timeout();
    }

private:
    static void popup_callback(void* context) {
        auto* instance = static_cast<MyPopup*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
```

### SubMenuCpp

Scrollable menu with selectable items.

Interface:

```cpp
void init()
void add_item(const char* label, uint32_t index, SubmenuItemCallback callback, void* context)
void set_selected_item(uint32_t index)
uint32_t get_selected_item()
void set_header(const char* header)
void reset()
```

Example usage:

```cpp
class MySubmenu : public flipper::SubMenuCpp {
public:
    void init() override {
        SubMenuCpp::init();
        set_header("Select Option");

        add_item("Option 1", 0, menu_callback, this);
        add_item("Option 2", 1, menu_callback, this);
        add_item("Exit", 2, menu_callback, this);
    }

private:
    static void menu_callback(void* context, uint32_t index) {
        if(index == 2) {
            auto* instance = static_cast<MySubmenu*>(context);
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
```

### TextBoxCpp

Scrollable text display.

Interface:

```cpp
void init()
void set_text(const char* text)
void set_font(TextBoxFont font)
void set_focus(TextBoxFocus focus)
```

Example usage:

```cpp
class MyTextBox : public flipper::TextBoxCpp {
public:
    void init() override {
        TextBoxCpp::init();
        set_text("This is a scrollable text box.\n"
                 "It can display multiple lines\n"
                 "of text with automatic wrapping.");
        set_font(TextBoxFontText);
        set_focus(TextBoxFocusStart);
    }
};
```

### TextInputCpp

Text input interface with customizable parameters.

Interface:

```cpp
void init()
void set_header_text(const char* text)
void set_result_callback(TextInputCallback callback, void* context,
                        char* text_buffer, size_t text_buffer_size, bool clear_default)
void set_minimum_length(size_t minimum_length)
```

Example usage:

```cpp
class MyTextInput : public flipper::TextInputCpp {
public:
    void init() override {
        TextInputCpp::init();
        set_header_text("Enter Text");
        set_result_callback(text_callback, this,
                          text_buffer, sizeof(text_buffer), true);
        set_minimum_length(1);
    }

private:
    char text_buffer[32] = "";
    static void text_callback(void* context) {
        auto* instance = static_cast<MyTextInput*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
```

### WidgetCpp

Flexible widget container for complex layouts.

Interface:

```cpp
void init()
void reset()
void add_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius)
void add_string(uint8_t x, uint8_t y, Align h, Align v, Font font, const char* text)
void add_string_multiline(uint8_t x, uint8_t y, Align h, Align v, Font font, const char* text)
void add_icon(uint8_t x, uint8_t y, const Icon* icon)
void add_button(GuiButtonType type, const char* text, ButtonCallback callback, void* context)
```

Example usage:

```cpp
class MyWidget : public flipper::WidgetCpp {
public:
    void init() override {
        WidgetCpp::init();
        reset();

        add_frame(0, 0, 128, 64, 3);
        add_string(64, 8, AlignCenter, AlignTop, FontPrimary, "Widget Demo");
        add_string_multiline(5, 20, AlignLeft, AlignTop, FontSecondary,
                           "This is a\nmultiline text");
        add_button(GuiButtonTypeRight, "Next", button_callback, this);
    }

private:
    static void button_callback(GuiButtonType result, InputType type, void* context) {
        if(type == InputTypeShort && result == GuiButtonTypeRight) {
            auto* instance = static_cast<MyWidget*>(context);
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
```

### VariableItemListCpp

Scrollable list of items with variable values.

Interface:

```cpp
void init()
VariableItem* add_item(const char* label, uint8_t values_count,
                      VariableItemChangeCallback change_callback, void* context)
void set_enter_callback(VariableItemListEnterCallback callback, void* context)
void set_selected_item(uint8_t index)
uint8_t get_selected_item_index()
void reset()

// Static helper methods
static void set_current_value_index(VariableItem* item, uint8_t current_value_index)
static void set_values_count(VariableItem* item, uint8_t values_count)
static void set_current_value_text(VariableItem* item, const char* current_value_text)
static uint8_t get_current_value_index(VariableItem* item)
static void* get_context(VariableItem* item)
```

Example usage:

```cpp
class VariableListTestView : public flipper::VariableItemListCpp {
public:
    void init() override {
        VariableItemListCpp::init();

        // Add an item with 5 possible values (0-4)
        brightness_item = add_item("Brightness", 5, brightness_change_callback, this);
        set_current_value_index(brightness_item, 2);  // Set to middle value
        set_current_value_text(brightness_item, "60%");

        // Add navigation callback
        set_enter_callback(enter_callback, this);
    }

private:
    VariableItem* brightness_item;

    static void brightness_change_callback(VariableItem* item) {
        uint8_t index = get_current_value_index(item);
        char str[8];
        snprintf(str, sizeof(str), "%d%%", (index + 1) * 20);
        set_current_value_text(item, str);
    }

    static void enter_callback(void* context, uint32_t index) {
        auto* instance = static_cast<VariableListTestView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
```

Key features:

- Add items with custom labels and value ranges
- Set initial values and display text
- Update display text dynamically in response to value changes
- Navigate between items with up/down buttons
- Adjust values with left/right buttons
- Handle enter button press with custom callback
- Reset list to clear all items

Common patterns:

1. Initialize base class in `init()`
2. Add items with labels and callbacks
3. Set initial values and text
4. Update text in change callbacks
5. Use enter callback for navigation
