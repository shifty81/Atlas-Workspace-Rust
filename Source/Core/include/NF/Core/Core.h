#pragma once
// NF::Core — Math, memory, logging, events, reflection, serialization

#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <chrono>
#include <format>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <mutex>

namespace NF {

// ── Logging ──────────────────────────────────────────────────────

enum class LogLevel : uint8_t {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger {
public:
    using SinkCallback = std::function<void(LogLevel, std::string_view, std::string_view)>;

    static Logger& instance() {
        static Logger s;
        return s;
    }

    void log(LogLevel level, std::string_view category, std::string_view message) {
        if (level < m_minLevel) return;
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif
        std::cout << std::format("[{:02}:{:02}:{:02}] [{}] [{}] {}\n",
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            levelString(level), category, message);

        // Notify registered sinks (thread-safe copy)
        std::vector<SinkCallback> sinks;
        {
            std::lock_guard<std::mutex> lock(m_sinkMutex);
            sinks = m_sinks;
        }
        for (auto& sink : sinks) {
            if (sink) sink(level, category, message);
        }
    }

    /// Register a callback that receives every log message.
    /// Returns an ID that can be passed to removeSink().
    size_t addSink(SinkCallback cb) {
        std::lock_guard<std::mutex> lock(m_sinkMutex);
        size_t id = m_nextSinkId++;
        m_sinks.push_back(std::move(cb));
        m_sinkIds.push_back(id);
        return id;
    }

    /// Remove a previously registered sink by ID.
    void removeSink(size_t id) {
        std::lock_guard<std::mutex> lock(m_sinkMutex);
        for (size_t i = 0; i < m_sinkIds.size(); ++i) {
            if (m_sinkIds[i] == id) {
                m_sinks.erase(m_sinks.begin() + static_cast<ptrdiff_t>(i));
                m_sinkIds.erase(m_sinkIds.begin() + static_cast<ptrdiff_t>(i));
                return;
            }
        }
    }

    void setMinLevel(LogLevel level) { m_minLevel = level; }
    LogLevel minLevel() const { return m_minLevel; }

    /// Public accessor for the level string (used by log sinks for file output).
    static constexpr const char* levelTag(LogLevel level) { return levelString(level); }

private:
    Logger() = default;
    LogLevel m_minLevel = LogLevel::Info;
    std::vector<SinkCallback> m_sinks;
    std::vector<size_t> m_sinkIds;
    size_t m_nextSinkId = 1;
    mutable std::mutex m_sinkMutex;

    static constexpr const char* levelString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info:  return "INFO ";
            case LogLevel::Warn:  return "WARN ";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Fatal: return "FATAL";
            default:              return "?????";
        }
    }
};

#define NF_LOG(level, category, msg) \
    ::NF::Logger::instance().log(::NF::LogLevel::level, category, msg)

#define NF_LOG_INFO(cat, msg)  NF_LOG(Info,  cat, msg)
#define NF_LOG_WARN(cat, msg)  NF_LOG(Warn,  cat, msg)
#define NF_LOG_ERROR(cat, msg) NF_LOG(Error, cat, msg)
#define NF_LOG_DEBUG(cat, msg) NF_LOG(Debug, cat, msg)
#define NF_LOG_TRACE(cat, msg) NF_LOG(Trace, cat, msg)

// ── Math types ───────────────────────────────────────────────────

struct Vec2 {
    float x = 0.f, y = 0.f;
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
    float lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSq()); }
    Vec2 normalized() const {
        float len = length();
        if (len < 1e-7f) return {0.f, 0.f};
        return {x / len, y / len};
    }
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }
};

struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    float lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSq()); }
    Vec3 normalized() const {
        float len = length();
        if (len < 1e-7f) return {0.f, 0.f, 0.f};
        return {x / len, y / len, z / len};
    }
    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vec3& o) const { return !(*this == o); }

    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }
};

