use crate::vm::{ScriptValue, ScriptVM};

pub struct ScriptSandbox;

impl ScriptSandbox {
    pub fn register_builtins(vm: &mut ScriptVM) {
        vm.register_function("atlas_abs", |args| {
            match args.first() {
                Some(ScriptValue::Int(v)) => ScriptValue::Int(v.abs()),
                Some(ScriptValue::Float(v)) => ScriptValue::Float(v.abs()),
                _ => ScriptValue::None,
            }
        });
        vm.register_function("atlas_min", |args| {
            match (args.first(), args.get(1)) {
                (Some(ScriptValue::Int(a)), Some(ScriptValue::Int(b))) => ScriptValue::Int(*a.min(b)),
                (Some(ScriptValue::Float(a)), Some(ScriptValue::Float(b))) => ScriptValue::Float(a.min(*b)),
                _ => ScriptValue::None,
            }
        });
        vm.register_function("atlas_max", |args| {
            match (args.first(), args.get(1)) {
                (Some(ScriptValue::Int(a)), Some(ScriptValue::Int(b))) => ScriptValue::Int(*a.max(b)),
                (Some(ScriptValue::Float(a)), Some(ScriptValue::Float(b))) => ScriptValue::Float(a.max(*b)),
                _ => ScriptValue::None,
            }
        });
        vm.register_function("atlas_clamp", |args| {
            match (args.first(), args.get(1), args.get(2)) {
                (Some(ScriptValue::Float(v)), Some(ScriptValue::Float(lo)), Some(ScriptValue::Float(hi))) => {
                    ScriptValue::Float(v.clamp(*lo, *hi))
                }
                (Some(ScriptValue::Int(v)), Some(ScriptValue::Int(lo)), Some(ScriptValue::Int(hi))) => {
                    ScriptValue::Int((*v).clamp(*lo, *hi))
                }
                _ => ScriptValue::None,
            }
        });
        vm.register_function("atlas_floor", |args| {
            match args.first() {
                Some(ScriptValue::Float(v)) => ScriptValue::Float(v.floor()),
                _ => ScriptValue::None,
            }
        });
        vm.register_function("atlas_ceil", |args| {
            match args.first() {
                Some(ScriptValue::Float(v)) => ScriptValue::Float(v.ceil()),
                _ => ScriptValue::None,
            }
        });
        vm.register_function("atlas_sqrt", |args| {
            match args.first() {
                Some(ScriptValue::Float(v)) => ScriptValue::Float(v.sqrt()),
                _ => ScriptValue::None,
            }
        });
        vm.register_function("atlas_strlen", |args| {
            match args.first() {
                Some(ScriptValue::String(s)) => ScriptValue::Int(s.len() as i64),
                _ => ScriptValue::Int(0),
            }
        });
    }
}
