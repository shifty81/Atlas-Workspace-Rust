use std::collections::HashMap;
use thiserror::Error;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Opcode {
    PushInt = 0,
    PushFloat,
    PushString,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Neg,
    Eq,
    Neq,
    Lt,
    Gt,
    Lte,
    Gte,
    And,
    Or,
    Not,
    LoadVar,
    StoreVar,
    Call,
    Jmp,
    JmpIf,
    Ret,
    Halt,
}

#[derive(Debug, Clone)]
pub struct Instruction {
    pub op: Opcode,
    /// For CALL: bits 0-15 = constant pool index of fn name, bits 16-23 = arg count
    pub operand: i32,
}

impl Instruction {
    pub fn new(op: Opcode, operand: i32) -> Self {
        Self { op, operand }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum ScriptValue {
    None,
    Int(i64),
    Float(f64),
    Bool(bool),
    String(String),
}

impl ScriptValue {
    pub fn is_truthy(&self) -> bool {
        match self {
            Self::None => false,
            Self::Int(i) => *i != 0,
            Self::Float(f) => *f != 0.0,
            Self::Bool(b) => *b,
            Self::String(s) => !s.is_empty(),
        }
    }

    pub fn as_str(&self) -> Option<&str> {
        if let Self::String(s) = self { Some(s) } else { None }
    }
}

#[derive(Debug, Clone)]
pub struct CompiledScript {
    pub name: String,
    pub version: u32,
    pub code: Vec<Instruction>,
    pub constants: Vec<ScriptValue>,
    pub source_hash: u64,
    pub deterministic_declared: bool,
    pub replay_safe: bool,
    pub migration_safe: bool,
}

impl CompiledScript {
    pub fn new(name: impl Into<String>) -> Self {
        Self {
            name: name.into(),
            version: 1,
            code: Vec::new(),
            constants: Vec::new(),
            source_hash: 0,
            deterministic_declared: false,
            replay_safe: false,
            migration_safe: false,
        }
    }
}

#[derive(Debug, Error)]
pub enum ScriptVmError {
    #[error("division by zero")]
    DivisionByZero,
    #[error("stack underflow")]
    StackUnderflow,
    #[error("undefined variable: {0}")]
    UndefinedVariable(String),
    #[error("undefined function: {0}")]
    UndefinedFunction(String),
    #[error("type mismatch")]
    TypeMismatch,
    #[error("budget exceeded")]
    BudgetExceeded,
    #[error("invalid instruction")]
    InvalidInstruction,
}

type NativeFn = Box<dyn Fn(&[ScriptValue]) -> ScriptValue + Send + Sync>;

pub struct ScriptVM {
    variables: HashMap<String, ScriptValue>,
    functions: HashMap<String, NativeFn>,
    stack: Vec<ScriptValue>,
    step_count: u64,
    max_steps: u64,
    budget_exceeded: bool,
}

impl Default for ScriptVM {
    fn default() -> Self {
        Self::new()
    }
}

impl ScriptVM {
    pub fn new() -> Self {
        Self {
            variables: HashMap::new(),
            functions: HashMap::new(),
            stack: Vec::new(),
            step_count: 0,
            max_steps: 1_000_000,
            budget_exceeded: false,
        }
    }

    pub fn set_variable(&mut self, name: &str, value: ScriptValue) {
        self.variables.insert(name.to_owned(), value);
    }

    pub fn get_variable(&self, name: &str) -> Option<&ScriptValue> {
        self.variables.get(name)
    }

    pub fn register_function(
        &mut self,
        name: &str,
        f: impl Fn(&[ScriptValue]) -> ScriptValue + Send + Sync + 'static,
    ) {
        self.functions.insert(name.to_owned(), Box::new(f));
    }

    pub fn reset(&mut self) {
        self.stack.clear();
        self.step_count = 0;
        self.budget_exceeded = false;
        self.variables.clear();
    }

    pub fn step_count(&self) -> u64 {
        self.step_count
    }

    pub fn set_max_steps(&mut self, n: u64) {
        self.max_steps = n;
    }

    pub fn was_budget_exceeded(&self) -> bool {
        self.budget_exceeded
    }

    pub fn state_hash(&self) -> u64 {
        // FNV-1a over variable names+values
        let mut hash: u64 = 14695981039346656037;
        for (k, v) in &self.variables {
            for b in k.bytes() {
                hash ^= b as u64;
                hash = hash.wrapping_mul(1099511628211);
            }
            let vbytes = format!("{:?}", v);
            for b in vbytes.bytes() {
                hash ^= b as u64;
                hash = hash.wrapping_mul(1099511628211);
            }
        }
        hash
    }

    pub fn execute(&mut self, script: &CompiledScript) -> Result<ScriptValue, ScriptVmError> {
        self.stack.clear();
        let mut pc: usize = 0;
        let code = &script.code;

        while pc < code.len() {
            if self.step_count >= self.max_steps {
                self.budget_exceeded = true;
                return Err(ScriptVmError::BudgetExceeded);
            }
            self.step_count += 1;

            let instr = &code[pc];
            pc += 1;

            match instr.op {
                Opcode::PushInt => {
                    let idx = instr.operand as usize;
                    let val = script.constants.get(idx).cloned().unwrap_or(ScriptValue::None);
                    self.stack.push(val);
                }
                Opcode::PushFloat => {
                    let idx = instr.operand as usize;
                    let val = script.constants.get(idx).cloned().unwrap_or(ScriptValue::None);
                    self.stack.push(val);
                }
                Opcode::PushString => {
                    let idx = instr.operand as usize;
                    let val = script.constants.get(idx).cloned().unwrap_or(ScriptValue::None);
                    self.stack.push(val);
                }
                Opcode::Add => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => ScriptValue::Int(x + y),
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => ScriptValue::Float(x + y),
                        (ScriptValue::Int(x), ScriptValue::Float(y)) => ScriptValue::Float(x as f64 + y),
                        (ScriptValue::Float(x), ScriptValue::Int(y)) => ScriptValue::Float(x + y as f64),
                        (ScriptValue::String(x), ScriptValue::String(y)) => ScriptValue::String(x + &y),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Sub => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => ScriptValue::Int(x - y),
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => ScriptValue::Float(x - y),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Mul => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => ScriptValue::Int(x * y),
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => ScriptValue::Float(x * y),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Div => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => {
                            if y == 0 { return Err(ScriptVmError::DivisionByZero); }
                            ScriptValue::Int(x / y)
                        }
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => {
                            if y == 0.0 { return Err(ScriptVmError::DivisionByZero); }
                            ScriptValue::Float(x / y)
                        }
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Mod => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => {
                            if y == 0 { return Err(ScriptVmError::DivisionByZero); }
                            ScriptValue::Int(x % y)
                        }
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Neg => {
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match a {
                        ScriptValue::Int(x) => ScriptValue::Int(-x),
                        ScriptValue::Float(x) => ScriptValue::Float(-x),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Eq => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    self.stack.push(ScriptValue::Bool(a == b));
                }
                Opcode::Neq => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    self.stack.push(ScriptValue::Bool(a != b));
                }
                Opcode::Lt => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => ScriptValue::Bool(x < y),
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => ScriptValue::Bool(x < y),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Gt => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => ScriptValue::Bool(x > y),
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => ScriptValue::Bool(x > y),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Lte => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => ScriptValue::Bool(x <= y),
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => ScriptValue::Bool(x <= y),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::Gte => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let result = match (a, b) {
                        (ScriptValue::Int(x), ScriptValue::Int(y)) => ScriptValue::Bool(x >= y),
                        (ScriptValue::Float(x), ScriptValue::Float(y)) => ScriptValue::Bool(x >= y),
                        _ => return Err(ScriptVmError::TypeMismatch),
                    };
                    self.stack.push(result);
                }
                Opcode::And => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    self.stack.push(ScriptValue::Bool(a.is_truthy() && b.is_truthy()));
                }
                Opcode::Or => {
                    let b = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    self.stack.push(ScriptValue::Bool(a.is_truthy() || b.is_truthy()));
                }
                Opcode::Not => {
                    let a = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    self.stack.push(ScriptValue::Bool(!a.is_truthy()));
                }
                Opcode::LoadVar => {
                    let idx = instr.operand as usize;
                    let name = script.constants.get(idx)
                        .and_then(|v| v.as_str())
                        .ok_or(ScriptVmError::InvalidInstruction)?
                        .to_owned();
                    let val = self.variables.get(&name)
                        .cloned()
                        .ok_or_else(|| ScriptVmError::UndefinedVariable(name))?;
                    self.stack.push(val);
                }
                Opcode::StoreVar => {
                    let idx = instr.operand as usize;
                    let name = script.constants.get(idx)
                        .and_then(|v| v.as_str())
                        .ok_or(ScriptVmError::InvalidInstruction)?
                        .to_owned();
                    let val = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    self.variables.insert(name, val);
                }
                Opcode::Call => {
                    let fn_idx = (instr.operand & 0xFFFF) as usize;
                    let arg_count = ((instr.operand >> 16) & 0xFF) as usize;
                    let fn_name = script.constants.get(fn_idx)
                        .and_then(|v| v.as_str())
                        .ok_or(ScriptVmError::InvalidInstruction)?
                        .to_owned();
                    let stack_len = self.stack.len();
                    if stack_len < arg_count {
                        return Err(ScriptVmError::StackUnderflow);
                    }
                    let args: Vec<ScriptValue> = self.stack.drain(stack_len - arg_count..).collect();
                    let func = self.functions.get(&fn_name)
                        .ok_or_else(|| ScriptVmError::UndefinedFunction(fn_name))?;
                    let result = func(&args);
                    self.stack.push(result);
                }
                Opcode::Jmp => {
                    pc = instr.operand as usize;
                }
                Opcode::JmpIf => {
                    let cond = self.stack.pop().ok_or(ScriptVmError::StackUnderflow)?;
                    if cond.is_truthy() {
                        pc = instr.operand as usize;
                    }
                }
                Opcode::Ret | Opcode::Halt => {
                    break;
                }
            }
        }

        Ok(self.stack.pop().unwrap_or(ScriptValue::None))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_script(name: &str, code: Vec<Instruction>, constants: Vec<ScriptValue>) -> CompiledScript {
        let mut s = CompiledScript::new(name);
        s.code = code;
        s.constants = constants;
        s
    }

    #[test]
    fn push_int_halt() {
        let mut vm = ScriptVM::new();
        let script = make_script("test", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::Int(42)]);
        let result = vm.execute(&script).unwrap();
        assert_eq!(result, ScriptValue::Int(42));
    }