struct Vec4 {
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
    Vec4 operator+(const Vec4& o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
    Vec4 operator-(const Vec4& o) const { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
    Vec4 operator*(float s) const { return {x * s, y * s, z * s, w * s}; }
    float dot(const Vec4& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
    bool operator==(const Vec4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
    bool operator!=(const Vec4& o) const { return !(*this == o); }
};

struct Rect {
    float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
    bool overlaps(const Rect& o) const {
        return x < o.x + o.w && x + w > o.x &&
               y < o.y + o.h && y + h > o.y;
    }
};

// ── Quaternion ───────────────────────────────────────────────────

struct Quat {
    float x = 0.f, y = 0.f, z = 0.f, w = 1.f;

    static Quat identity() { return {0.f, 0.f, 0.f, 1.f}; }

    static Quat fromAxisAngle(const Vec3& axis, float angleRadians) {
        float half = angleRadians * 0.5f;
        float s = std::sin(half);
        Vec3 n = axis.normalized();
        return {n.x * s, n.y * s, n.z * s, std::cos(half)};
    }

    static Quat fromEuler(float pitchRad, float yawRad, float rollRad) {
        float cp = std::cos(pitchRad * 0.5f), sp = std::sin(pitchRad * 0.5f);
        float cy = std::cos(yawRad   * 0.5f), sy = std::sin(yawRad   * 0.5f);
        float cr = std::cos(rollRad  * 0.5f), sr = std::sin(rollRad  * 0.5f);
        return {
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        };
    }

    Quat operator*(const Quat& o) const {
        return {
            w * o.x + x * o.w + y * o.z - z * o.y,
            w * o.y - x * o.z + y * o.w + z * o.x,
            w * o.z + x * o.y - y * o.x + z * o.w,
            w * o.w - x * o.x - y * o.y - z * o.z
        };
    }

    Quat conjugate() const { return {-x, -y, -z, w}; }

    float lengthSq() const { return x * x + y * y + z * z + w * w; }
    float length() const { return std::sqrt(lengthSq()); }

    Quat normalized() const {
        float len = length();
        if (len < 1e-7f) return identity();
        return {x / len, y / len, z / len, w / len};
    }

    Vec3 rotate(const Vec3& v) const {
        Quat p{v.x, v.y, v.z, 0.f};
        Quat result = *this * p * conjugate();
        return {result.x, result.y, result.z};
    }

    static Quat slerp(const Quat& a, const Quat& b, float t) {
        float cosTheta = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        Quat target = b;
        if (cosTheta < 0.f) {
            target = {-b.x, -b.y, -b.z, -b.w};
            cosTheta = -cosTheta;
        }
        if (cosTheta > 0.9995f) {
            return Quat{
                a.x + (target.x - a.x) * t,
                a.y + (target.y - a.y) * t,
                a.z + (target.z - a.z) * t,
                a.w + (target.w - a.w) * t
            }.normalized();
        }
        float theta = std::acos(cosTheta);
        float sinTheta = std::sin(theta);
        float wa = std::sin((1.f - t) * theta) / sinTheta;
        float wb = std::sin(t * theta) / sinTheta;
        return Quat{
            a.x * wa + target.x * wb,
            a.y * wa + target.y * wb,
            a.z * wa + target.z * wb,
            a.w * wa + target.w * wb
        };
    }
};

// ── 4x4 Matrix (column-major) ────────────────────────────────────

struct Mat4 {
    float m[16] = {};

    static Mat4 identity() {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f;
        return r;
    }

    static Mat4 translation(float tx, float ty, float tz) {
        Mat4 r = identity();
        r.m[12] = tx; r.m[13] = ty; r.m[14] = tz;
        return r;
    }

    static Mat4 translation(const Vec3& v) { return translation(v.x, v.y, v.z); }

    static Mat4 scaling(float sx, float sy, float sz) {
        Mat4 r;
        r.m[0] = sx; r.m[5] = sy; r.m[10] = sz; r.m[15] = 1.f;
        return r;
    }

    static Mat4 scaling(const Vec3& v) { return scaling(v.x, v.y, v.z); }

    static Mat4 fromQuat(const Quat& q) {
        Mat4 r = identity();
        float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
        float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
        r.m[0]  = 1.f - 2.f * (yy + zz);
        r.m[1]  = 2.f * (xy + wz);
        r.m[2]  = 2.f * (xz - wy);
        r.m[4]  = 2.f * (xy - wz);
        r.m[5]  = 1.f - 2.f * (xx + zz);
        r.m[6]  = 2.f * (yz + wx);
        r.m[8]  = 2.f * (xz + wy);
        r.m[9]  = 2.f * (yz - wx);
        r.m[10] = 1.f - 2.f * (xx + yy);
        return r;
    }

    static Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane) {
        Mat4 r;
        float tanHalf = std::tan(fovRadians * 0.5f);
        r.m[0]  = 1.f / (aspect * tanHalf);
        r.m[5]  = 1.f / tanHalf;
        r.m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        r.m[11] = -1.f;
        r.m[14] = -(2.f * farPlane * nearPlane) / (farPlane - nearPlane);
        return r;
    }

    static Mat4 ortho(float left, float right, float bottom, float top,
                      float nearPlane, float farPlane) {
        Mat4 r;
        r.m[0]  =  2.f / (right - left);
        r.m[5]  =  2.f / (top - bottom);
        r.m[10] = -2.f / (farPlane - nearPlane);
        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        r.m[15] = 1.f;
        return r;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 r = f.cross(up).normalized();
        Vec3 u = r.cross(f);
        Mat4 res = identity();
        res.m[0] = r.x;  res.m[4] = r.y;  res.m[8]  = r.z;
        res.m[1] = u.x;  res.m[5] = u.y;  res.m[9]  = u.z;
        res.m[2] = -f.x; res.m[6] = -f.y; res.m[10] = -f.z;
        res.m[12] = -r.dot(eye);
        res.m[13] = -u.dot(eye);
        res.m[14] =  f.dot(eye);
        return res;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.f;
                for (int k = 0; k < 4; ++k)
                    sum += m[k * 4 + row] * o.m[col * 4 + k];
                r.m[col * 4 + row] = sum;
            }
        }
        return r;
    }

    Vec4 operator*(const Vec4& v) const {
        return {
            m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12] * v.w,
            m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13] * v.w,
            m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
            m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
        };
    }

    Vec3 transformPoint(const Vec3& p) const {
        Vec4 r = *this * Vec4{p.x, p.y, p.z, 1.f};
        return {r.x, r.y, r.z};
    }

    Vec3 transformDir(const Vec3& d) const {
        Vec4 r = *this * Vec4{d.x, d.y, d.z, 0.f};
        return {r.x, r.y, r.z};
    }

    float& at(int row, int col) { return m[col * 4 + row]; }
    float  at(int row, int col) const { return m[col * 4 + row]; }
};

