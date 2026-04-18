#include "NF/Pipeline/Pipeline.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>

namespace NF {

// ── JSON helpers (no external dependencies) ───────────────────────────────
// Simple helpers to build/parse the fixed-schema JSON used by ChangeEvent
// and Manifest.  Not a general-purpose parser.

namespace {

// Escape a string for embedding inside a JSON string value.
static std::string jsonEscape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out += c;
    }
    return out;
}

// Unescape a JSON string value (already stripped of surrounding quotes).
static std::string jsonUnescape(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            ++i;
            switch (s[i]) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                default:   out += s[i]; break;
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

// Extract the value of a JSON string field: "key": "value"
// Returns empty string if not found.
static std::string jsonGetString(const std::string& json, std::string_view key) {
    std::string pattern = "\"";
    pattern += key;
    pattern += "\":";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) {
        // try with space
        pattern = "\"";
        pattern += key;
        pattern += "\": ";
        pos = json.find(pattern);
    }
    if (pos == std::string::npos) return {};
    pos += pattern.size();
    // skip whitespace
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;
    if (pos >= json.size() || json[pos] != '"') return {};
    ++pos; // skip opening quote
    auto end = pos;
    while (end < json.size() && json[end] != '"') {
        if (json[end] == '\\') ++end; // skip escaped char
        ++end;
    }
    return jsonUnescape(json.substr(pos, end - pos));
}

// Extract the value of a JSON integer field: "key": 12345
static int64_t jsonGetInt64(const std::string& json, std::string_view key) {
    std::string pattern = "\"";
    pattern += key;
    pattern += "\":";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) {
        pattern = "\"";
        pattern += key;
        pattern += "\": ";
        pos = json.find(pattern);
    }
    if (pos == std::string::npos) return 0;
    pos += pattern.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;
    if (pos >= json.size()) return 0;
    return std::stoll(json.substr(pos));
}

// Current time in milliseconds since epoch.
static int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// Last-write-time of a path as milliseconds since epoch.
static int64_t lastWriteMs(const std::filesystem::path& p) {
    std::error_code ec;
    auto lwt = std::filesystem::last_write_time(p, ec);
    if (ec) return 0;
    // Convert file_time_type to system_clock duration (C++20 clock_cast not
    // always available; use a portable workaround).
    auto dur = lwt.time_since_epoch();
    // file_time_type uses nanoseconds on most platforms; convert to ms.
    return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
}

} // anonymous namespace

// ── ChangeEvent ───────────────────────────────────────────────────────────

std::string ChangeEvent::toJson() const {
    std::string j = "{\n";
    j += "  \"tool\": \""       + jsonEscape(tool)                              + "\",\n";
    j += "  \"event_type\": \"" + jsonEscape(changeEventTypeName(eventType))    + "\",\n";
    j += "  \"path\": \""       + jsonEscape(path)                              + "\",\n";
    j += "  \"timestamp\": "    + std::to_string(timestamp)                     + ",\n";
    j += "  \"metadata\": \""   + jsonEscape(metadata)                          + "\"\n";
    j += "}";
    return j;
}

bool ChangeEvent::fromJson(const std::string& json, ChangeEvent& out) {
    if (json.empty()) return false;
    out.tool      = jsonGetString(json, "tool");
    out.eventType = changeEventTypeFromString(jsonGetString(json, "event_type"));
    out.path      = jsonGetString(json, "path");
    out.timestamp = jsonGetInt64(json, "timestamp");
    out.metadata  = jsonGetString(json, "metadata");
    return true;
}

bool ChangeEvent::writeToFile(const std::filesystem::path& changesDir) const {
    std::error_code ec;
    std::filesystem::create_directories(changesDir, ec);
    if (ec) return false;

    // Build a filename that is unique and human-readable.
    std::string fname = tool.empty() ? "tool" : tool;
    fname += "_";
    fname += changeEventTypeName(eventType);
    fname += "_";
    fname += std::to_string(timestamp > 0 ? timestamp : nowMs());
    fname += ".change.json";

    // Sanitize: replace spaces with underscores.
    for (char& c : fname) {
        if (c == ' ' || c == '/' || c == '\\') c = '_';
    }

    auto filePath = changesDir / fname;
    std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return false;
    ofs << toJson();
    return ofs.good();
}

bool ChangeEvent::readFromFile(const std::filesystem::path& filePath,
                               ChangeEvent& out) {
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());
    return fromJson(content, out);
}

// ── Manifest — GUID helpers ───────────────────────────────────────────────

