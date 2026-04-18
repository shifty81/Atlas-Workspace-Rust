#include "TextRenderer.h"

// Base class and NullTextRenderer are fully defined in the header.
// This translation unit exists so the library always contains the
// vtable for TextRenderer, preventing linker issues when backends
// are conditionally compiled.

namespace atlas::ui {

// Weak-symbol anchor (intentionally empty).

} // namespace atlas::ui
