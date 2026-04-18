#pragma once
// NF::Editor — analytics dashboard
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class AnalyticsMetricType : uint8_t {
    Counter, Gauge, Histogram, Summary, Rate, Average, Percentile
};

inline const char* analyticsMetricTypeName(AnalyticsMetricType t) {
    switch (t) {
        case AnalyticsMetricType::Counter:    return "Counter";
        case AnalyticsMetricType::Gauge:      return "Gauge";
        case AnalyticsMetricType::Histogram:  return "Histogram";
        case AnalyticsMetricType::Summary:    return "Summary";
        case AnalyticsMetricType::Rate:       return "Rate";
        case AnalyticsMetricType::Average:    return "Average";
        case AnalyticsMetricType::Percentile: return "Percentile";
    }
    return "Unknown";
}

enum class AnalyticsTimeRange : uint8_t {
    LastHour, LastDay, LastWeek, LastMonth, AllTime, Custom
};

inline const char* analyticsTimeRangeName(AnalyticsTimeRange r) {
    switch (r) {
        case AnalyticsTimeRange::LastHour:  return "LastHour";
        case AnalyticsTimeRange::LastDay:   return "LastDay";
        case AnalyticsTimeRange::LastWeek:  return "LastWeek";
        case AnalyticsTimeRange::LastMonth: return "LastMonth";
        case AnalyticsTimeRange::AllTime:   return "AllTime";
        case AnalyticsTimeRange::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class AnalyticsChartType : uint8_t {
    Line, Bar, Pie, Scatter, Heatmap, Funnel
};

inline const char* analyticsChartTypeName(AnalyticsChartType c) {
    switch (c) {
        case AnalyticsChartType::Line:    return "Line";
        case AnalyticsChartType::Bar:     return "Bar";
        case AnalyticsChartType::Pie:     return "Pie";
        case AnalyticsChartType::Scatter: return "Scatter";
        case AnalyticsChartType::Heatmap: return "Heatmap";
        case AnalyticsChartType::Funnel:  return "Funnel";
    }
    return "Unknown";
}

class AnalyticsWidget {
public:
    explicit AnalyticsWidget(uint32_t id, const std::string& name, AnalyticsMetricType metricType)
        : m_id(id), m_name(name), m_metricType(metricType) {}

    void setChartType(AnalyticsChartType v)  { m_chartType  = v; }
    void setTimeRange(AnalyticsTimeRange v)  { m_timeRange  = v; }
    void setIsVisible(bool v)               { m_isVisible  = v; }
    void setRefreshHz(float v)              { m_refreshHz  = v; }

    [[nodiscard]] uint32_t              id()          const { return m_id;          }
    [[nodiscard]] const std::string&    name()        const { return m_name;        }
    [[nodiscard]] AnalyticsMetricType   metricType()  const { return m_metricType;  }
    [[nodiscard]] AnalyticsChartType    chartType()   const { return m_chartType;   }
    [[nodiscard]] AnalyticsTimeRange    timeRange()   const { return m_timeRange;   }
    [[nodiscard]] bool                  isVisible()   const { return m_isVisible;   }
    [[nodiscard]] float                 refreshHz()   const { return m_refreshHz;   }

private:
    uint32_t             m_id;
    std::string          m_name;
    AnalyticsMetricType  m_metricType;
    AnalyticsChartType   m_chartType  = AnalyticsChartType::Line;
    AnalyticsTimeRange   m_timeRange  = AnalyticsTimeRange::LastDay;
    bool                 m_isVisible  = true;
    float                m_refreshHz  = 1.0f;
};

class AnalyticsDashboard {
public:
    void setActiveTimeRange(AnalyticsTimeRange v)  { m_activeTimeRange = v; }
    void setShowGrid(bool v)                       { m_showGrid        = v; }
    void setAutoRefresh(bool v)                    { m_autoRefresh     = v; }
    void setRefreshIntervalSec(float v)            { m_refreshIntervalSec = v; }

    bool addWidget(const AnalyticsWidget& w) {
        for (auto& e : m_widgets) if (e.id() == w.id()) return false;
        m_widgets.push_back(w); return true;
    }
    bool removeWidget(uint32_t id) {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [&](const AnalyticsWidget& e){ return e.id() == id; });
        if (it == m_widgets.end()) return false;
        m_widgets.erase(it); return true;
    }
    [[nodiscard]] AnalyticsWidget* findWidget(uint32_t id) {
        for (auto& e : m_widgets) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] AnalyticsTimeRange activeTimeRange()     const { return m_activeTimeRange;    }
    [[nodiscard]] bool               isShowGrid()          const { return m_showGrid;           }
    [[nodiscard]] bool               isAutoRefresh()       const { return m_autoRefresh;        }
    [[nodiscard]] float              refreshIntervalSec()  const { return m_refreshIntervalSec; }
    [[nodiscard]] size_t             widgetCount()         const { return m_widgets.size();     }

    [[nodiscard]] size_t countByMetricType(AnalyticsMetricType t) const {
        size_t n = 0; for (auto& e : m_widgets) if (e.metricType() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByChartType(AnalyticsChartType c) const {
        size_t n = 0; for (auto& e : m_widgets) if (e.chartType() == c) ++n; return n;
    }
    [[nodiscard]] size_t countVisible() const {
        size_t n = 0; for (auto& e : m_widgets) if (e.isVisible()) ++n; return n;
    }

private:
    std::vector<AnalyticsWidget> m_widgets;
    AnalyticsTimeRange m_activeTimeRange    = AnalyticsTimeRange::LastDay;
    bool               m_showGrid          = true;
    bool               m_autoRefresh       = true;
    float              m_refreshIntervalSec = 30.0f;
};

} // namespace NF
