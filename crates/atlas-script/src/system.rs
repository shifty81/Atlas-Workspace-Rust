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