std::string Manifest::generateGuid() {
    // Simple monotonic GUID: timestamp-ms + counter, formatted as a UUID-like
    // string.  Not cryptographically random, but unique within a session.
    static std::atomic<uint32_t> counter{0};
    int64_t ms  = nowMs();
    uint32_t seq = counter.fetch_add(1);

    char buf[64];
    // Format: xxxxxxxx-xxxx-4xxx-xxxx-xxxxxxxxxxxx (UUID v4 shape, not true random)
    unsigned int a  = static_cast<unsigned int>((ms >> 32) & 0xFFFFFFFFu);
    unsigned int b  = static_cast<unsigned int>(ms & 0xFFFFu);
    unsigned int c  = 0x4000u | (static_cast<unsigned int>(ms >> 16) & 0x0FFFu);
    unsigned int d  = 0x8000u | (seq & 0x3FFFu);
    unsigned int e1 = static_cast<unsigned int>((ms ^ seq) & 0xFFFFFFu);
    unsigned int e2 = static_cast<unsigned int>((ms * 0x9e3779b9u) & 0xFFFFFFu);
    std::snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%04x-%06x%06x",
                  a, b, c, d, e1, e2);
    return buf;
}

void Manifest::rebuildIndex() {
    m_guidIndex.clear();
    m_pathIndex.clear();
    for (size_t i = 0; i < m_records.size(); ++i) {
        m_guidIndex[m_records[i].guid] = i;
        if (!m_records[i].path.empty())
            m_pathIndex[m_records[i].path] = i;
    }
}

// ── Manifest — persistence ────────────────────────────────────────────────
// Format:
// {
//   "project": "MyGame",
//   "modules": ["Core","Engine"],
//   "assets": [
//     {"guid":"...","type":"mesh","path":"...","import_date":0,"checksum":""}
//   ]
// }

bool Manifest::load(const std::filesystem::path& manifestPath) {
    std::ifstream ifs(manifestPath);
    if (!ifs.is_open()) return false;
    std::string json((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());

    projectName = jsonGetString(json, "project");

    // Parse modules array: "modules": ["A","B",...]
    {
        modules.clear();
        std::string arrKey = "\"modules\":";
        auto pos = json.find(arrKey);
        if (pos != std::string::npos) {
            pos += arrKey.size();
            while (pos < json.size() && json[pos] != '[') ++pos;
            auto end = json.find(']', pos);
            if (end != std::string::npos) {
                std::string arr = json.substr(pos + 1, end - pos - 1);
                size_t i = 0;
                while (i < arr.size()) {
                    auto q1 = arr.find('"', i);
                    if (q1 == std::string::npos) break;
                    auto q2 = arr.find('"', q1 + 1);
                    if (q2 == std::string::npos) break;
                    modules.push_back(jsonUnescape(arr.substr(q1 + 1, q2 - q1 - 1)));
                    i = q2 + 1;
                }
            }
        }
    }

    // Parse assets array: "assets": [ { ... }, ... ]
    {
        m_records.clear();
        std::string arrKey = "\"assets\":";
        auto pos = json.find(arrKey);
        if (pos != std::string::npos) {
            pos += arrKey.size();
            while (pos < json.size() && json[pos] != '[') ++pos;
            ++pos; // skip '['
            // Iterate object blocks
            while (pos < json.size()) {
                auto objStart = json.find('{', pos);
                if (objStart == std::string::npos) break;
                auto objEnd = json.find('}', objStart);
                if (objEnd == std::string::npos) break;
                std::string obj = json.substr(objStart, objEnd - objStart + 1);
                AssetRecord rec;
                rec.guid       = jsonGetString(obj, "guid");
                rec.type       = jsonGetString(obj, "type");
                rec.path       = jsonGetString(obj, "path");
                rec.importDate = jsonGetInt64(obj,  "import_date");
                rec.checksum   = jsonGetString(obj, "checksum");
                if (!rec.guid.empty())
                    m_records.push_back(std::move(rec));
                pos = objEnd + 1;
            }
        }
    }

    rebuildIndex();
    return true;
}

bool Manifest::save(const std::filesystem::path& manifestPath) const {
    std::error_code ec;
    std::filesystem::create_directories(manifestPath.parent_path(), ec);
    if (ec) return false;

    std::ofstream ofs(manifestPath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return false;

    ofs << "{\n";
    ofs << "  \"project\": \"" << jsonEscape(projectName) << "\",\n";
    ofs << "  \"modules\": [";
    for (size_t i = 0; i < modules.size(); ++i) {
        if (i) ofs << ", ";
        ofs << "\"" << jsonEscape(modules[i]) << "\"";
    }
    ofs << "],\n";
    ofs << "  \"assets\": [\n";
    for (size_t i = 0; i < m_records.size(); ++i) {
        const auto& r = m_records[i];
        ofs << "    {";
        ofs << "\"guid\":\"" << jsonEscape(r.guid) << "\",";
        ofs << "\"type\":\"" << jsonEscape(r.type) << "\",";
        ofs << "\"path\":\"" << jsonEscape(r.path) << "\",";
        ofs << "\"import_date\":" << r.importDate << ",";
        ofs << "\"checksum\":\"" << jsonEscape(r.checksum) << "\"";
        ofs << "}";
        if (i + 1 < m_records.size()) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ]\n}\n";
    return ofs.good();
}

std::string Manifest::registerAsset(AssetRecord record) {
    if (record.guid.empty())
        record.guid = generateGuid();

    // If a record with the same path already exists, update it in place.
    auto pathIt = m_pathIndex.find(record.path);
    if (!record.path.empty() && pathIt != m_pathIndex.end()) {
        size_t idx = pathIt->second;
        std::string oldGuid = m_records[idx].guid;
        m_guidIndex.erase(oldGuid);
        m_records[idx] = record;
        m_guidIndex[record.guid] = idx;
        m_pathIndex[record.path] = idx;
        return record.guid;
    }

    size_t idx = m_records.size();
    m_records.push_back(record);
    m_guidIndex[record.guid] = idx;
    if (!record.path.empty())
        m_pathIndex[record.path] = idx;
    return record.guid;
}

const AssetRecord* Manifest::findByGuid(const std::string& guid) const {
    auto it = m_guidIndex.find(guid);
    if (it == m_guidIndex.end()) return nullptr;
    return &m_records[it->second];
}

const AssetRecord* Manifest::findByPath(const std::string& path) const {
    auto it = m_pathIndex.find(path);
    if (it == m_pathIndex.end()) return nullptr;
    return &m_records[it->second];
}

bool Manifest::removeAsset(const std::string& guid) {
    auto it = m_guidIndex.find(guid);
    if (it == m_guidIndex.end()) return false;
    size_t idx = it->second;
    m_records.erase(m_records.begin() + static_cast<std::ptrdiff_t>(idx));
    rebuildIndex();
    return true;
}

// ── WatchLog ──────────────────────────────────────────────────────────────

WatchLog::WatchLog(std::filesystem::path logPath)
    : m_logPath(std::move(logPath)) {
    std::error_code ec;
    std::filesystem::create_directories(m_logPath.parent_path(), ec);
}

void WatchLog::append(const ChangeEvent& event) {
    appendLine(event.toJson());
}

void WatchLog::appendLine(std::string_view message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ofstream ofs(m_logPath, std::ios::out | std::ios::app);
    if (ofs.is_open()) {
        ofs << message << "\n";
    }
}

void WatchLog::flush() {
    // ofstream flushes on close; this is a no-op for the current
    // file-per-write implementation.  Kept as a public API hook for
    // future buffered implementations.
}

// ── PipelineWatcher ───────────────────────────────────────────────────────

PipelineWatcher::PipelineWatcher(std::filesystem::path watchDir)
    : m_watchDir(std::move(watchDir)) {}

PipelineWatcher::~PipelineWatcher() {
    stop();
}

void PipelineWatcher::subscribe(PipelineEventCallback cb) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callbacks.push_back(std::move(cb));
}

