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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::vm::{ScriptVM, ScriptValue};

    fn vm_with_builtins() -> ScriptVM {
        let mut vm = ScriptVM::new();
        ScriptSandbox::register_builtins(&mut vm);
        vm
    }

    fn call(vm: &ScriptVM, name: &str, args: Vec<ScriptValue>) -> ScriptValue {
        vm.call_function(name, &args).unwrap_or(ScriptValue::None)
    }

    #[test]
    fn abs_int() {
        let vm = vm_with_builtins();
        assert_eq!(call(&vm, "atlas_abs", vec![ScriptValue::Int(-7)]), ScriptValue::Int(7));
        assert_eq!(call(&vm, "atlas_abs", vec![ScriptValue::Int(3)]),  ScriptValue::Int(3));
    }

    #[test]
    fn abs_float() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_abs", vec![ScriptValue::Float(-2.5)]);
        assert_eq!(r, ScriptValue::Float(2.5));
    }

    #[test]
    fn abs_wrong_type_returns_none() {
        let vm = vm_with_builtins();
        assert_eq!(call(&vm, "atlas_abs", vec![ScriptValue::Bool(true)]), ScriptValue::None);
    }

    #[test]
    fn min_int() {
        let vm = vm_with_builtins();
        assert_eq!(call(&vm, "atlas_min", vec![ScriptValue::Int(3), ScriptValue::Int(7)]), ScriptValue::Int(3));
        assert_eq!(call(&vm, "atlas_min", vec![ScriptValue::Int(10), ScriptValue::Int(2)]), ScriptValue::Int(2));
    }

    #[test]
    fn max_int() {
        let vm = vm_with_builtins();
        assert_eq!(call(&vm, "atlas_max", vec![ScriptValue::Int(3), ScriptValue::Int(7)]), ScriptValue::Int(7));
    }

    #[test]
    fn min_float() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_min", vec![ScriptValue::Float(1.5), ScriptValue::Float(3.0)]);
        assert_eq!(r, ScriptValue::Float(1.5));
    }

    #[test]
    fn clamp_float_within_range() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_clamp", vec![ScriptValue::Float(5.0), ScriptValue::Float(0.0), ScriptValue::Float(10.0)]);
        assert_eq!(r, ScriptValue::Float(5.0));
    }

    #[test]
    fn clamp_float_clamped_low() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_clamp", vec![ScriptValue::Float(-5.0), ScriptValue::Float(0.0), ScriptValue::Float(10.0)]);
        assert_eq!(r, ScriptValue::Float(0.0));
    }

    #[test]
    fn clamp_int() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_clamp", vec![ScriptValue::Int(15), ScriptValue::Int(0), ScriptValue::Int(10)]);
        assert_eq!(r, ScriptValue::Int(10));
    }

    #[test]
    fn floor_ceil() {
        let vm = vm_with_builtins();
        assert_eq!(call(&vm, "atlas_floor", vec![ScriptValue::Float(2.9)]), ScriptValue::Float(2.0));
        assert_eq!(call(&vm, "atlas_ceil",  vec![ScriptValue::Float(2.1)]), ScriptValue::Float(3.0));
    }

    #[test]
    fn sqrt() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_sqrt", vec![ScriptValue::Float(9.0)]);
        assert_eq!(r, ScriptValue::Float(3.0));
    }

    #[test]
    fn strlen() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_strlen", vec![ScriptValue::String("hello".into())]);
        assert_eq!(r, ScriptValue::Int(5));
    }

    #[test]
    fn strlen_non_string_returns_zero() {
        let vm = vm_with_builtins();
        let r = call(&vm, "atlas_strlen", vec![ScriptValue::Int(42)]);
        assert_eq!(r, ScriptValue::Int(0));
    }
}