// ── Transform ────────────────────────────────────────────────────

struct Transform {
    Vec3 position{0.f, 0.f, 0.f};
    Quat rotation = Quat::identity();
    Vec3 scale{1.f, 1.f, 1.f};

    Mat4 toMatrix() const {
        return Mat4::translation(position)
             * Mat4::fromQuat(rotation)
             * Mat4::scaling(scale);
    }

    Vec3 forward() const { return rotation.rotate({0.f, 0.f, -1.f}); }
    Vec3 right()   const { return rotation.rotate({1.f, 0.f,  0.f}); }
    Vec3 up()      const { return rotation.rotate({0.f, 1.f,  0.f}); }
};

// ── StringID (hashed string identifier) ──────────────────────────

class StringID {
public:
    constexpr StringID() = default;
    explicit constexpr StringID(uint32_t hash) : m_hash(hash) {}

    static constexpr uint32_t fnv1a(const char* str, uint32_t hash = 2166136261u) {
        return (str[0] == '\0') ? hash : fnv1a(str + 1, (hash ^ static_cast<uint32_t>(str[0])) * 16777619u);
    }

    explicit constexpr StringID(const char* str) : m_hash(fnv1a(str)) {}
    explicit StringID(std::string_view str) : m_hash(computeHash(str)) {}

    constexpr uint32_t hash() const { return m_hash; }
    constexpr bool isValid() const { return m_hash != 0; }

    constexpr bool operator==(const StringID& o) const { return m_hash == o.m_hash; }
    constexpr bool operator!=(const StringID& o) const { return m_hash != o.m_hash; }
    constexpr bool operator<(const StringID& o) const { return m_hash < o.m_hash; }

private:
    uint32_t m_hash = 0;

    static uint32_t computeHash(std::string_view str) {
        uint32_t hash = 2166136261u;
        for (char c : str)
            hash = (hash ^ static_cast<uint32_t>(c)) * 16777619u;
        return hash;
    }
};

} // namespace NF

template<>
struct std::hash<NF::StringID> {
    size_t operator()(const NF::StringID& id) const noexcept {
        return static_cast<size_t>(id.hash());
    }
};