bool PipelineWatcher::start() {
    if (m_running.exchange(true)) return false; // already running
    m_thread = std::thread([this] { watchLoop(); });
    return true;
}

void PipelineWatcher::stop() {
    m_running.store(false);
    if (m_thread.joinable()) m_thread.join();
}

int PipelineWatcher::poll() {
    std::error_code ec;
    if (!std::filesystem::exists(m_watchDir, ec)) return 0;

    int count = 0;
    for (const auto& entry :
         std::filesystem::directory_iterator(m_watchDir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        const auto& p = entry.path();
        if (p.extension() != ".json") continue;
        auto fname = p.filename().string();
        // Only process .change.json files.
        if (fname.size() < 12 ||
            fname.substr(fname.size() - 12) != ".change.json") continue;

        int64_t mtime = lastWriteMs(p);
        {
            std::lock_guard<std::mutex> lock(m_seenMutex);
            auto it = m_seen.find(fname);
            if (it != m_seen.end() && it->second == mtime) continue;
            m_seen[fname] = mtime;
        }
        processFile(p);
        ++count;
    }
    return count;
}

void PipelineWatcher::processFile(const std::filesystem::path& filePath) {
    ChangeEvent event;
    if (!ChangeEvent::readFromFile(filePath, event)) return;

    std::lock_guard<std::mutex> lock(m_callbackMutex);
    for (auto& cb : m_callbacks) {
        cb(event);
    }
}

void PipelineWatcher::watchLoop() {
    using namespace std::chrono_literals;
    while (m_running.load()) {
        poll();
        std::this_thread::sleep_for(100ms);
    }
}

// ── PipelineDirectories ───────────────────────────────────────────────────

PipelineDirectories PipelineDirectories::fromRoot(
    const std::filesystem::path& workspaceRoot)
{
    PipelineDirectories d;
    d.root          = workspaceRoot;
    d.dotAtlas      = workspaceRoot / ".atlas";
    d.pipeline      = d.dotAtlas / "pipeline";
    d.changes       = d.pipeline  / "changes";
    d.assets        = d.pipeline  / "assets";
    d.worlds        = d.pipeline  / "worlds";
    d.scripts       = d.pipeline  / "scripts";
    d.animations    = d.pipeline  / "animations";
    d.sessions      = d.pipeline  / "sessions";
    d.manifestFile  = d.dotAtlas / "manifest.json";
    d.watchLogFile  = d.dotAtlas / "watch.log";
    return d;
}

bool PipelineDirectories::ensureCreated() const {
    std::error_code ec;
    for (const auto& dir : { changes, assets, worlds, scripts,
                              animations, sessions }) {
        std::filesystem::create_directories(dir, ec);
        if (ec) return false;
    }
    return true;
}

} // namespace NF
