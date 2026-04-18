#include "AtlasShaderIR.h"
#include <cstring>

namespace atlas::render {

// FNV-1a constants for 64-bit (same as StateHasher)
static constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
static constexpr uint64_t FNV_PRIME  = 1099511628211ULL;

static uint64_t HashBytes(uint64_t prev, const uint8_t* data, size_t size) {
    uint64_t h = prev;
    for (size_t i = 0; i < size; ++i) {
        h ^= static_cast<uint64_t>(data[i]);
        h *= FNV_PRIME;
    }
    return h;
}

static void WriteU8(std::vector<uint8_t>& buf, uint8_t v) {
    buf.push_back(v);
}

static void WriteU16(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back(static_cast<uint8_t>(v & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}

static void WriteU32(std::vector<uint8_t>& buf, uint32_t v) {
    buf.push_back(static_cast<uint8_t>(v & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
}

static void WriteFloat(std::vector<uint8_t>& buf, float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    WriteU32(buf, bits);
}

static void WriteString(std::vector<uint8_t>& buf, const std::string& s) {
    WriteU32(buf, static_cast<uint32_t>(s.size()));
    for (char c : s) {
        buf.push_back(static_cast<uint8_t>(c));
    }
}

static uint8_t ReadU8(const std::vector<uint8_t>& data, size_t& pos) {
    if (pos >= data.size()) return 0;
    return data[pos++];
}

static uint16_t ReadU16(const std::vector<uint8_t>& data, size_t& pos) {
    if (pos + 2 > data.size()) return 0;
    uint16_t v = static_cast<uint16_t>(data[pos])
               | (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    return v;
}

static uint32_t ReadU32(const std::vector<uint8_t>& data, size_t& pos) {
    if (pos + 4 > data.size()) return 0;
    uint32_t v = static_cast<uint32_t>(data[pos])
               | (static_cast<uint32_t>(data[pos + 1]) << 8)
               | (static_cast<uint32_t>(data[pos + 2]) << 16)
               | (static_cast<uint32_t>(data[pos + 3]) << 24);
    pos += 4;
    return v;
}

static float ReadFloat(const std::vector<uint8_t>& data, size_t& pos) {
    uint32_t bits = ReadU32(data, pos);
    float v;
    std::memcpy(&v, &bits, sizeof(v));
    return v;
}

static std::string ReadString(const std::vector<uint8_t>& data, size_t& pos) {
    uint32_t len = ReadU32(data, pos);
    if (len > 65536 || len > data.size() || pos > data.size() - len) return "";
    std::string s(data.begin() + static_cast<ptrdiff_t>(pos),
                  data.begin() + static_cast<ptrdiff_t>(pos + len));
    pos += len;
    return s;
}

// --- ShaderModule ---

bool ShaderModule::Validate() const {
    if (name.empty()) return false;

    uint16_t maxReg = 0;

    // Determine max register from inputs/outputs/uniforms
    for (const auto& io : inputs) {
        if (io.location + 1 > maxReg)
            maxReg = io.location + 1;
    }
    for (const auto& io : outputs) {
        if (io.location + 1 > maxReg)
            maxReg = io.location + 1;
    }
    for (const auto& u : uniforms) {
        if (u.binding + 1 > maxReg)
            maxReg = u.binding + 1;
    }

    // Allow registers up to a generous bound based on instructions
    uint16_t maxInstrReg = maxReg;
    for (const auto& instr : instructions) {
        if (instr.result + 1 > maxInstrReg)
            maxInstrReg = instr.result + 1;
    }

    // Validate each instruction's operand bounds
    for (const auto& instr : instructions) {
        switch (instr.op) {
            case ShaderOp::Nop:
            case ShaderOp::Return:
                break;
            case ShaderOp::LoadConst:
                break;
            case ShaderOp::LoadInput:
                if (instr.operand0 >= inputs.size()) return false;
                break;
            case ShaderOp::LoadUniform:
                if (instr.operand0 >= uniforms.size()) return false;
                break;
            case ShaderOp::StoreOutput:
                if (instr.operand0 >= outputs.size()) return false;
                break;
            case ShaderOp::SampleTexture:
                if (instr.operand0 >= uniforms.size()) return false;
                break;
            case ShaderOp::Add:
            case ShaderOp::Sub:
            case ShaderOp::Mul:
            case ShaderOp::Div:
            case ShaderOp::Dot:
            case ShaderOp::Cross:
            case ShaderOp::Lerp:
            case ShaderOp::Clamp:
                break;
            case ShaderOp::Normalize:
                break;
        }
    }

    return true;
}

uint64_t ShaderModule::Hash() const {
    uint64_t h = FNV_OFFSET;

    // Hash stage
    auto stageVal = static_cast<uint8_t>(stage);
    h = HashBytes(h, &stageVal, 1);

    // Hash name
    h = HashBytes(h, reinterpret_cast<const uint8_t*>(name.data()), name.size());

    // Hash inputs
    for (const auto& io : inputs) {
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(io.name.data()), io.name.size());
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&io.location), sizeof(io.location));
        h = HashBytes(h, &io.componentCount, 1);
    }

    // Hash outputs
    for (const auto& io : outputs) {
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(io.name.data()), io.name.size());
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&io.location), sizeof(io.location));
        h = HashBytes(h, &io.componentCount, 1);
    }