namespace NF {

// ── Timer ────────────────────────────────────────────────────────

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;

    void reset() { m_start = Clock::now(); m_last = m_start; }

    float elapsed() const {
        return std::chrono::duration<float>(Clock::now() - m_start).count();
    }

    float delta() {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - m_last).count();
        m_last = now;
        return dt;
    }

private:
    Clock::time_point m_start = Clock::now();
    Clock::time_point m_last  = Clock::now();
};

// ── Event bus ────────────────────────────────────────────────────

using EventID = uint32_t;

class EventBus {
public:
    using Handler = std::function<void(const void*)>;

    void subscribe(EventID id, Handler handler) {
        m_handlers[id].push_back(std::move(handler));
    }

    void publish(EventID id, const void* data = nullptr) {
        auto it = m_handlers.find(id);
        if (it != m_handlers.end()) {
            for (auto& h : it->second) h(data);
        }
    }

    void clear() { m_handlers.clear(); }

    void clear(EventID id) {
        auto it = m_handlers.find(id);
        if (it != m_handlers.end()) it->second.clear();
    }

    size_t subscriberCount(EventID id) const {
        auto it = m_handlers.find(id);
        return (it != m_handlers.end()) ? it->second.size() : 0;
    }

private:
    std::unordered_map<EventID, std::vector<Handler>> m_handlers;
};

// ── Type ID (compile-time type identification) ───────────────────

class TypeID {
public:
    template<typename T>
    static uint32_t get() {
        static const uint32_t id = s_nextID++;
        return id;
    }
private:
    static inline uint32_t s_nextID = 1;
};

// ── Memory: Linear Allocator ─────────────────────────────────────

class LinearAllocator {
public:
    explicit LinearAllocator(size_t capacityBytes)
        : m_capacity(capacityBytes)
    {
        m_buffer = std::make_unique<uint8_t[]>(capacityBytes);
    }

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t aligned = align(m_offset, alignment);
        if (aligned + size > m_capacity) return nullptr;
        void* ptr = m_buffer.get() + aligned;
        m_offset = aligned + size;
        ++m_allocationCount;
        return ptr;
    }

    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        if (!mem) return nullptr;
        return new (mem) T(std::forward<Args>(args)...);
    }

    void reset() { m_offset = 0; m_allocationCount = 0; }

    [[nodiscard]] size_t used() const { return m_offset; }
    [[nodiscard]] size_t capacity() const { return m_capacity; }
    [[nodiscard]] size_t remaining() const { return m_capacity - m_offset; }
    [[nodiscard]] size_t allocationCount() const { return m_allocationCount; }

private:
    static size_t align(size_t offset, size_t alignment) {
        return (offset + alignment - 1) & ~(alignment - 1);
    }

    std::unique_ptr<uint8_t[]> m_buffer;
    size_t m_capacity = 0;
    size_t m_offset = 0;
    size_t m_allocationCount = 0;
};

// ── Memory: Pool Allocator ───────────────────────────────────────

class PoolAllocator {
public:
    PoolAllocator(size_t objectSize, size_t count)
        : m_objectSize(std::max(objectSize, sizeof(void*)))
        , m_count(count)
    {
        m_buffer = std::make_unique<uint8_t[]>(m_objectSize * count);
        buildFreeList();
    }

    void* allocate() {
        if (!m_freeHead) return nullptr;
        void* ptr = m_freeHead;
        m_freeHead = *reinterpret_cast<void**>(m_freeHead);
        ++m_activeCount;
        return ptr;
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        *reinterpret_cast<void**>(ptr) = m_freeHead;
        m_freeHead = ptr;
        --m_activeCount;
    }

    template<typename T, typename... Args>
    T* create(Args&&... args) {
        static_assert(sizeof(T) <= sizeof(void*) || sizeof(T) <= 4096,
            "Object too large for pool");
        void* mem = allocate();
        if (!mem) return nullptr;
        return new (mem) T(std::forward<Args>(args)...);
    }

    template<typename T>
    void destroy(T* ptr) {
        if (!ptr) return;
        ptr->~T();
        deallocate(ptr);
    }

    void reset() {
        m_activeCount = 0;
        buildFreeList();
    }

