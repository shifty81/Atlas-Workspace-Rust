//! Genetic-algorithm constraint / fitting solver.
//!
//! Faithful Rust port of the C++ `atlas::procedural::ConstraintSolver`.
//! Used to generate valid ship loadouts, module configurations, and other
//! fitting problems where items must respect a capacity budget.

use crate::rng::DeterministicRng;

/// A single item to be fitted by the solver.
#[derive(Clone, Debug)]
pub struct FitItem {
    pub name:  String,
    /// Capacity cost (e.g. power-grid / CPU usage).
    pub cost:  f32,
    /// Fitness contribution.
    pub value: f32,
    /// Optional group tag for mutual exclusion (`-1` = no group).
    pub group: i32,
}

/// Configuration for a constraint-solving run.
#[derive(Clone, Debug)]
pub struct ConstraintConfig {
    pub max_capacity:    f32,
    pub max_items:       i32,
    pub population_size: i32,
    pub generations:     i32,
    pub mutation_rate:   f32,
    pub crossover_rate:  f32,
}

impl Default for ConstraintConfig {
    fn default() -> Self {
        Self {
            max_capacity:    100.0,
            max_items:       10,
            population_size: 50,
            generations:     100,
            mutation_rate:   0.1,
            crossover_rate:  0.7,
        }
    }
}

/// Result of a solver run.
#[derive(Clone, Debug, Default)]
pub struct ConstraintResult {
    pub selected_indices: Vec<usize>,
    pub total_cost:   f32,
    pub total_value:  f32,
    pub item_count:   i32,
    pub feasible:     bool,
}

type Chromosome = Vec<bool>;

/// Deterministic genetic-algorithm constraint solver.
pub struct ConstraintSolver {
    rng:   DeterministicRng,
    items: Vec<FitItem>,
}

impl ConstraintSolver {
    pub fn new(seed: u64) -> Self {
        Self { rng: DeterministicRng::new(seed), items: Vec::new() }
    }

    pub fn add_item(&mut self, item: FitItem) {
        self.items.push(item);
    }

    pub fn clear_items(&mut self) {
        self.items.clear();
    }

    pub fn item_count(&self) -> usize {
        self.items.len()
    }

    /// Run the GA solver.  Returns the best feasible chromosome found, or the
    /// best infeasible one if no feasible solution exists.
    pub fn solve(&mut self, config: &ConstraintConfig) -> ConstraintResult {
        let n = self.items.len();
        if n == 0 {
            return ConstraintResult::default();
        }

        let pop_size = config.population_size.max(2) as usize;
        let mut population: Vec<Chromosome> = (0..pop_size)
            .map(|_| self.random_chromosome(n, config))
            .collect();

        let mut best = population[0].clone();
        let mut best_fitness = self.fitness(&best, config);

        for _ in 0..config.generations {
            // Sort by descending fitness
            let mut scored: Vec<(f32, Chromosome)> = population
                .drain(..)
                .map(|c| {
                    let f = self.fitness(&c, config);
                    (f, c)
                })
                .collect();
            scored.sort_by(|a, b| b.0.partial_cmp(&a.0).unwrap_or(std::cmp::Ordering::Equal));

            // Update best
            if scored[0].0 > best_fitness {
                best_fitness = scored[0].0;
                best = scored[0].1.clone();
            }

            // Elitism: carry top half, breed bottom half
            let elite: Vec<Chromosome> = scored.iter().take(pop_size / 2).map(|(_, c)| c.clone()).collect();
            let mut next_gen = elite.clone();

            while next_gen.len() < pop_size {
                let a = &elite[self.rng.next_u32(elite.len() as u32) as usize];
                let b = &elite[self.rng.next_u32(elite.len() as u32) as usize];
                let mut child = if self.rng.next_float() < config.crossover_rate {
                    self.crossover(a, b)
                } else {
                    a.clone()
                };
                self.mutate(&mut child, config.mutation_rate);
                next_gen.push(child);
            }

            population = next_gen;
        }

        self.decode(&best)
    }

