#include "ConstraintSolver.h"
#include <algorithm>
#include <numeric>

namespace atlas::procedural {

ConstraintSolver::ConstraintSolver(uint64_t seed)
    : m_rng(seed) {}

void ConstraintSolver::AddItem(const FitItem& item) {
    m_items.push_back(item);
}

void ConstraintSolver::ClearItems() {
    m_items.clear();
}

// ---- GA internals ----

ConstraintSolver::Chromosome
ConstraintSolver::RandomChromosome(size_t len, const ConstraintConfig& cfg) {
    Chromosome chr(len, false);
    // Randomly enable items, respecting the max-items heuristic.
    int count = 0;
    for (size_t i = 0; i < len; ++i) {
        if (count < cfg.maxItems && m_rng.NextBool(0.4f)) {
            chr[i] = true;
            ++count;
        }
    }
    return chr;
}

float ConstraintSolver::Fitness(const Chromosome& chr,
                                const ConstraintConfig& cfg) const {
    float totalCost  = 0;
    float totalValue = 0;
    int   count      = 0;
    size_t limit = std::min(chr.size(), m_items.size());
    for (size_t i = 0; i < limit; ++i) {
        if (chr[i]) {
            totalCost  += m_items[i].cost;
            totalValue += m_items[i].value;
            ++count;
        }
    }
    // Penalise infeasible solutions.
    if (totalCost > cfg.maxCapacity || count > cfg.maxItems) {
        float overCost = (totalCost > cfg.maxCapacity)
                         ? (totalCost - cfg.maxCapacity) : 0;
        int overCount  = (count > cfg.maxItems)
                         ? (count - cfg.maxItems) : 0;
        return totalValue - overCost * 10.0f - static_cast<float>(overCount) * 50.0f;
    }
    return totalValue;
}

ConstraintResult ConstraintSolver::Decode(const Chromosome& chr) const {
    ConstraintResult res;
    size_t limit = std::min(chr.size(), m_items.size());
    for (size_t i = 0; i < limit; ++i) {
        if (chr[i]) {
            res.selectedIndices.push_back(i);
            res.totalCost  += m_items[i].cost;
            res.totalValue += m_items[i].value;
            ++res.itemCount;
        }
    }
    return res;
}

ConstraintSolver::Chromosome
ConstraintSolver::Crossover(const Chromosome& a, const Chromosome& b) {
    if (a.empty() || a.size() != b.size()) return a;
    size_t point = static_cast<size_t>(m_rng.NextU32(static_cast<uint32_t>(a.size())));
    Chromosome child(a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        child[i] = (i < point) ? a[i] : b[i];
    }
    return child;
}

void ConstraintSolver::Mutate(Chromosome& chr, float rate) {
    for (size_t i = 0; i < chr.size(); ++i) {
        if (m_rng.NextFloat() < rate) {
            chr[i] = !chr[i];
        }
    }
}

// ---- Public solve ----

ConstraintResult ConstraintSolver::Solve(const ConstraintConfig& config) {
    if (m_items.empty()) {
        return ConstraintResult{};
    }

    size_t len = m_items.size();
    int popSize = std::max(2, config.populationSize);
    int gens = std::max(1, config.generations);

    // Initialise population.
    std::vector<Chromosome> population(static_cast<size_t>(popSize));
    for (auto& chr : population) {
        chr = RandomChromosome(len, config);
    }

    // Evolution loop.
    for (int g = 0; g < gens; ++g) {
        // Evaluate fitness.
        std::vector<float> fitnesses(population.size());
        for (size_t i = 0; i < population.size(); ++i) {
            fitnesses[i] = Fitness(population[i], config);
        }

        // Tournament selection + breed next generation.
        std::vector<Chromosome> next;
        next.reserve(population.size());
        while (next.size() < population.size()) {
            // Tournament: pick 2 random, keep better.
            size_t a = m_rng.NextU32(static_cast<uint32_t>(population.size()));
            size_t b = m_rng.NextU32(static_cast<uint32_t>(population.size()));
            size_t c = m_rng.NextU32(static_cast<uint32_t>(population.size()));
            size_t d = m_rng.NextU32(static_cast<uint32_t>(population.size()));
            size_t p1 = (fitnesses[a] >= fitnesses[b]) ? a : b;
            size_t p2 = (fitnesses[c] >= fitnesses[d]) ? c : d;

            Chromosome child;
            if (m_rng.NextFloat() < config.crossoverRate) {
                child = Crossover(population[p1], population[p2]);
            } else {
                child = population[p1];
            }
            Mutate(child, config.mutationRate);
            next.push_back(std::move(child));
        }
        population = std::move(next);
    }

    // Find best individual.
    float bestFit = -1e30f;
    size_t bestIdx = 0;
    for (size_t i = 0; i < population.size(); ++i) {
        float f = Fitness(population[i], config);
        if (f > bestFit) {
            bestFit = f;
            bestIdx = i;
        }
    }

    auto result = Decode(population[bestIdx]);
    result.feasible = IsFeasible(result, config);
    return result;
}

bool ConstraintSolver::IsFeasible(const ConstraintResult& result,
                                  const ConstraintConfig& config) {
    return result.totalCost <= config.maxCapacity &&
           result.itemCount <= config.maxItems;
}

} // namespace atlas::procedural
