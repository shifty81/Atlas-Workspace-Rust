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
