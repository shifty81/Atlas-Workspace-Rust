#include "NF/Core/Core.h"

namespace NF {

// Core module initialization
static bool s_coreInitialized = false;

void coreInit() {
    if (s_coreInitialized) return;
    NF_LOG_INFO("Core", "Atlas Engine Core initialized");
    NF_LOG_INFO("Core", std::string("Version: ") + NF_VERSION_STRING);
    s_coreInitialized = true;
}

void coreShutdown() {
    if (!s_coreInitialized) return;
    NF_LOG_INFO("Core", "Atlas Engine Core shutdown");
    s_coreInitialized = false;
}

bool isCoreInitialized() {
    return s_coreInitialized;
}

} // namespace NF
