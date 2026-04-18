use std::collections::HashMap;
use thiserror::Error;

pub type EntityId = u32;
pub type Value = i64;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum OpCode {
    Nop = 0,
    LoadConst,
    LoadVar,
    StoreVar,
    Add,
    Sub,
    Mul,
    Div,
    CmpEq,
    CmpLt,
    CmpGt,
    Jump,
    JumpIfFalse,
    EmitEvent,
    End,
}

impl TryFrom<u8> for OpCode {
    type Error = ();
    fn try_from(v: u8) -> Result<Self, ()> {
        match v {
            0 => Ok(Self::Nop),
            1 => Ok(Self::LoadConst),
            2 => Ok(Self::LoadVar),
            3 => Ok(Self::StoreVar),
            4 => Ok(Self::Add),
            5 => Ok(Self::Sub),
            6 => Ok(Self::Mul),
            7 => Ok(Self::Div),
            8 => Ok(Self::CmpEq),
            9 => Ok(Self::CmpLt),
            10 => Ok(Self::CmpGt),
            11 => Ok(Self::Jump),
            12 => Ok(Self::JumpIfFalse),
            13 => Ok(Self::EmitEvent),
            14 => Ok(Self::End),
            _ => Err(()),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Instruction {
    pub opcode: OpCode,
    pub a: u32,
    pub b: u32,
    pub c: u32,
}

#[derive(Debug, Clone, Default)]
pub struct Bytecode {
    pub instructions: Vec<Instruction>,
    pub constants: Vec<Value>,
}

#[derive(Debug, Clone, Default)]
pub struct VmContext {
    pub entity: EntityId,
    pub tick: u64,
}

#[derive(Debug, Error)]
pub enum GraphVmError {
    #[error("division by zero")]
    DivisionByZero,
    #[error("stack underflow")]
    StackUnderflow,
    #[error("invalid opcode")]
    InvalidOpcode,
    #[error("invalid constant index")]
    InvalidConstant,
    #[error("invalid local index")]
    InvalidLocal,
    #[error("infinite loop detected")]
    InfiniteLoop,
}

pub struct GraphVM {
    stack: Vec<Value>,
    locals: HashMap<u32, Value>,
    emitted_events: Vec<(EntityId, Value)>,
    max_steps: usize,
}

impl Default for GraphVM {
    fn default() -> Self { Self::new() }
}

impl GraphVM {
    pub fn new() -> Self {
        Self {
            stack: Vec::new(),
            locals: HashMap::new(),
            emitted_events: Vec::new(),
            max_steps: 100_000,
        }
    }

    pub fn execute(&mut self, bytecode: &Bytecode, ctx: &mut VmContext) -> Result<(), GraphVmError> {
        self.stack.clear();
        self.emitted_events.clear();
        let mut pc: usize = 0;
        let mut steps = 0usize;

        while pc < bytecode.instructions.len() {
            steps += 1;
            if steps > self.max_steps {
                return Err(GraphVmError::InfiniteLoop);
            }

            let instr = &bytecode.instructions[pc];
            pc += 1;

            match instr.opcode {
                OpCode::Nop => {}
                OpCode::LoadConst => {
                    let idx = instr.a as usize;
                    let val = *bytecode.constants.get(idx).ok_or(GraphVmError::InvalidConstant)?;
                    self.stack.push(val);
                }
                OpCode::LoadVar => {
                    let idx = instr.a;
                    let val = *self.locals.get(&idx).ok_or(GraphVmError::InvalidLocal)?;
                    self.stack.push(val);
                }
                OpCode::StoreVar => {
                    let idx = instr.a;
                    let val = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    self.locals.insert(idx, val);
                }
                OpCode::Add => {
                    let b = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    self.stack.push(a.wrapping_add(b));
                }
                OpCode::Sub => {
                    let b = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    self.stack.push(a.wrapping_sub(b));
                }
                OpCode::Mul => {
                    let b = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    self.stack.push(a.wrapping_mul(b));
                }
                OpCode::Div => {
                    let b = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    if b == 0 {
                        return Err(GraphVmError::DivisionByZero);
                    }
                    self.stack.push(a / b);
                }
                OpCode::CmpEq => {
                    let b = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    self.stack.push(if a == b { 1 } else { 0 });
                }
                OpCode::CmpLt => {
                    let b = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    self.stack.push(if a < b { 1 } else { 0 });
                }
                OpCode::CmpGt => {
                    let b = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    let a = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    self.stack.push(if a > b { 1 } else { 0 });
                }
                OpCode::Jump => {
                    pc = instr.a as usize;
                }
                OpCode::JumpIfFalse => {
                    let cond = self.stack.pop().ok_or(GraphVmError::StackUnderflow)?;
                    if cond == 0 {
                        pc = instr.a as usize;
                    }
                }
                OpCode::EmitEvent => {
                    let val = self.stack.last().copied().unwrap_or(0);
                    self.emitted_events.push((ctx.entity, val));
                }
                OpCode::End => break,
            }
        }
        Ok(())
    }

    pub fn get_local(&self, idx: u32) -> Option<Value> {
        self.locals.get(&idx).copied()
    }

    pub fn stack(&self) -> &[Value] {
        &self.stack
    }

    pub fn emitted_events(&self) -> &[(EntityId, Value)] {
        &self.emitted_events
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn bc(instructions: Vec<(OpCode, u32, u32, u32)>, constants: Vec<Value>) -> Bytecode {
        Bytecode {
            instructions: instructions.into_iter().map(|(op, a, b, c)| Instruction { opcode: op, a, b, c }).collect(),
            constants,
        }
    }

    fn ctx() -> VmContext { VmContext { entity: 1, tick: 0 } }

    #[test]
    fn load_const_end() {
        let mut vm = GraphVM::new();
        let b = bc(vec![
            (OpCode::LoadConst, 0, 0, 0),
            (OpCode::End, 0, 0, 0),
        ], vec![42]);
        vm.execute(&b, &mut ctx()).unwrap();
        assert_eq!(vm.stack(), &[42]);
    }

    #[test]
    fn arithmetic() {
        let mut vm = GraphVM::new();
        // 10 + 5 = 15
        let b = bc(vec![
            (OpCode::LoadConst, 0, 0, 0),
            (OpCode::LoadConst, 1, 0, 0),
            (OpCode::Add, 0, 0, 0),
            (OpCode::End, 0, 0, 0),
        ], vec![10, 5]);
        vm.execute(&b, &mut ctx()).unwrap();
        assert_eq!(vm.stack(), &[15]);
    }

    #[test]
    fn division_by_zero() {
        let mut vm = GraphVM::new();
        let b = bc(vec![
            (OpCode::LoadConst, 0, 0, 0),
            (OpCode::LoadConst, 1, 0, 0),
            (OpCode::Div, 0, 0, 0),
            (OpCode::End, 0, 0, 0),
        ], vec![10, 0]);
        assert!(matches!(vm.execute(&b, &mut ctx()), Err(GraphVmError::DivisionByZero)));
    }

    #[test]
    fn store_and_load_var() {
        let mut vm = GraphVM::new();
        let b = bc(vec![
            (OpCode::LoadConst, 0, 0, 0),   // push 99
            (OpCode::StoreVar,  7, 0, 0),   // locals[7] = 99
            (OpCode::LoadVar,   7, 0, 0),   // push locals[7]
            (OpCode::End, 0, 0, 0),
        ], vec![99]);
        vm.execute(&b, &mut ctx()).unwrap();
        assert_eq!(vm.get_local(7), Some(99));
        assert_eq!(vm.stack(), &[99]);
    }

    #[test]
    fn conditional_jump_false() {
        let mut vm = GraphVM::new();
        // push 0 (false) → JumpIfFalse to End, so no Add runs
        let b = bc(vec![
            (OpCode::LoadConst,   0, 0, 0),   // 0: push 0 (false)
            (OpCode::JumpIfFalse, 3, 0, 0),   // 1: jump to 3 if false
            (OpCode::LoadConst,   1, 0, 0),   // 2: push 100 (skipped)
            (OpCode::End,         0, 0, 0),   // 3: end
        ], vec![0, 100]);
        vm.execute(&b, &mut ctx()).unwrap();
        // Stack should have only what was pushed before the branch, which got popped
        assert!(vm.stack().is_empty());
    }

    #[test]
    fn emit_event() {
        let mut vm = GraphVM::new();
        let b = bc(vec![
            (OpCode::LoadConst,  0, 0, 0),   // push 7
            (OpCode::EmitEvent,  0, 0, 0),   // emit (entity=1, value=7)
            (OpCode::End,        0, 0, 0),
        ], vec![7]);
        let mut c = VmContext { entity: 42, tick: 1 };
        vm.execute(&b, &mut c).unwrap();
        let events = vm.emitted_events();
        assert_eq!(events.len(), 1);
        assert_eq!(events[0], (42, 7));
    }

    #[test]
    fn compare_ops() {
        let mut vm = GraphVM::new();
        // 3 < 5 → 1
        let b = bc(vec![
            (OpCode::LoadConst, 0, 0, 0),
            (OpCode::LoadConst, 1, 0, 0),
            (OpCode::CmpLt,     0, 0, 0),
            (OpCode::End,       0, 0, 0),
        ], vec![3, 5]);
        vm.execute(&b, &mut ctx()).unwrap();
        assert_eq!(vm.stack(), &[1]);
    }

    #[test]
    fn nop_is_harmless() {
        let mut vm = GraphVM::new();
        let b = bc(vec![
            (OpCode::Nop, 0, 0, 0),
            (OpCode::LoadConst, 0, 0, 0),
            (OpCode::End, 0, 0, 0),
        ], vec![5]);
        vm.execute(&b, &mut ctx()).unwrap();
        assert_eq!(vm.stack(), &[5]);
    }

    #[test]
    fn infinite_loop_detected() {
        let mut vm = GraphVM::new();
        // jump to itself
        let b = bc(vec![(OpCode::Jump, 0, 0, 0)], vec![]);
        assert!(matches!(vm.execute(&b, &mut ctx()), Err(GraphVmError::InfiniteLoop)));
    }
}
