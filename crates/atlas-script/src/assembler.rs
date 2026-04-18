//! Text assembler for the Atlas script VM.
//!
//! Converts a simple mnemonic assembly language into a [`CompiledScript`].
//!
//! ## Syntax
//!
//! Each line is either:
//! - A comment: `; anything`
//! - A mnemonic with an optional operand: `PUSH_INT 42`
//! - A label declaration: `loop:` (used as jump target)
//!
//! ## Mnemonics
//!
//! | Mnemonic     | Operand        | Notes |
//! |--------------|----------------|-------|
//! | `PUSH_INT`   | integer literal | Adds an `Int` constant and pushes it |
//! | `PUSH_FLOAT` | float literal   | Adds a `Float` constant and pushes it |
//! | `PUSH_STR`   | string literal  | String without quotes, rest of line |
//! | `ADD`        | —               | |
//! | `SUB`        | —               | |
//! | `MUL`        | —               | |
//! | `DIV`        | —               | |
//! | `MOD`        | —               | |
//! | `NEG`        | —               | |
//! | `EQ`         | —               | |
//! | `NEQ`        | —               | |
//! | `LT`         | —               | |
//! | `GT`         | —               | |
//! | `LTE`        | —               | |
//! | `GTE`        | —               | |
//! | `AND`        | —               | |
//! | `OR`         | —               | |
//! | `NOT`        | —               | |
//! | `LOAD`       | var_name        | LoadVar — name added to constants |
//! | `STORE`      | var_name        | StoreVar |
//! | `CALL`       | fn_name n_args  | two operands: fn name + arg count |
//! | `JMP`        | label           | Unconditional jump |
//! | `JMPIF`      | label           | Conditional jump (pops bool) |
//! | `RET`        | —               | |
//! | `HALT`       | —               | |
//!
//! ## Example
//!
//! ```text
//! ; Compute 2 + 3 and halt
//! PUSH_INT 2
//! PUSH_INT 3
//! ADD
//! HALT
//! ```

use crate::vm::{CompiledScript, Instruction, Opcode, ScriptValue};

/// Assembly error.
#[derive(Debug)]
pub struct AssemblerError {
    pub line: usize,
    pub message: String,
}

impl std::fmt::Display for AssemblerError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "line {}: {}", self.line, self.message)
    }
}

/// Compiles mnemonic assembly text into a [`CompiledScript`].
pub struct ScriptAssembler;

