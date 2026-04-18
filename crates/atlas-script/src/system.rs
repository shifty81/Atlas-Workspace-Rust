use crate::vm::{CompiledScript, ScriptValue, ScriptVM};

#[derive(Debug, Clone)]
pub struct ScriptContract {
    pub script_name: String,
    pub deterministic: bool,
    pub replay_safe: bool,
    pub migration_safe: bool,
}

pub struct ScriptSystem {
    vm: ScriptVM,
    scripts: Vec<CompiledScript>,
    validation_errors: Vec<String>,
    total_steps_this_tick: u64,
}

impl Default for ScriptSystem {
    fn default() -> Self {
        Self::new()
    }
}

impl ScriptSystem {
    pub fn new() -> Self {
        Self {
            vm: ScriptVM::new(),
            scripts: Vec::new(),
            validation_errors: Vec::new(),
            total_steps_this_tick: 0,
        }
    }

    pub fn register_script(&mut self, script: CompiledScript) {
        self.scripts.retain(|s| s.name != script.name);
        self.scripts.push(script);
    }

    pub fn unregister_script(&mut self, name: &str) {
        self.scripts.retain(|s| s.name != name);
    }

    pub fn execute_tick(&mut self, tick: u64, seed: u64) {
        self.vm.set_variable("tick", ScriptValue::Int(tick as i64));
        self.vm.set_variable("seed", ScriptValue::Int(seed as i64));
        self.total_steps_this_tick = 0;
        let scripts = self.scripts.clone();
        for script in &scripts {
            let _ = self.vm.execute(script);
            self.total_steps_this_tick += self.vm.step_count();
        }
    }

    pub fn vm(&self) -> &ScriptVM {
        &self.vm
    }

    pub fn vm_mut(&mut self) -> &mut ScriptVM {
        &mut self.vm
    }

    pub fn validate_contracts(&mut self) -> bool {
        self.validation_errors.clear();
        for script in &self.scripts {
            if !script.deterministic_declared {
                self.validation_errors.push(format!("{}: not declared deterministic", script.name));
            }
        }
        self.validation_errors.is_empty()
    }

    pub fn validation_errors(&self) -> &[String] {
        &self.validation_errors
    }

    pub fn registered_scripts(&self) -> Vec<&str> {
        self.scripts.iter().map(|s| s.name.as_str()).collect()
    }

    pub fn script_count(&self) -> usize {
        self.scripts.len()
    }

    pub fn total_steps_this_tick(&self) -> u64 {
        self.total_steps_this_tick
    }

    pub fn combined_hash(&self) -> u64 {
        self.vm.state_hash()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::vm::{CompiledScript, Instruction, Opcode, ScriptValue};

    fn make_script(name: &str, deterministic: bool) -> CompiledScript {
        let mut s = CompiledScript::new(name);
        s.code = vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::Halt, 0),
        ];
        s.constants = vec![ScriptValue::Int(1)];
        s.deterministic_declared = deterministic;
        s.replay_safe = deterministic;
        s
    }

    #[test]
    fn register_and_count() {
        let mut sys = ScriptSystem::new();
        sys.register_script(make_script("alpha", true));
        sys.register_script(make_script("beta", true));
        assert_eq!(sys.script_count(), 2);
        let names = sys.registered_scripts();
        assert!(names.contains(&"alpha"));
        assert!(names.contains(&"beta"));
    }

    #[test]
    fn register_replaces_same_name() {
        let mut sys = ScriptSystem::new();
        sys.register_script(make_script("a", true));
        sys.register_script(make_script("a", false));
        assert_eq!(sys.script_count(), 1);
        // Validation should fail because deterministic_declared = false
        assert!(!sys.validate_contracts());
    }

    #[test]
    fn unregister_removes_script() {
        let mut sys = ScriptSystem::new();
        sys.register_script(make_script("a", true));
        sys.unregister_script("a");
        assert_eq!(sys.script_count(), 0);
    }

    #[test]
    fn execute_tick_sets_variables() {
        let mut sys = ScriptSystem::new();
        sys.register_script(make_script("s", true));
        sys.execute_tick(42, 99);
        let tick = sys.vm().get_variable("tick");
        let seed = sys.vm().get_variable("seed");
        assert_eq!(tick, Some(&ScriptValue::Int(42)));
        assert_eq!(seed, Some(&ScriptValue::Int(99)));
    }

    #[test]
    fn validate_contracts_pass() {
        let mut sys = ScriptSystem::new();
        sys.register_script(make_script("x", true));
        assert!(sys.validate_contracts());
        assert!(sys.validation_errors().is_empty());
    }

    #[test]
    fn validate_contracts_fail_non_deterministic() {
        let mut sys = ScriptSystem::new();
        sys.register_script(make_script("bad", false));
        assert!(!sys.validate_contracts());
        assert!(!sys.validation_errors().is_empty());
    }

    #[test]
    fn combined_hash_changes_after_variable_set() {
        let mut sys = ScriptSystem::new();
        let h1 = sys.combined_hash();
        sys.execute_tick(1, 0);
        let h2 = sys.combined_hash();
        // After setting tick=1 the VM state should be different
        assert_ne!(h1, h2);
    }

    #[test]
    fn total_steps_tracked() {
        let mut sys = ScriptSystem::new();
        sys.register_script(make_script("s", true));
        sys.execute_tick(1, 0);
        // Script has 2 instructions (PushInt + Halt), so at least 1 step counted
        assert!(sys.total_steps_this_tick() > 0);
    }
}
