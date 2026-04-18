#pragma once
// NF::Editor — Analytics dashboard v1: dashboard widgets, charts, metric displays
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Adv1WidgetType        : uint8_t { Counter, LineChart, BarChart, PieChart, Heatmap, Table };
enum class Adv1MetricAggregation : uint8_t { Sum, Avg, Min, Max, Count, P95 };

inline const char* adv1WidgetTypeName(Adv1WidgetType t) {
    switch (t) {
        case Adv1WidgetType::Counter:   return "Counter";
        case Adv1WidgetType::LineChart: return "LineChart";
        case Adv1WidgetType::BarChart:  return "BarChart";
        case Adv1WidgetType::PieChart:  return "PieChart";
        case Adv1WidgetType::Heatmap:   return "Heatmap";
        case Adv1WidgetType::Table:     return "Table";
    }
    return "Unknown";
}

inline const char* adv1MetricAggregationName(Adv1MetricAggregation a) {
    switch (a) {
        case Adv1MetricAggregation::Sum:   return "Sum";
        case Adv1MetricAggregation::Avg:   return "Avg";
        case Adv1MetricAggregation::Min:   return "Min";
        case Adv1MetricAggregation::Max:   return "Max";
        case Adv1MetricAggregation::Count: return "Count";
        case Adv1MetricAggregation::P95:   return "P95";
    }
    return "Unknown";
}

struct Adv1Metric {
    uint64_t              id          = 0;
    std::string           name;
    Adv1MetricAggregation aggregation = Adv1MetricAggregation::Sum;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Adv1Widget {
    uint64_t        id    = 0;
    std::string     name;
    Adv1WidgetType  type  = Adv1WidgetType::Counter;
    bool            visible = true;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isVisible() const { return visible; }
};

using Adv1RefreshCallback = std::function<void(uint64_t)>;

class AnalyticsDashboardV1 {
public:
    static constexpr size_t MAX_WIDGETS = 128;
    static constexpr size_t MAX_METRICS = 512;

    bool addWidget(const Adv1Widget& widget) {
        if (!widget.isValid()) return false;
        for (const auto& w : m_widgets) if (w.id == widget.id) return false;
        if (m_widgets.size() >= MAX_WIDGETS) return false;
        m_widgets.push_back(widget);
        return true;
    }

    bool removeWidget(uint64_t id) {
        for (auto it = m_widgets.begin(); it != m_widgets.end(); ++it) {
            if (it->id == id) { m_widgets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Adv1Widget* findWidget(uint64_t id) {
        for (auto& w : m_widgets) if (w.id == id) return &w;
        return nullptr;
    }

    bool addMetric(const Adv1Metric& metric) {
        if (!metric.isValid()) return false;
        for (const auto& m : m_metrics) if (m.id == metric.id) return false;
        if (m_metrics.size() >= MAX_METRICS) return false;
        m_metrics.push_back(metric);
        return true;
    }

    bool removeMetric(uint64_t id) {
        for (auto it = m_metrics.begin(); it != m_metrics.end(); ++it) {
            if (it->id == id) { m_metrics.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Adv1Metric* findMetric(uint64_t id) {
        for (auto& m : m_metrics) if (m.id == id) return &m;
        return nullptr;
    }

    bool setVisible(uint64_t id, bool visible) {
        auto* w = findWidget(id);
        if (!w) return false;
        w->visible = visible;
        if (m_onRefresh) m_onRefresh(id);
        return true;
    }

    [[nodiscard]] size_t widgetCount()  const { return m_widgets.size(); }
    [[nodiscard]] size_t metricCount()  const { return m_metrics.size(); }
    [[nodiscard]] size_t countByWidgetType(Adv1WidgetType type) const {
        size_t c = 0; for (const auto& w : m_widgets) if (w.type == type) ++c; return c;
    }
    [[nodiscard]] size_t countByAggregation(Adv1MetricAggregation agg) const {
        size_t c = 0; for (const auto& m : m_metrics) if (m.aggregation == agg) ++c; return c;
    }

    void setOnRefresh(Adv1RefreshCallback cb) { m_onRefresh = std::move(cb); }

private:
    std::vector<Adv1Widget> m_widgets;
    std::vector<Adv1Metric> m_metrics;
    Adv1RefreshCallback     m_onRefresh;
};

} // namespace NF
