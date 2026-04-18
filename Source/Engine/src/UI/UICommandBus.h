#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <map>

namespace atlas::ui {

enum class UICommandType : uint8_t {
    ButtonPress,
    ButtonRelease,
    TextInput,
    SliderChange,
    ListSelect,
    FocusChange,
    VisibilityToggle,
    DockSplit,
    DockMerge,
    DockResize,
    Custom
};

struct UICommand {
    UICommandType type = UICommandType::Custom;
    uint32_t targetWidgetId = 0;
    uint64_t tick = 0;
    float valueFloat = 0.0f;
    std::string valueString;
};

class UICommandBus {
public:
    void Enqueue(const UICommand& cmd);
    void Enqueue(UICommand&& cmd);

    void Drain(std::vector<UICommand>& out);

    size_t PendingCount() const;

    void Clear();

    using Handler = std::function<void(const UICommand&)>;
    void RegisterHandler(UICommandType type, Handler handler);

    void Dispatch();

private:
    mutable std::mutex m_mutex;
    std::vector<UICommand> m_pending;
    std::map<uint8_t, std::vector<Handler>> m_handlers;
};

} // namespace atlas::ui
