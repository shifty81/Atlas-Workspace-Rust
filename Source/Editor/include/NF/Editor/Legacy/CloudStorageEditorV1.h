#pragma once
// NF::Editor — Cloud storage editor v1: bucket/asset cloud storage management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Csev1BucketState : uint8_t { Unknown, Available, Full, Archived, Error };
enum class Csev1AccessLevel : uint8_t { Private, Internal, Public, ReadOnly };

inline const char* csev1BucketStateName(Csev1BucketState s) {
    switch (s) {
        case Csev1BucketState::Unknown:   return "Unknown";
        case Csev1BucketState::Available: return "Available";
        case Csev1BucketState::Full:      return "Full";
        case Csev1BucketState::Archived:  return "Archived";
        case Csev1BucketState::Error:     return "Error";
    }
    return "Unknown";
}

inline const char* csev1AccessLevelName(Csev1AccessLevel a) {
    switch (a) {
        case Csev1AccessLevel::Private:  return "Private";
        case Csev1AccessLevel::Internal: return "Internal";
        case Csev1AccessLevel::Public:   return "Public";
        case Csev1AccessLevel::ReadOnly: return "ReadOnly";
    }
    return "Unknown";
}

struct Csev1AssetRef {
    std::string assetId;
    std::string remotePath;
    bool isValid() const { return !assetId.empty() && !remotePath.empty(); }
};

struct Csev1Bucket {
    uint64_t         id      = 0;
    std::string      name;
    Csev1BucketState state   = Csev1BucketState::Unknown;
    Csev1AccessLevel access  = Csev1AccessLevel::Private;
    std::vector<Csev1AssetRef> assets;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isAvailable()  const { return state == Csev1BucketState::Available; }
    [[nodiscard]] bool isArchived()   const { return state == Csev1BucketState::Archived; }
    [[nodiscard]] bool hasError()     const { return state == Csev1BucketState::Error; }

    bool addAsset(const Csev1AssetRef& ref) {
        if (!ref.isValid()) return false;
        for (const auto& a : assets) if (a.assetId == ref.assetId) return false;
        assets.push_back(ref);
        return true;
    }
};

using Csev1ChangeCallback = std::function<void(uint64_t)>;

class CloudStorageEditorV1 {
public:
    static constexpr size_t MAX_BUCKETS = 128;

    bool addBucket(const Csev1Bucket& bucket) {
        if (!bucket.isValid()) return false;
        for (const auto& b : m_buckets) if (b.id == bucket.id) return false;
        if (m_buckets.size() >= MAX_BUCKETS) return false;
        m_buckets.push_back(bucket);
        return true;
    }

    bool removeBucket(uint64_t id) {
        for (auto it = m_buckets.begin(); it != m_buckets.end(); ++it) {
            if (it->id == id) { m_buckets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Csev1Bucket* findBucket(uint64_t id) {
        for (auto& b : m_buckets) if (b.id == id) return &b;
        return nullptr;
    }

    bool setAccessLevel(uint64_t id, Csev1AccessLevel access) {
        auto* b = findBucket(id);
        if (!b) return false;
        b->access = access;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setState(uint64_t id, Csev1BucketState state) {
        auto* b = findBucket(id);
        if (!b) return false;
        b->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t bucketCount()    const { return m_buckets.size(); }
    [[nodiscard]] size_t availableCount() const {
        size_t c = 0; for (const auto& b : m_buckets) if (b.isAvailable()) ++c; return c;
    }
    [[nodiscard]] size_t archivedCount()  const {
        size_t c = 0; for (const auto& b : m_buckets) if (b.isArchived())  ++c; return c;
    }
    [[nodiscard]] size_t countByAccess(Csev1AccessLevel access) const {
        size_t c = 0; for (const auto& b : m_buckets) if (b.access == access) ++c; return c;
    }

    void setOnChange(Csev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Csev1Bucket> m_buckets;
    Csev1ChangeCallback      m_onChange;
};

} // namespace NF