    [[nodiscard]] size_t objectSize() const { return m_objectSize; }
    [[nodiscard]] size_t totalCount() const { return m_count; }
    [[nodiscard]] size_t activeCount() const { return m_activeCount; }
    [[nodiscard]] size_t freeCount() const { return m_count - m_activeCount; }

private:
    void buildFreeList() {
        m_freeHead = nullptr;
        for (size_t i = m_count; i > 0; --i) {
            void* block = m_buffer.get() + (i - 1) * m_objectSize;
            *reinterpret_cast<void**>(block) = m_freeHead;
            m_freeHead = block;
        }
    }

    std::unique_ptr<uint8_t[]> m_buffer;
    size_t m_objectSize;
    size_t m_count;
    size_t m_activeCount = 0;
    void*  m_freeHead = nullptr;
};

// ── Reflection: PropertyInfo ─────────────────────────────────────

enum class PropertyType : uint8_t {
    Int32,
    Float,
    Bool,
    String,
    Vec2,
    Vec3,
    Vec4,
    Quat,
    Color,
    Enum,
    Custom
};

struct PropertyInfo {
    std::string name;
    PropertyType type = PropertyType::Custom;
    size_t offset = 0;
    size_t size = 0;
};

class TypeDescriptor {
public:
    explicit TypeDescriptor(std::string name, size_t size)
        : m_name(std::move(name)), m_size(size) {}

    TypeDescriptor& addProperty(const std::string& name, PropertyType type,
                                 size_t offset, size_t size) {
        m_properties.push_back({name, type, offset, size});
        return *this;
    }

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] size_t size() const { return m_size; }
    [[nodiscard]] const std::vector<PropertyInfo>& properties() const { return m_properties; }

    [[nodiscard]] const PropertyInfo* findProperty(std::string_view propName) const {
        for (auto& p : m_properties) {
            if (p.name == propName) return &p;
        }
        return nullptr;
    }

private:
    std::string m_name;
    size_t m_size;
    std::vector<PropertyInfo> m_properties;
};

class TypeRegistry {
public:
    static TypeRegistry& instance() {
        static TypeRegistry reg;
        return reg;
    }

    TypeDescriptor& registerType(const std::string& name, size_t size) {
        auto it = m_types.find(name);
        if (it != m_types.end()) return it->second;
        auto [newIt, _] = m_types.emplace(name, TypeDescriptor(name, size));
        return newIt->second;
    }

