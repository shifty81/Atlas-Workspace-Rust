#pragma once
// NF::Editor — Linker settings v1: library references, output type, args generation
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class LsOutputType : uint8_t { Executable, SharedLib, StaticLib, Module };

inline const char* lsOutputTypeName(LsOutputType t) {
    switch(t){
        case LsOutputType::Executable: return "Executable";
        case LsOutputType::SharedLib:  return "SharedLib";
        case LsOutputType::StaticLib:  return "StaticLib";
        case LsOutputType::Module:     return "Module";
    }
    return "Unknown";
}

struct LsLibRef {
    uint32_t    id       = 0;
    std::string name;
    std::string path;
    bool        required = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

class LinkerSettingsV1 {
public:
    static constexpr size_t MAX_LIBS = 128;

    bool addLibRef(const LsLibRef& lib) {
        if (!lib.isValid()) return false;
        if (m_libs.size() >= MAX_LIBS) return false;
        for (const auto& l : m_libs) if (l.id == lib.id) return false;
        m_libs.push_back(lib);
        return true;
    }

    bool removeLibRef(uint32_t id) {
        for (auto it = m_libs.begin(); it != m_libs.end(); ++it) {
            if (it->id == id) { m_libs.erase(it); return true; }
        }
        return false;
    }

    void setOutputType(LsOutputType t) { m_outputType = t; }
    [[nodiscard]] LsOutputType getOutputType() const { return m_outputType; }

    void setOutputPath(const std::string& path) { m_outputPath = path; }
    [[nodiscard]] const std::string& getOutputPath() const { return m_outputPath; }

    [[nodiscard]] size_t libRefCount() const { return m_libs.size(); }

    [[nodiscard]] std::string generateArgs() const {
        std::string args;
        for (const auto& l : m_libs) {
            if (!l.path.empty()) args += "-L" + l.path + " ";
            args += "-l" + l.name + " ";
        }
        if (!m_outputPath.empty()) args += "-o " + m_outputPath;
        return args;
    }

private:
    std::vector<LsLibRef> m_libs;
    LsOutputType          m_outputType = LsOutputType::Executable;
    std::string           m_outputPath;
};

} // namespace NF