    /// Check whether a result satisfies the constraints.
    pub fn is_feasible(result: &ConstraintResult, config: &ConstraintConfig) -> bool {
        result.total_cost <= config.max_capacity
            && result.item_count <= config.max_items
    }

    // ── Private ──────────────────────────────────────────────────────────

    fn random_chromosome(&mut self, len: usize, config: &ConstraintConfig) -> Chromosome {
        let mut chr: Chromosome = (0..len)
            .map(|_| self.rng.next_bool(0.3))
            .collect();
        // Enforce max-items constraint immediately
        let on: Vec<usize> = chr.iter().enumerate()
            .filter_map(|(i, &b)| if b { Some(i) } else { None })
            .collect();
        if on.len() as i32 > config.max_items {
            for &i in &on[config.max_items as usize..] {
                chr[i] = false;
            }
        }
        chr
    }

    fn fitness(&self, chr: &Chromosome, config: &ConstraintConfig) -> f32 {
        let result = self.decode(chr);
        if result.total_cost > config.max_capacity || result.item_count > config.max_items {
            // Penalty for infeasible solutions
            -(result.total_cost - config.max_capacity).max(0.0)
        } else {
            result.total_value
        }
    }

    fn decode(&self, chr: &Chromosome) -> ConstraintResult {
        let mut res = ConstraintResult::default();
        for (i, &active) in chr.iter().enumerate() {
            if active && i < self.items.len() {
                let item = &self.items[i];
                res.selected_indices.push(i);
                res.total_cost  += item.cost;
                res.total_value += item.value;
                res.item_count  += 1;
            }
        }
        res.feasible = res.total_cost >= 0.0; // basic sanity
        res
    }

    fn crossover(&mut self, a: &Chromosome, b: &Chromosome) -> Chromosome {
        let point = self.rng.next_u32(a.len() as u32) as usize;
        let mut child = a[..point].to_vec();
        child.extend_from_slice(&b[point..]);
        child
    }

    fn mutate(&mut self, chr: &mut Chromosome, rate: f32) {
        for gene in chr.iter_mut() {
            if self.rng.next_bool(rate) {
                *gene = !*gene;
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_solver() -> ConstraintSolver {
        let mut solver = ConstraintSolver::new(1);
        solver.add_item(FitItem { name: "Laser".into(),  cost: 25.0, value: 10.0, group: -1 });
        solver.add_item(FitItem { name: "Shield".into(), cost: 40.0, value: 15.0, group: -1 });
        solver.add_item(FitItem { name: "Engine".into(), cost: 30.0, value: 12.0, group: -1 });
        solver.add_item(FitItem { name: "Armor".into(),  cost: 20.0, value:  8.0, group: -1 });
        solver
    }

    #[test]
    fn solve_respects_capacity() {
        let mut solver = make_solver();
        let config = ConstraintConfig { max_capacity: 60.0, ..Default::default() };
        let result = solver.solve(&config);
        assert!(result.total_cost <= 60.0, "cost {} exceeded budget", result.total_cost);
    }

    #[test]
    fn solve_deterministic() {
        let config = ConstraintConfig::default();
        let r1 = make_solver().solve(&config);
        let r2 = make_solver().solve(&config);
        assert_eq!(r1.selected_indices, r2.selected_indices);
    }

    #[test]
    fn empty_items() {
        let mut solver = ConstraintSolver::new(1);
        let result = solver.solve(&ConstraintConfig::default());
        assert_eq!(result.item_count, 0);
    }

    #[test]
    fn is_feasible_check() {
        let result = ConstraintResult {
            total_cost: 50.0,
            total_value: 20.0,
            item_count: 3,
            selected_indices: vec![0, 1, 2],
            feasible: true,
        };
        let config = ConstraintConfig { max_capacity: 100.0, max_items: 5, ..Default::default() };
        assert!(ConstraintSolver::is_feasible(&result, &config));

        let config2 = ConstraintConfig { max_capacity: 40.0, ..Default::default() };
        assert!(!ConstraintSolver::is_feasible(&result, &config2));
    }
}