impl ScriptAssembler {
    /// Assemble `source` into a [`CompiledScript`] named `name`.
    ///
    /// Returns `Err` if any line cannot be parsed.
    pub fn assemble(name: &str, source: &str) -> Result<CompiledScript, AssemblerError> {
        let mut constants: Vec<ScriptValue> = Vec::new();
        let mut raw_instrs: Vec<(Opcode, RawOperand, usize)> = Vec::new(); // (opcode, operand, line_no)
        let mut labels: std::collections::HashMap<String, usize> = std::collections::HashMap::new();

        // ── First pass: collect labels and raw instructions ────────────────────
        for (line_idx, raw_line) in source.lines().enumerate() {
            let line_no = line_idx + 1;
            let line = raw_line.trim();

            if line.is_empty() || line.starts_with(';') {
                continue;
            }

            // Label declaration: "my_label:"
            if line.ends_with(':') {
                let label = &line[..line.len() - 1];
                labels.insert(label.to_string(), raw_instrs.len());
                continue;
            }

            // Split into mnemonic and rest
            let mut parts = line.splitn(2, |c: char| c.is_whitespace());
            let mnemonic = parts.next().unwrap().to_uppercase();
            let rest = parts.next().map(|s| s.trim()).unwrap_or("");

            let (opcode, operand) = match mnemonic.as_str() {
                "PUSH_INT" => {
                    let v: i64 = rest.parse().map_err(|_| err(line_no, format!("expected integer, got {rest:?}")))?;
                    let idx = add_constant(&mut constants, ScriptValue::Int(v));
                    (Opcode::PushInt, RawOperand::Index(idx as i32))
                }
                "PUSH_FLOAT" => {
                    let v: f64 = rest.parse().map_err(|_| err(line_no, format!("expected float, got {rest:?}")))?;
                    let idx = add_constant(&mut constants, ScriptValue::Float(v));
                    (Opcode::PushFloat, RawOperand::Index(idx as i32))
                }
                "PUSH_STR" => {
                    let idx = add_constant(&mut constants, ScriptValue::String(rest.to_string()));
                    (Opcode::PushString, RawOperand::Index(idx as i32))
                }
                "ADD"  => (Opcode::Add,  RawOperand::None),
                "SUB"  => (Opcode::Sub,  RawOperand::None),
                "MUL"  => (Opcode::Mul,  RawOperand::None),
                "DIV"  => (Opcode::Div,  RawOperand::None),
                "MOD"  => (Opcode::Mod,  RawOperand::None),
                "NEG"  => (Opcode::Neg,  RawOperand::None),
                "EQ"   => (Opcode::Eq,   RawOperand::None),
                "NEQ"  => (Opcode::Neq,  RawOperand::None),
                "LT"   => (Opcode::Lt,   RawOperand::None),
                "GT"   => (Opcode::Gt,   RawOperand::None),
                "LTE"  => (Opcode::Lte,  RawOperand::None),
                "GTE"  => (Opcode::Gte,  RawOperand::None),
                "AND"  => (Opcode::And,  RawOperand::None),
                "OR"   => (Opcode::Or,   RawOperand::None),
                "NOT"  => (Opcode::Not,  RawOperand::None),
                "RET"  => (Opcode::Ret,  RawOperand::None),
                "HALT" => (Opcode::Halt, RawOperand::None),
                "LOAD" => {
                    let idx = add_constant(&mut constants, ScriptValue::String(rest.to_string()));
                    (Opcode::LoadVar, RawOperand::Index(idx as i32))
                }
                "STORE" => {
                    let idx = add_constant(&mut constants, ScriptValue::String(rest.to_string()));
                    (Opcode::StoreVar, RawOperand::Index(idx as i32))
                }
                "CALL" => {
                    // CALL fn_name n_args
                    let mut cparts = rest.splitn(2, |c: char| c.is_whitespace());
                    let fn_name = cparts.next().ok_or_else(|| err(line_no, "CALL requires fn_name".into()))?;
                    let n_args_str = cparts.next().map(|s| s.trim()).unwrap_or("0");
                    let n_args: i32 = n_args_str.parse().map_err(|_| err(line_no, format!("expected arg count, got {n_args_str:?}")))?;
                    let fn_idx = add_constant(&mut constants, ScriptValue::String(fn_name.to_string()));
                    let operand = (n_args << 16) | (fn_idx as i32 & 0xFFFF);
                    (Opcode::Call, RawOperand::Index(operand))
                }
                "JMP" => {
                    (Opcode::Jmp,    RawOperand::Label(rest.to_string()))
                }
                "JMPIF" => {
                    (Opcode::JmpIf, RawOperand::Label(rest.to_string()))
                }
                other => return Err(err(line_no, format!("unknown mnemonic {other:?}"))),
            };

            raw_instrs.push((opcode, operand, line_no));
        }

        // ── Second pass: resolve labels ────────────────────────────────────────
        let mut code = Vec::with_capacity(raw_instrs.len());
        for (opcode, operand, line_no) in raw_instrs {
            let resolved_operand = match operand {
                RawOperand::None        => 0,
                RawOperand::Index(i)    => i,
                RawOperand::Label(lbl)  => {
                    let target = labels.get(&lbl).ok_or_else(|| err(line_no, format!("undefined label {lbl:?}")))?;
                    *target as i32
                }
            };
            code.push(Instruction::new(opcode, resolved_operand));
        }

        let mut script = CompiledScript::new(name);
        script.code      = code;
        script.constants = constants;
        Ok(script)
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

enum RawOperand {
    None,
    Index(i32),
    Label(String),
}

fn err(line: usize, message: String) -> AssemblerError {
    AssemblerError { line, message }
}

/// Find an existing identical constant or push a new one; return the index.
fn add_constant(pool: &mut Vec<ScriptValue>, val: ScriptValue) -> usize {
    if let Some(pos) = pool.iter().position(|v| v == &val) {
        return pos;
    }
    pool.push(val);
    pool.len() - 1
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::vm::ScriptVM;

    fn run(src: &str) -> crate::vm::ScriptValue {
        let script = ScriptAssembler::assemble("test", src).expect("assemble");
        let mut vm = ScriptVM::new();
        vm.execute(&script).expect("execute")
    }

    #[test]
    fn simple_push_halt() {
        let v = run("PUSH_INT 42\nHALT");
        assert_eq!(v, crate::vm::ScriptValue::Int(42));
    }

    #[test]
    fn addition() {
        let v = run("PUSH_INT 10\nPUSH_INT 3\nADD\nHALT");
        assert_eq!(v, crate::vm::ScriptValue::Int(13));
    }

    #[test]
    fn float_arithmetic() {
        let v = run("PUSH_FLOAT 1.5\nPUSH_FLOAT 2.5\nADD\nHALT");
        assert_eq!(v, crate::vm::ScriptValue::Float(4.0));
    }

    #[test]
    fn store_load() {
        let src = "PUSH_INT 99\nSTORE counter\nLOAD counter\nHALT";
        let v = run(src);
        assert_eq!(v, crate::vm::ScriptValue::Int(99));
    }

    #[test]
    fn label_jump() {
        // Unconditional jump skips the push of 0 → returns 7
        let src = "JMP done\nPUSH_INT 0\ndone:\nPUSH_INT 7\nHALT";
        let v = run(src);
        assert_eq!(v, crate::vm::ScriptValue::Int(7));
    }

    #[test]
    fn conditional_jump() {
        let src = "\
PUSH_INT 1
JMPIF skip
PUSH_INT 0
skip:
PUSH_INT 9
HALT";
        let v = run(src);
        assert_eq!(v, crate::vm::ScriptValue::Int(9));
    }

    #[test]
    fn comments_ignored() {
        let v = run("; this is a comment\nPUSH_INT 5\n; another\nHALT");
        assert_eq!(v, crate::vm::ScriptValue::Int(5));
    }

    #[test]
    fn unknown_mnemonic_error() {
        let result = ScriptAssembler::assemble("bad", "FOOBAR 1");
        assert!(result.is_err());
        let e = result.unwrap_err();
        assert_eq!(e.line, 1);
    }

    #[test]
    fn undefined_label_error() {
        let result = ScriptAssembler::assemble("bad", "JMP missing");
        assert!(result.is_err());
    }

    #[test]
    fn call_native_fn() {
        let mut vm = ScriptVM::new();
        crate::sandbox::ScriptSandbox::register_builtins(&mut vm);
        let script = ScriptAssembler::assemble("test",
            "PUSH_INT -5\nCALL atlas_abs 1\nHALT"
        ).unwrap();
        let v = vm.execute(&script).unwrap();
        assert_eq!(v, crate::vm::ScriptValue::Int(5));
    }

    #[test]
    fn constant_dedup() {
        // Two uses of same integer should produce only one constant entry
        let script = ScriptAssembler::assemble("dedup", "PUSH_INT 42\nPUSH_INT 42\nADD\nHALT").unwrap();
        assert_eq!(script.constants.len(), 1);
    }
}