    [[nodiscard]] const TypeDescriptor* findType(const std::string& name) const {
        auto it = m_types.find(name);
        return (it != m_types.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] size_t typeCount() const { return m_types.size(); }

private:
    TypeRegistry() = default;
    std::unordered_map<std::string, TypeDescriptor> m_types;
};

// Helper macro to register a type with reflection
#define NF_REFLECT_TYPE(TypeName) \
    NF::TypeRegistry::instance().registerType(#TypeName, sizeof(TypeName))

#define NF_REFLECT_PROPERTY(desc, TypeName, PropName, PropType) \
    (desc).addProperty(#PropName, PropType, offsetof(TypeName, PropName), sizeof(((TypeName*)nullptr)->PropName))

// ── Serialization: JSON Value ────────────────────────────────────

enum class JsonType : uint8_t {
    Null,
    Bool,
    Int,
    Float,
    String,
    Array,
    Object
};

class JsonValue {
public:
    JsonValue() : m_type(JsonType::Null) {}
    explicit JsonValue(bool v) : m_type(JsonType::Bool), m_bool(v) {}
    explicit JsonValue(int32_t v) : m_type(JsonType::Int), m_int(v) {}
    explicit JsonValue(float v) : m_type(JsonType::Float), m_float(v) {}
    explicit JsonValue(double v) : m_type(JsonType::Float), m_float(static_cast<float>(v)) {}
    explicit JsonValue(const std::string& v) : m_type(JsonType::String), m_string(v) {}
    explicit JsonValue(const char* v) : m_type(JsonType::String), m_string(v) {}

    static JsonValue array() { JsonValue v; v.m_type = JsonType::Array; return v; }
    static JsonValue object() { JsonValue v; v.m_type = JsonType::Object; return v; }

    // Type checks
    [[nodiscard]] JsonType type() const { return m_type; }
    [[nodiscard]] bool isNull() const { return m_type == JsonType::Null; }
    [[nodiscard]] bool isBool() const { return m_type == JsonType::Bool; }
    [[nodiscard]] bool isInt() const { return m_type == JsonType::Int; }
    [[nodiscard]] bool isFloat() const { return m_type == JsonType::Float; }
    [[nodiscard]] bool isNumber() const { return m_type == JsonType::Int || m_type == JsonType::Float; }
    [[nodiscard]] bool isString() const { return m_type == JsonType::String; }
    [[nodiscard]] bool isArray() const { return m_type == JsonType::Array; }
    [[nodiscard]] bool isObject() const { return m_type == JsonType::Object; }

    // Getters
    [[nodiscard]] bool asBool(bool def = false) const { return isBool() ? m_bool : def; }
    [[nodiscard]] int32_t asInt(int32_t def = 0) const {
        if (isInt()) return m_int;
        if (isFloat()) return static_cast<int32_t>(m_float);
        return def;
    }
    [[nodiscard]] float asFloat(float def = 0.f) const {
        if (isFloat()) return m_float;
        if (isInt()) return static_cast<float>(m_int);
        return def;
    }
    [[nodiscard]] const std::string& asString() const {
        static const std::string empty;
        return isString() ? m_string : empty;
    }

    // Array operations
    void push(JsonValue val) {
        if (m_type != JsonType::Array) m_type = JsonType::Array;
        m_array.push_back(std::move(val));
    }
    [[nodiscard]] size_t size() const {
        if (isArray()) return m_array.size();
        if (isObject()) return m_object.size();
        return 0;
    }
    const JsonValue& operator[](size_t index) const {
        static const JsonValue null;
        return (isArray() && index < m_array.size()) ? m_array[index] : null;
    }
    JsonValue& operator[](size_t index) {
        if (m_type != JsonType::Array) m_type = JsonType::Array;
        if (index >= m_array.size()) m_array.resize(index + 1);
        return m_array[index];
    }

    // Object operations
    void set(const std::string& key, JsonValue val) {
        if (m_type != JsonType::Object) m_type = JsonType::Object;
        m_object[key] = std::move(val);
    }
    const JsonValue& operator[](const std::string& key) const {
        static const JsonValue null;
        if (!isObject()) return null;
        auto it = m_object.find(key);
        return (it != m_object.end()) ? it->second : null;
    }
    JsonValue& operator[](const std::string& key) {
        if (m_type != JsonType::Object) m_type = JsonType::Object;
        return m_object[key];
    }
    [[nodiscard]] bool hasKey(const std::string& key) const {
        return isObject() && m_object.count(key) > 0;
    }
    [[nodiscard]] const std::unordered_map<std::string, JsonValue>& members() const {
        return m_object;
    }
    [[nodiscard]] const std::vector<JsonValue>& elements() const {
        return m_array;
    }

    // Serialization to string
    [[nodiscard]] std::string toJson(int indent = 0) const {
        return serialize(indent, 0);
    }

private:
    JsonType m_type;
    bool m_bool = false;
    int32_t m_int = 0;
    float m_float = 0.f;
    std::string m_string;
    std::vector<JsonValue> m_array;
    std::unordered_map<std::string, JsonValue> m_object;

    [[nodiscard]] std::string serialize(int indent, int depth) const {
        switch (m_type) {
        case JsonType::Null: return "null";
        case JsonType::Bool: return m_bool ? "true" : "false";
        case JsonType::Int: return std::to_string(m_int);
        case JsonType::Float: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.6g", static_cast<double>(m_float));
            return buf;
        }
        case JsonType::String: return "\"" + escapeString(m_string) + "\"";
        case JsonType::Array: {
            if (m_array.empty()) return "[]";
            std::string s = "[";
            bool compact = indent == 0;
            for (size_t i = 0; i < m_array.size(); ++i) {
                if (!compact) s += "\n" + std::string((depth + 1) * indent, ' ');
                s += m_array[i].serialize(indent, depth + 1);
                if (i + 1 < m_array.size()) s += ",";
                if (compact && i + 1 < m_array.size()) s += " ";
            }
            if (!compact) s += "\n" + std::string(depth * indent, ' ');
            s += "]";
            return s;
        }
        case JsonType::Object: {
            if (m_object.empty()) return "{}";
            std::string s = "{";
            bool compact = indent == 0;
            size_t idx = 0;
            // Use sorted keys for deterministic output
            std::vector<std::string> keys;
            keys.reserve(m_object.size());
            for (auto& [k, _] : m_object) keys.push_back(k);
            std::sort(keys.begin(), keys.end());
            for (auto& k : keys) {
                if (!compact) s += "\n" + std::string((depth + 1) * indent, ' ');
                s += "\"" + escapeString(k) + "\":";
                if (!compact) s += " ";
                s += m_object.at(k).serialize(indent, depth + 1);
                if (idx + 1 < keys.size()) s += ",";
                if (compact && idx + 1 < keys.size()) s += " ";
                ++idx;
            }
            if (!compact) s += "\n" + std::string(depth * indent, ' ');
            s += "}";
            return s;
        }
        }
        return "null";
    }

    static std::string escapeString(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c; break;
            }
        }
        return out;
    }
};