    #[test]
    fn arithmetic_int() {
        let mut vm = ScriptVM::new();
        // 10 + 3 = 13
        let script = make_script("arith", vec![
            Instruction::new(Opcode::PushInt, 0),   // 10
            Instruction::new(Opcode::PushInt, 1),   // 3
            Instruction::new(Opcode::Add, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::Int(10), ScriptValue::Int(3)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(13));
    }

    #[test]
    fn arithmetic_sub_mul_div() {
        let mut vm = ScriptVM::new();
        // 20 - 5 = 15
        let script = make_script("sub", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::PushInt, 1),
            Instruction::new(Opcode::Sub, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::Int(20), ScriptValue::Int(5)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(15));

        vm.reset();
        // 4 * 3 = 12
        let script = make_script("mul", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::PushInt, 1),
            Instruction::new(Opcode::Mul, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::Int(4), ScriptValue::Int(3)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(12));

        vm.reset();
        // 10 / 2 = 5
        let script = make_script("div", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::PushInt, 1),
            Instruction::new(Opcode::Div, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::Int(10), ScriptValue::Int(2)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(5));
    }

    #[test]
    fn division_by_zero_error() {
        let mut vm = ScriptVM::new();
        let script = make_script("divzero", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::PushInt, 1),
            Instruction::new(Opcode::Div, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::Int(5), ScriptValue::Int(0)]);
        assert!(matches!(vm.execute(&script), Err(ScriptVmError::DivisionByZero)));
    }

    #[test]
    fn comparison_and_bool() {
        let mut vm = ScriptVM::new();
        // 3 < 5 → true
        let script = make_script("lt", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::PushInt, 1),
            Instruction::new(Opcode::Lt, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::Int(3), ScriptValue::Int(5)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Bool(true));
    }

    #[test]
    fn conditional_jump() {
        let mut vm = ScriptVM::new();
        // if true jump to idx 3 (PushInt 99), else PushInt 0 → result should be 99
        let script = make_script("jmpif", vec![
            Instruction::new(Opcode::PushInt,   0),  // 0: push true
            Instruction::new(Opcode::JmpIf,      3),  // 1: if truthy jump to 3
            Instruction::new(Opcode::PushInt,    1),  // 2: push 0 (never reached)
            Instruction::new(Opcode::PushInt,    2),  // 3: push 99
            Instruction::new(Opcode::Halt,        0),  // 4: halt
        ], vec![ScriptValue::Bool(true), ScriptValue::Int(0), ScriptValue::Int(99)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(99));
    }

    #[test]
    fn store_load_variable() {
        let mut vm = ScriptVM::new();
        // StoreVar "x" = 7, then LoadVar "x"
        let script = make_script("var", vec![
            Instruction::new(Opcode::PushInt,  0),  // push 7
            Instruction::new(Opcode::StoreVar, 1),  // store as "x"
            Instruction::new(Opcode::LoadVar,  1),  // load "x"
            Instruction::new(Opcode::Halt,      0),
        ], vec![ScriptValue::Int(7), ScriptValue::String("x".into())]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(7));
    }

    #[test]
    fn native_function_call() {
        let mut vm = ScriptVM::new();
        vm.register_function("double", |args| {
            match args.first() {
                Some(ScriptValue::Int(n)) => ScriptValue::Int(n * 2),
                _ => ScriptValue::None,
            }
        });
        // call "double"(5) → 10
        // CALL operand: fn_idx=0 in lower 16 bits, arg_count=1 in next 8 bits
        let arg_count: i32 = 1 << 16;
        let script = make_script("call", vec![
            Instruction::new(Opcode::PushInt,  1),       // push 5
            Instruction::new(Opcode::Call,     arg_count | 0), // call "double" with 1 arg
            Instruction::new(Opcode::Halt,     0),
        ], vec![ScriptValue::String("double".into()), ScriptValue::Int(5)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(10));
    }

    #[test]
    fn budget_exceeded() {
        let mut vm = ScriptVM::new();
        vm.set_max_steps(5);
        // Infinite loop via unconditional jump to 0
        let script = make_script("loop", vec![
            Instruction::new(Opcode::Jmp, 0),
        ], vec![]);
        assert!(matches!(vm.execute(&script), Err(ScriptVmError::BudgetExceeded)));
        assert!(vm.was_budget_exceeded());
    }

    #[test]
    fn string_concat() {
        let mut vm = ScriptVM::new();
        let script = make_script("strcat", vec![
            Instruction::new(Opcode::PushString, 0),
            Instruction::new(Opcode::PushString, 1),
            Instruction::new(Opcode::Add, 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::String("Hello, ".into()), ScriptValue::String("World!".into())]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::String("Hello, World!".into()));
    }

    #[test]
    fn state_hash_deterministic() {
        let mut vm = ScriptVM::new();
        vm.set_variable("x", ScriptValue::Int(42));
        let h1 = vm.state_hash();
        let h2 = vm.state_hash();
        assert_eq!(h1, h2);
    }

    #[test]
    fn negation() {
        let mut vm = ScriptVM::new();
        let script = make_script("neg", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::Neg,     0),
            Instruction::new(Opcode::Halt,    0),
        ], vec![ScriptValue::Int(7)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(-7));
    }

    #[test]
    fn sandbox_builtins() {
        use crate::sandbox::ScriptSandbox;
        let mut vm = ScriptVM::new();
        ScriptSandbox::register_builtins(&mut vm);
        // atlas_abs(-5) → 5
        let arg_count: i32 = 1 << 16;
        let script = make_script("abs", vec![
            Instruction::new(Opcode::PushInt, 1),
            Instruction::new(Opcode::Call, arg_count | 0),
            Instruction::new(Opcode::Halt, 0),
        ], vec![ScriptValue::String("atlas_abs".into()), ScriptValue::Int(-5)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(5));
    }

    #[test]
    fn modulo() {
        let mut vm = ScriptVM::new();
        // 10 % 3 = 1
        let script = make_script("mod", vec![
            Instruction::new(Opcode::PushInt, 0),
            Instruction::new(Opcode::PushInt, 1),
            Instruction::new(Opcode::Mod,     0),
            Instruction::new(Opcode::Halt,    0),
        ], vec![ScriptValue::Int(10), ScriptValue::Int(3)]);
        assert_eq!(vm.execute(&script).unwrap(), ScriptValue::Int(1));
    }
}
