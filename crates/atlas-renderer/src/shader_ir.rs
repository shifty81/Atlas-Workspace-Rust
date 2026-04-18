#[derive(Debug, Clone, PartialEq)]
pub enum IrShaderStage { Vertex, Fragment, Compute }

impl Default for IrShaderStage { fn default() -> Self { IrShaderStage::Vertex } }

#[derive(Debug, Clone, PartialEq)]
pub enum ShaderOp {
    Nop, LoadConst, LoadInput, LoadUniform, StoreOutput,
    Add, Sub, Mul, Div, Dot, Cross, Normalize, Lerp, Clamp,
    SampleTexture, Return,
}

impl Default for ShaderOp { fn default() -> Self { ShaderOp::Nop } }

#[derive(Debug, Clone, Default)]
pub struct ShaderInstruction {
    pub op: ShaderOp,
    pub operand0: u16,
    pub operand1: u16,
    pub result: u16,
    pub const_value: f32,
}

#[derive(Debug, Clone, Default)]
pub struct ShaderUniform {
    pub name: String,
    pub binding: u16,
    pub size: u16,
}

#[derive(Debug, Clone, Default)]
pub struct ShaderIo {
    pub name: String,
    pub location: u16,
    pub component_count: u8,
}

#[derive(Debug, Clone)]
pub struct ShaderIrModule {
    pub magic: u32,
    pub version: u32,
    pub stage: IrShaderStage,
    pub name: String,
    pub inputs: Vec<ShaderIo>,
    pub outputs: Vec<ShaderIo>,
    pub uniforms: Vec<ShaderUniform>,
    pub instructions: Vec<ShaderInstruction>,
}

impl Default for ShaderIrModule {
    fn default() -> Self {
        Self {
            magic: 0x53484452,
            version: 1,
            stage: IrShaderStage::Vertex,
            name: String::new(),
            inputs: Vec::new(),
            outputs: Vec::new(),
            uniforms: Vec::new(),
            instructions: Vec::new(),
        }
    }
}

impl ShaderIrModule {
    pub fn validate(&self) -> bool {
        let reg_count = (self.inputs.len() + self.outputs.len() + self.uniforms.len() + 256) as u16;
        for instr in &self.instructions {
            if instr.operand0 > reg_count || instr.operand1 > reg_count { return false; }
        }
        true
    }

    pub fn hash(&self) -> u64 {
        let mut h: u64 = 14695981039346656037u64;
        h ^= self.stage.clone() as u8 as u64;
        h = h.wrapping_mul(0x100000001b3u64);
        for instr in &self.instructions {
            for b in instr.operand0.to_le_bytes().iter()
                .chain(instr.operand1.to_le_bytes().iter())
                .chain(instr.result.to_le_bytes().iter()) {
                h ^= *b as u64;
                h = h.wrapping_mul(0x100000001b3u64);
            }
        }
        h
    }
}

#[derive(Debug, Default)]
pub struct ShaderIrCompiler {
    errors: Vec<String>,
}

impl ShaderIrCompiler {
    pub fn new() -> Self { Self::default() }

    pub fn compile(&mut self, module: &mut ShaderIrModule) -> bool {
        self.errors.clear();
        if !module.validate() {
            self.errors.push("Invalid module: out-of-range operands".into());
            return false;
        }
        true
    }

    pub fn errors(&self) -> &[String] { &self.errors }

    pub fn create_passthrough_vertex(&self) -> ShaderIrModule {
        let mut m = ShaderIrModule::default();
        m.stage = IrShaderStage::Vertex;
        m.name = "passthrough_vertex".into();
        m.inputs.push(ShaderIo { name: "position".into(), location: 0, component_count: 4 });
        m.outputs.push(ShaderIo { name: "gl_Position".into(), location: 0, component_count: 4 });
        m.instructions.push(ShaderInstruction { op: ShaderOp::LoadInput, operand0: 0, result: 0, ..Default::default() });
        m.instructions.push(ShaderInstruction { op: ShaderOp::StoreOutput, operand0: 0, result: 0, ..Default::default() });
        m.instructions.push(ShaderInstruction { op: ShaderOp::Return, ..Default::default() });
        m
    }

    pub fn create_solid_color_fragment(&self, r: f32, g: f32, b: f32, a: f32) -> ShaderIrModule {
        let mut m = ShaderIrModule::default();
        m.stage = IrShaderStage::Fragment;
        m.name = "solid_color_fragment".into();
        m.outputs.push(ShaderIo { name: "fragColor".into(), location: 0, component_count: 4 });
        let vals = [r, g, b, a];
        for (i, &v) in vals.iter().enumerate() {
            m.instructions.push(ShaderInstruction { op: ShaderOp::LoadConst, result: i as u16, const_value: v, ..Default::default() });
        }
        m.instructions.push(ShaderInstruction { op: ShaderOp::StoreOutput, operand0: 0, result: 0, ..Default::default() });
        m.instructions.push(ShaderInstruction { op: ShaderOp::Return, ..Default::default() });
        m
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_module_is_valid() {
        let m = ShaderIrModule::default();
        assert!(m.validate());
        assert_eq!(m.stage, IrShaderStage::Vertex);
    }

    #[test]
    fn hash_is_deterministic() {
        let m = ShaderIrModule::default();
        assert_eq!(m.hash(), m.hash());
    }

    #[test]
    fn hash_changes_with_instructions() {
        let m1 = ShaderIrModule::default();
        let mut m2 = ShaderIrModule::default();
        m2.instructions.push(ShaderInstruction { op: ShaderOp::Nop, operand0: 1, ..Default::default() });
        assert_ne!(m1.hash(), m2.hash());
    }

    #[test]
    fn compile_valid_module_succeeds() {
        let mut compiler = ShaderIrCompiler::new();
        let mut m = ShaderIrModule::default();
        assert!(compiler.compile(&mut m));
        assert!(compiler.errors().is_empty());
    }

    #[test]
    fn create_passthrough_vertex_is_valid() {
        let compiler = ShaderIrCompiler::new();
        let m = compiler.create_passthrough_vertex();
        assert_eq!(m.stage, IrShaderStage::Vertex);
        assert!(!m.instructions.is_empty());
        assert!(m.validate());
    }

    #[test]
    fn create_solid_color_fragment_has_four_load_consts() {
        let compiler = ShaderIrCompiler::new();
        let m = compiler.create_solid_color_fragment(1.0, 0.0, 0.5, 1.0);
        assert_eq!(m.stage, IrShaderStage::Fragment);
        let load_const_count = m.instructions.iter().filter(|i| i.op == ShaderOp::LoadConst).count();
        assert_eq!(load_const_count, 4);
        assert!(m.validate());
    }

    #[test]
    fn stage_default() {
        assert_eq!(IrShaderStage::default(), IrShaderStage::Vertex);
    }

    #[test]
    fn shader_op_default() {
        assert_eq!(ShaderOp::default(), ShaderOp::Nop);
    }
}