// ── Serialization: JSON Parser ───────────────────────────────────

class JsonParser {
public:
    static JsonValue parse(std::string_view input) {
        size_t pos = 0;
        skipWhitespace(input, pos);
        if (pos >= input.size()) return {};
        return parseValue(input, pos);
    }

private:
    static void skipWhitespace(std::string_view s, size_t& pos) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' ||
               s[pos] == '\n' || s[pos] == '\r')) ++pos;
    }

    static JsonValue parseValue(std::string_view s, size_t& pos) {
        skipWhitespace(s, pos);
        if (pos >= s.size()) return {};

        char c = s[pos];
        if (c == '"') return parseString(s, pos);
        if (c == '{') return parseObject(s, pos);
        if (c == '[') return parseArray(s, pos);
        if (c == 't' || c == 'f') return parseBool(s, pos);
        if (c == 'n') return parseNull(s, pos);
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(s, pos);
        return {};
    }

    static JsonValue parseString(std::string_view s, size_t& pos) {
        ++pos; // skip opening quote
        std::string result;
        while (pos < s.size() && s[pos] != '"') {
            if (s[pos] == '\\' && pos + 1 < s.size()) {
                ++pos;
                switch (s[pos]) {
                case '"':  result += '"'; break;
                case '\\': result += '\\'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                default:   result += s[pos]; break;
                }
            } else {
                result += s[pos];
            }
            ++pos;
        }
        if (pos < s.size()) ++pos; // skip closing quote
        return JsonValue(result);
    }

    static JsonValue parseNumber(std::string_view s, size_t& pos) {
        size_t start = pos;
        bool isFloat = false;
        if (s[pos] == '-') ++pos;
        while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        if (pos < s.size() && s[pos] == '.') { isFloat = true; ++pos; }
        while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
            isFloat = true;
            ++pos;
            if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
            while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        }
        std::string numStr(s.substr(start, pos - start));
        if (isFloat) return JsonValue(std::stof(numStr));
        return JsonValue(std::stoi(numStr));
    }

    static JsonValue parseBool(std::string_view s, size_t& pos) {
        if (s.substr(pos, 4) == "true") { pos += 4; return JsonValue(true); }
        if (s.substr(pos, 5) == "false") { pos += 5; return JsonValue(false); }
        return {};
    }

    static JsonValue parseNull(std::string_view s, size_t& pos) {
        if (s.substr(pos, 4) == "null") { pos += 4; return JsonValue(); }
        return {};
    }

    static JsonValue parseArray(std::string_view s, size_t& pos) {
        ++pos; // skip [
        JsonValue arr = JsonValue::array();
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') { ++pos; return arr; }
        while (pos < s.size()) {
            arr.push(parseValue(s, pos));
            skipWhitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; skipWhitespace(s, pos); }
            else break;
        }
        if (pos < s.size() && s[pos] == ']') ++pos;
        return arr;
    }

    static JsonValue parseObject(std::string_view s, size_t& pos) {
        ++pos; // skip {
        JsonValue obj = JsonValue::object();
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == '}') { ++pos; return obj; }
        while (pos < s.size()) {
            skipWhitespace(s, pos);
            if (pos >= s.size() || s[pos] != '"') break;
            auto keyVal = parseString(s, pos);
            skipWhitespace(s, pos);
            if (pos < s.size() && s[pos] == ':') ++pos;
            obj.set(keyVal.asString(), parseValue(s, pos));
            skipWhitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; skipWhitespace(s, pos); }
            else break;
        }
        if (pos < s.size() && s[pos] == '}') ++pos;
        return obj;
    }
};