    // Hash uniforms
    for (const auto& u : uniforms) {
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(u.name.data()), u.name.size());
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&u.binding), sizeof(u.binding));
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&u.size), sizeof(u.size));
    }

    // Hash instructions
    for (const auto& instr : instructions) {
        auto opVal = static_cast<uint8_t>(instr.op);
        h = HashBytes(h, &opVal, 1);
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&instr.operand0), sizeof(instr.operand0));
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&instr.operand1), sizeof(instr.operand1));
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&instr.result), sizeof(instr.result));
        uint32_t constBits;
        std::memcpy(&constBits, &instr.constValue, sizeof(constBits));
        h = HashBytes(h, reinterpret_cast<const uint8_t*>(&constBits), sizeof(constBits));
    }

    return h;
}

std::vector<uint8_t> ShaderModule::Serialize() const {
    std::vector<uint8_t> buf;
    buf.reserve(256);

    // Header
    WriteU32(buf, MAGIC);
    WriteU32(buf, VERSION);
    WriteU8(buf, static_cast<uint8_t>(stage));
    WriteString(buf, name);

    // Inputs
    WriteU32(buf, static_cast<uint32_t>(inputs.size()));
    for (const auto& io : inputs) {
        WriteString(buf, io.name);
        WriteU16(buf, io.location);
        WriteU8(buf, io.componentCount);
    }

    // Outputs
    WriteU32(buf, static_cast<uint32_t>(outputs.size()));
    for (const auto& io : outputs) {
        WriteString(buf, io.name);
        WriteU16(buf, io.location);
        WriteU8(buf, io.componentCount);
    }

    // Uniforms
    WriteU32(buf, static_cast<uint32_t>(uniforms.size()));
    for (const auto& u : uniforms) {
        WriteString(buf, u.name);
        WriteU16(buf, u.binding);
        WriteU16(buf, u.size);
    }

    // Instructions
    WriteU32(buf, static_cast<uint32_t>(instructions.size()));
    for (const auto& instr : instructions) {
        WriteU8(buf, static_cast<uint8_t>(instr.op));
        WriteU16(buf, instr.operand0);
        WriteU16(buf, instr.operand1);
        WriteU16(buf, instr.result);
        WriteFloat(buf, instr.constValue);
    }

    return buf;
}

ShaderModule ShaderModule::Deserialize(const std::vector<uint8_t>& data) {
    ShaderModule mod;
    size_t pos = 0;

    // Header
    uint32_t magic = ReadU32(data, pos);
    if (magic != MAGIC) return mod;
    uint32_t version = ReadU32(data, pos);
    if (version != VERSION) return mod;

    mod.stage = static_cast<ShaderStage>(ReadU8(data, pos));
    mod.name = ReadString(data, pos);

    // Inputs
    uint32_t inputCount = ReadU32(data, pos);
    if (inputCount > 4096) return mod;
    mod.inputs.resize(inputCount);
    for (uint32_t i = 0; i < inputCount; ++i) {
        mod.inputs[i].name = ReadString(data, pos);
        mod.inputs[i].location = ReadU16(data, pos);
        mod.inputs[i].componentCount = ReadU8(data, pos);
    }

    // Outputs
    uint32_t outputCount = ReadU32(data, pos);
    if (outputCount > 4096) return mod;
    mod.outputs.resize(outputCount);
    for (uint32_t i = 0; i < outputCount; ++i) {
        mod.outputs[i].name = ReadString(data, pos);
        mod.outputs[i].location = ReadU16(data, pos);
        mod.outputs[i].componentCount = ReadU8(data, pos);
    }

    // Uniforms
    uint32_t uniformCount = ReadU32(data, pos);
    if (uniformCount > 4096) return mod;
    mod.uniforms.resize(uniformCount);
    for (uint32_t i = 0; i < uniformCount; ++i) {
        mod.uniforms[i].name = ReadString(data, pos);
        mod.uniforms[i].binding = ReadU16(data, pos);
        mod.uniforms[i].size = ReadU16(data, pos);
    }

    // Instructions
    uint32_t instrCount = ReadU32(data, pos);
    if (instrCount > 1000000) return mod;
    mod.instructions.resize(instrCount);
    for (uint32_t i = 0; i < instrCount; ++i) {
        mod.instructions[i].op = static_cast<ShaderOp>(ReadU8(data, pos));
        mod.instructions[i].operand0 = ReadU16(data, pos);
        mod.instructions[i].operand1 = ReadU16(data, pos);
        mod.instructions[i].result = ReadU16(data, pos);
        mod.instructions[i].constValue = ReadFloat(data, pos);
    }

    return mod;
}