// ── Undo/Redo: Command Pattern ───────────────────────────────────

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    [[nodiscard]] virtual std::string description() const = 0;
};

class CommandStack {
public:
    void execute(std::unique_ptr<ICommand> cmd) {
        cmd->execute();
        m_undoStack.push_back(std::move(cmd));
        m_redoStack.clear();
        m_dirty = true;
    }

    bool canUndo() const { return !m_undoStack.empty(); }
    bool canRedo() const { return !m_redoStack.empty(); }

    bool undo() {
        if (m_undoStack.empty()) return false;
        auto cmd = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        cmd->undo();
        m_redoStack.push_back(std::move(cmd));
        m_dirty = true;
        return true;
    }

    bool redo() {
        if (m_redoStack.empty()) return false;
        auto cmd = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        cmd->execute();
        m_undoStack.push_back(std::move(cmd));
        m_dirty = true;
        return true;
    }

    void clear() {
        m_undoStack.clear();
        m_redoStack.clear();
        m_dirty = false;
    }

    [[nodiscard]] size_t undoCount() const { return m_undoStack.size(); }
    [[nodiscard]] size_t redoCount() const { return m_redoStack.size(); }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markClean() { m_dirty = false; }

    [[nodiscard]] std::string undoDescription() const {
        return m_undoStack.empty() ? "" : m_undoStack.back()->description();
    }
    [[nodiscard]] std::string redoDescription() const {
        return m_redoStack.empty() ? "" : m_redoStack.back()->description();
    }

private:
    std::vector<std::unique_ptr<ICommand>> m_undoStack;
    std::vector<std::unique_ptr<ICommand>> m_redoStack;
    bool m_dirty = false;
};

// ── Property Change Command ──────────────────────────────────────

template<typename T>
class PropertyChangeCommand : public ICommand {
public:
    PropertyChangeCommand(T* target, T oldValue, T newValue, std::string desc)
        : m_target(target), m_oldValue(std::move(oldValue)),
          m_newValue(std::move(newValue)), m_desc(std::move(desc)) {}

    void execute() override { *m_target = m_newValue; }
    void undo() override { *m_target = m_oldValue; }
    [[nodiscard]] std::string description() const override { return m_desc; }

private:
    T* m_target;
    T m_oldValue;
    T m_newValue;
    std::string m_desc;
};

// ── Color type (for inspector property editing) ──────────────────

struct Color {
    float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

    static Color white() { return {1.f, 1.f, 1.f, 1.f}; }
    static Color black() { return {0.f, 0.f, 0.f, 1.f}; }
    static Color red()   { return {1.f, 0.f, 0.f, 1.f}; }
    static Color green() { return {0.f, 1.f, 0.f, 1.f}; }
    static Color blue()  { return {0.f, 0.f, 1.f, 1.f}; }

    uint32_t toRGBA8() const {
        auto clamp01 = [](float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
        uint8_t ri = static_cast<uint8_t>(clamp01(r) * 255.f);
        uint8_t gi = static_cast<uint8_t>(clamp01(g) * 255.f);
        uint8_t bi = static_cast<uint8_t>(clamp01(b) * 255.f);
        uint8_t ai = static_cast<uint8_t>(clamp01(a) * 255.f);
        return (ri << 24) | (gi << 16) | (bi << 8) | ai;
    }

    static Color fromRGBA8(uint32_t packed) {
        return {
            static_cast<float>((packed >> 24) & 0xFF) / 255.f,
            static_cast<float>((packed >> 16) & 0xFF) / 255.f,
            static_cast<float>((packed >>  8) & 0xFF) / 255.f,
            static_cast<float>((packed >>  0) & 0xFF) / 255.f
        };
    }

    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    bool operator!=(const Color& o) const { return !(*this == o); }
};

// ── Version ──────────────────────────────────────────────────────

constexpr int NF_VERSION_MAJOR = 0;
constexpr int NF_VERSION_MINOR = 1;
constexpr int NF_VERSION_PATCH = 0;
constexpr const char* NF_VERSION_STRING = "0.1.0";

// ── Core lifecycle ───────────────────────────────────────────────

void coreInit();
void coreShutdown();
bool isCoreInitialized();

} // namespace NF