// --- ShaderIRCompiler ---

ShaderModule ShaderIRCompiler::CreatePassthroughVertex() {
    ShaderModule mod;
    mod.stage = ShaderStage::Vertex;
    mod.name = "passthrough_vertex";

    // Input: position (location 0, vec3)
    ShaderIO posInput;
    posInput.name = "inPosition";
    posInput.location = 0;
    posInput.componentCount = 3;
    mod.inputs.push_back(posInput);

    // Output: position (location 0, vec4)
    ShaderIO posOutput;
    posOutput.name = "outPosition";
    posOutput.location = 0;
    posOutput.componentCount = 4;
    mod.outputs.push_back(posOutput);

    // Instructions: load input position, store to output, return
    ShaderInstruction loadPos;
    loadPos.op = ShaderOp::LoadInput;
    loadPos.operand0 = 0; // input index 0
    loadPos.result = 0;   // register 0
    mod.instructions.push_back(loadPos);

    ShaderInstruction storePos;
    storePos.op = ShaderOp::StoreOutput;
    storePos.operand0 = 0; // output index 0
    storePos.operand1 = 0; // from register 0
    mod.instructions.push_back(storePos);

    ShaderInstruction ret;
    ret.op = ShaderOp::Return;
    mod.instructions.push_back(ret);

    return mod;
}

ShaderModule ShaderIRCompiler::CreateSolidColorFragment(float r, float g, float b, float a) {
    ShaderModule mod;
    mod.stage = ShaderStage::Fragment;
    mod.name = "solid_color_fragment";

    // Output: color (location 0, vec4)
    ShaderIO colorOutput;
    colorOutput.name = "outColor";
    colorOutput.location = 0;
    colorOutput.componentCount = 4;
    mod.outputs.push_back(colorOutput);

    // Instructions: load four constants (r, g, b, a), store to output, return
    ShaderInstruction loadR;
    loadR.op = ShaderOp::LoadConst;
    loadR.result = 0;
    loadR.constValue = r;
    mod.instructions.push_back(loadR);

    ShaderInstruction loadG;
    loadG.op = ShaderOp::LoadConst;
    loadG.result = 1;
    loadG.constValue = g;
    mod.instructions.push_back(loadG);

    ShaderInstruction loadB;
    loadB.op = ShaderOp::LoadConst;
    loadB.result = 2;
    loadB.constValue = b;
    mod.instructions.push_back(loadB);

    ShaderInstruction loadA;
    loadA.op = ShaderOp::LoadConst;
    loadA.result = 3;
    loadA.constValue = a;
    mod.instructions.push_back(loadA);

    ShaderInstruction storeColor;
    storeColor.op = ShaderOp::StoreOutput;
    storeColor.operand0 = 0; // output index 0
    storeColor.operand1 = 0; // from register 0 (4 components starting here)
    mod.instructions.push_back(storeColor);

    ShaderInstruction ret;
    ret.op = ShaderOp::Return;
    mod.instructions.push_back(ret);

    return mod;
}

bool ShaderIRCompiler::Compile(ShaderModule& module) {
    m_errors.clear();

    if (module.name.empty()) {
        m_errors.push_back("Shader module name is empty");
        return false;
    }

    if (module.instructions.empty()) {
        m_errors.push_back("Shader module has no instructions");
        return false;
    }

    // Validate each instruction
    for (size_t i = 0; i < module.instructions.size(); ++i) {
        if (!ValidateInstruction(module.instructions[i], module)) {
            m_errors.push_back("Invalid instruction at index " + std::to_string(i));
            return false;
        }
    }

    // Check that the last instruction is a Return
    if (module.instructions.back().op != ShaderOp::Return) {
        m_errors.push_back("Shader must end with a Return instruction");
        return false;
    }

    return true;
}

const std::vector<std::string>& ShaderIRCompiler::Errors() const {
    return m_errors;
}

bool ShaderIRCompiler::ValidateInstruction(const ShaderInstruction& instr, const ShaderModule& module) {
    switch (instr.op) {
        case ShaderOp::Nop:
        case ShaderOp::Return:
        case ShaderOp::LoadConst:
            return true;
        case ShaderOp::LoadInput:
            return instr.operand0 < module.inputs.size();
        case ShaderOp::LoadUniform:
            return instr.operand0 < module.uniforms.size();
        case ShaderOp::StoreOutput:
            return instr.operand0 < module.outputs.size();
        case ShaderOp::SampleTexture:
            return instr.operand0 < module.uniforms.size();
        case ShaderOp::Add:
        case ShaderOp::Sub:
        case ShaderOp::Mul:
        case ShaderOp::Div:
        case ShaderOp::Dot:
        case ShaderOp::Cross:
        case ShaderOp::Lerp:
        case ShaderOp::Clamp:
        case ShaderOp::Normalize:
            return true;
    }
    return false;
}

} // namespace atlas::render
