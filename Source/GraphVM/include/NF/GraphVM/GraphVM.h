#pragma once
// NF::GraphVM — Deterministic bytecode VM, compiler, serialization
#include "NF/Core/Core.h"
#include <variant>
#include <algorithm>
#include <queue>
#include <unordered_set>

namespace NF {

using GraphNodeID = uint32_t;

enum class GraphOpCode : uint8_t {
    Nop,
    LoadConst,
    Add,
    Subtract,
    Multiply,
    Divide,
    Compare,
    Branch,
    Call,
    Return,
    Store,
    Load,
    Halt
};

struct GraphInstruction {
    GraphOpCode opcode = GraphOpCode::Nop;
    uint32_t operandA = 0;
    uint32_t operandB = 0;
    uint32_t operandC = 0;
};

using GraphValue = std::variant<int32_t, float, bool, std::string>;

// ── GraphProgram ─────────────────────────────────────────────────

struct GraphProgram {
    std::vector<GraphInstruction> instructions;
    std::vector<GraphValue> constants;
};

// ── GraphVM ──────────────────────────────────────────────────────

static constexpr size_t kRegisterCount = 16;
static constexpr size_t kMemorySlots   = 256;

class GraphVM {
public:
    void init() {
        NF_LOG_INFO("GraphVM", "Graph VM initialized");
        m_registers.resize(kRegisterCount);
        m_memory.resize(kMemorySlots);
    }

    void shutdown() { NF_LOG_INFO("GraphVM", "Graph VM shutdown"); }

    void loadProgram(const std::vector<GraphInstruction>& program) {
        GraphProgram prog;
        prog.instructions = program;
        loadProgram(prog);
    }

    void loadProgram(const GraphProgram& program) {
        m_instructions = program.instructions;
        m_constants    = program.constants;
        m_pc           = 0;
        m_halted       = false;
        std::fill(m_registers.begin(), m_registers.end(), GraphValue{0});
        m_callStack.clear();
    }

    void step() {
        if (m_halted || m_pc >= m_instructions.size()) {
            m_halted = true;
            return;
        }
        const auto& inst = m_instructions[m_pc];
        switch (inst.opcode) {
        case GraphOpCode::Nop:
            ++m_pc;
            break;

        case GraphOpCode::LoadConst:
            if (inst.operandB < m_constants.size()) {
                reg(inst.operandA) = m_constants[inst.operandB];
            }
            ++m_pc;
            break;

        case GraphOpCode::Add:       execArith(inst, std::plus<>{});       break;
        case GraphOpCode::Subtract:  execArith(inst, std::minus<>{});      break;
        case GraphOpCode::Multiply:  execArith(inst, std::multiplies<>{}); break;
        case GraphOpCode::Divide:    execArith(inst, DivOp{});             break;

        case GraphOpCode::Compare: {
            const auto& a = reg(inst.operandB);
            const auto& b = reg(inst.operandC);
            bool result = false;
            if (std::holds_alternative<float>(a) || std::holds_alternative<float>(b)) {
                result = toFloat(a) < toFloat(b);
            } else {
                result = toInt(a) < toInt(b);
            }
            reg(inst.operandA) = result;
            ++m_pc;
            break;
        }

        case GraphOpCode::Branch: {
            bool cond = false;
            const auto& v = reg(inst.operandA);
            if (std::holds_alternative<bool>(v))    cond = std::get<bool>(v);
            else if (std::holds_alternative<int32_t>(v)) cond = std::get<int32_t>(v) != 0;
            m_pc = cond ? inst.operandB : inst.operandC;
            break;
        }

        case GraphOpCode::Store:
            if (inst.operandB < m_memory.size()) {
                m_memory[inst.operandB] = reg(inst.operandA);
            }
            ++m_pc;
            break;

        case GraphOpCode::Load:
            if (inst.operandB < m_memory.size()) {
                reg(inst.operandA) = m_memory[inst.operandB];
            }
            ++m_pc;
            break;

        case GraphOpCode::Call:
            m_callStack.push_back(m_pc + 1);
            m_pc = inst.operandA;
            break;

        case GraphOpCode::Return:
            if (!m_callStack.empty()) {
                m_pc = m_callStack.back();
                m_callStack.pop_back();
            } else {
                m_halted = true;
            }
            break;

        case GraphOpCode::Halt:
            m_halted = true;
            break;
        }
    }

    void run() {
        while (!m_halted) step();
    }

    [[nodiscard]] bool halted() const { return m_halted; }

    [[nodiscard]] const GraphValue& getRegister(uint32_t idx) const {
        static const GraphValue nil{0};
        return idx < m_registers.size() ? m_registers[idx] : nil;
    }

    [[nodiscard]] const GraphValue& getMemory(uint32_t slot) const {
        static const GraphValue nil{0};
        return slot < m_memory.size() ? m_memory[slot] : nil;
    }

    [[nodiscard]] size_t pc() const { return m_pc; }

private:
    std::vector<GraphInstruction> m_instructions;
    std::vector<GraphValue>       m_constants;
    std::vector<GraphValue>       m_registers{kRegisterCount, GraphValue{0}};
    std::vector<GraphValue>       m_memory{kMemorySlots, GraphValue{0}};
    std::vector<size_t>           m_callStack;
    size_t m_pc      = 0;
    bool   m_halted  = false;

    GraphValue& reg(uint32_t idx) {
        if (idx >= kRegisterCount) {
            NF_LOG_WARN("GraphVM", "Register index out of bounds, wrapping");
        }
        return m_registers[idx % kRegisterCount];
    }

    static int32_t toInt(const GraphValue& v) {
        if (std::holds_alternative<int32_t>(v)) return std::get<int32_t>(v);
        if (std::holds_alternative<float>(v))   return static_cast<int32_t>(std::get<float>(v));
        if (std::holds_alternative<bool>(v))    return std::get<bool>(v) ? 1 : 0;
        return 0;
    }

    static float toFloat(const GraphValue& v) {
        if (std::holds_alternative<float>(v))   return std::get<float>(v);
        if (std::holds_alternative<int32_t>(v)) return static_cast<float>(std::get<int32_t>(v));
        if (std::holds_alternative<bool>(v))    return std::get<bool>(v) ? 1.f : 0.f;
        return 0.f;
    }

    struct DivOp {
        int32_t operator()(int32_t a, int32_t b) const {
            if (b == 0) { NF_LOG_WARN("GraphVM", "Integer division by zero"); return 0; }
            return a / b;
        }
        float operator()(float a, float b) const {
            if (b == 0.f) { NF_LOG_WARN("GraphVM", "Float division by zero"); return 0.f; }
            return a / b;
        }
    };

    template <typename Op>
    void execArith(const GraphInstruction& inst, Op op) {
        const auto& a = reg(inst.operandB);
        const auto& b = reg(inst.operandC);
        if (std::holds_alternative<float>(a) || std::holds_alternative<float>(b)) {
            reg(inst.operandA) = op(toFloat(a), toFloat(b));
        } else {
            reg(inst.operandA) = op(toInt(a), toInt(b));
        }
        ++m_pc;
    }
};

// ── Graph types ──────────────────────────────────────────────────

enum class GraphType : uint8_t {
    World,
    Strategy,
    Conversation,
    Behavior,
    Animation,
    Character,
    Weapon,
    Tile,
    Sound,
    UI,
    UIScreen,
    GameFlow,
    Story,
    Asset
};

// ── GraphPort / GraphNode / GraphLink / Graph ────────────────────

enum class GraphDataType : uint8_t {
    Int,
    Float,
    Bool,
    String
};

struct GraphPort {
    std::string   name;
    GraphDataType dataType = GraphDataType::Float;
    uint32_t      index    = 0;
};

struct GraphNode {
    GraphNodeID            id   = 0;
    std::string            name;
    GraphType              type = GraphType::World;
    Vec2                   position{0.f, 0.f};
    std::vector<GraphPort> inputs;
    std::vector<GraphPort> outputs;
};

struct GraphLink {
    GraphNodeID sourceNode = 0;
    uint32_t    sourcePort = 0;
    GraphNodeID targetNode = 0;
    uint32_t    targetPort = 0;
};

class Graph {
public:
    void addNode(const GraphNode& node) {
        m_nodes.push_back(node);
    }

    void removeNode(GraphNodeID id) {
        m_nodes.erase(
            std::remove_if(m_nodes.begin(), m_nodes.end(),
                           [id](const GraphNode& n) { return n.id == id; }),
            m_nodes.end());
        m_links.erase(
            std::remove_if(m_links.begin(), m_links.end(),
                           [id](const GraphLink& l) {
                               return l.sourceNode == id || l.targetNode == id;
                           }),
            m_links.end());
    }

    void addLink(const GraphLink& link) {
        m_links.push_back(link);
    }

    void removeLink(size_t index) {
        if (index < m_links.size())
            m_links.erase(m_links.begin() + static_cast<ptrdiff_t>(index));
    }

    [[nodiscard]] GraphNode* findNode(GraphNodeID id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] const GraphNode* findNode(GraphNodeID id) const {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] const std::vector<GraphNode>& nodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<GraphLink>& links() const { return m_links; }

private:
    std::vector<GraphNode> m_nodes;
    std::vector<GraphLink> m_links;
};

// ── GraphCompiler ────────────────────────────────────────────────

class GraphCompiler {
public:
    [[nodiscard]] static GraphProgram compile(const Graph& graph) {
        GraphProgram prog;

        auto sorted = topologicalSort(graph);
        if (sorted.empty() && !graph.nodes().empty()) {
            NF_LOG_WARN("GraphCompiler", "Topological sort produced no output");
            return prog;
        }

        if (sorted.size() < graph.nodes().size()) {
            NF_LOG_WARN("GraphCompiler", "Graph contains a cycle; some nodes skipped");
        }

        // Register allocation: each node's first output gets a register
        std::unordered_map<GraphNodeID, uint32_t> nodeRegister;
        uint32_t nextReg = 0;
        for (auto id : sorted) {
            if (nextReg >= kRegisterCount) {
                NF_LOG_WARN("GraphCompiler", "Out of registers; some nodes unallocated");
                break;
            }
            nodeRegister[id] = nextReg++;
        }

        for (auto id : sorted) {
            const auto* node = graph.findNode(id);
            if (!node) continue;

            uint32_t destReg = nodeRegister[id];
            const auto& name = node->name;

            if (name == "Constant") {
                // First output port's index stores the constant value index
                uint32_t constIdx = static_cast<uint32_t>(prog.constants.size());
                // Read value from node: use position.x as int constant by convention
                prog.constants.push_back(static_cast<int32_t>(node->position.x));
                prog.instructions.push_back(
                    {GraphOpCode::LoadConst, destReg, constIdx, 0});
            } else if (name == "ConstantFloat") {
                uint32_t constIdx = static_cast<uint32_t>(prog.constants.size());
                prog.constants.push_back(node->position.x);
                prog.instructions.push_back(
                    {GraphOpCode::LoadConst, destReg, constIdx, 0});
            } else if (name == "Add" || name == "Subtract" ||
                       name == "Multiply" || name == "Divide") {
                auto srcRegs = resolveInputs(graph, *node, nodeRegister);
                uint32_t src1 = srcRegs.size() > 0 ? srcRegs[0] : 0;
                uint32_t src2 = srcRegs.size() > 1 ? srcRegs[1] : 0;

                GraphOpCode op = GraphOpCode::Nop;
                if (name == "Add")      op = GraphOpCode::Add;
                if (name == "Subtract") op = GraphOpCode::Subtract;
                if (name == "Multiply") op = GraphOpCode::Multiply;
                if (name == "Divide")   op = GraphOpCode::Divide;

                prog.instructions.push_back({op, destReg, src1, src2});
            }
        }

        prog.instructions.push_back({GraphOpCode::Halt, 0, 0, 0});
        return prog;
    }

private:
    static std::vector<GraphNodeID> topologicalSort(const Graph& graph) {
        std::unordered_map<GraphNodeID, std::vector<GraphNodeID>> adj;
        std::unordered_map<GraphNodeID, uint32_t> inDegree;

        for (auto& n : graph.nodes()) {
            inDegree[n.id] = 0;
            adj[n.id];
        }
        for (auto& l : graph.links()) {
            adj[l.sourceNode].push_back(l.targetNode);
            inDegree[l.targetNode]++;
        }

        std::queue<GraphNodeID> q;
        for (auto& [id, deg] : inDegree) {
            if (deg == 0) q.push(id);
        }

        std::vector<GraphNodeID> sorted;
        while (!q.empty()) {
            auto cur = q.front(); q.pop();
            sorted.push_back(cur);
            for (auto next : adj[cur]) {
                if (--inDegree[next] == 0) q.push(next);
            }
        }
        return sorted;
    }

    static std::vector<uint32_t> resolveInputs(
        const Graph& graph, const GraphNode& node,
        const std::unordered_map<GraphNodeID, uint32_t>& nodeRegister)
    {
        std::vector<uint32_t> regs;
        for (uint32_t i = 0; i < static_cast<uint32_t>(node.inputs.size()); ++i) {
            bool found = false;
            for (auto& l : graph.links()) {
                if (l.targetNode == node.id && l.targetPort == i) {
                    auto it = nodeRegister.find(l.sourceNode);
                    if (it != nodeRegister.end()) {
                        regs.push_back(it->second);
                    }
                    found = true;
                    break;
                }
            }
            if (!found) {
                NF_LOG_WARN("GraphCompiler", "Unconnected input; defaulting to r0");
                regs.push_back(0);
            }
        }
        return regs;
    }
};

// ── GraphSerializer ──────────────────────────────────────────────

class GraphSerializer {
public:
    // ── Program serialization ────────────────────────────────────

    [[nodiscard]] static JsonValue programToJson(const GraphProgram& prog) {
        auto root = JsonValue::object();

        auto instArr = JsonValue::array();
        for (auto& inst : prog.instructions) {
            auto obj = JsonValue::object();
            obj.set("op", JsonValue(static_cast<int32_t>(inst.opcode)));
            obj.set("a",  JsonValue(static_cast<int32_t>(inst.operandA)));
            obj.set("b",  JsonValue(static_cast<int32_t>(inst.operandB)));
            obj.set("c",  JsonValue(static_cast<int32_t>(inst.operandC)));
            instArr.push(std::move(obj));
        }
        root.set("instructions", std::move(instArr));

        auto constArr = JsonValue::array();
        for (auto& c : prog.constants) {
            constArr.push(valueToJson(c));
        }
        root.set("constants", std::move(constArr));

        return root;
    }

    [[nodiscard]] static GraphProgram programFromJson(const JsonValue& json) {
        GraphProgram prog;
        if (!json.isObject()) return prog;

        const auto& instArr = json["instructions"];
        for (size_t i = 0; i < instArr.size(); ++i) {
            const auto& obj = instArr[i];
            GraphInstruction inst;
            inst.opcode  = static_cast<GraphOpCode>(obj["op"].asInt());
            inst.operandA = static_cast<uint32_t>(obj["a"].asInt());
            inst.operandB = static_cast<uint32_t>(obj["b"].asInt());
            inst.operandC = static_cast<uint32_t>(obj["c"].asInt());
            prog.instructions.push_back(inst);
        }

        const auto& constArr = json["constants"];
        for (size_t i = 0; i < constArr.size(); ++i) {
            prog.constants.push_back(valueFromJson(constArr[i]));
        }

        return prog;
    }

    // ── Graph serialization ──────────────────────────────────────

    [[nodiscard]] static JsonValue graphToJson(const Graph& graph) {
        auto root = JsonValue::object();

        auto nodeArr = JsonValue::array();
        for (auto& n : graph.nodes()) {
            auto obj = JsonValue::object();
            obj.set("id",   JsonValue(static_cast<int32_t>(n.id)));
            obj.set("name", JsonValue(n.name));
            obj.set("type", JsonValue(static_cast<int32_t>(n.type)));
            obj.set("posX", JsonValue(n.position.x));
            obj.set("posY", JsonValue(n.position.y));

            auto inArr = JsonValue::array();
            for (auto& p : n.inputs) {
                auto pobj = JsonValue::object();
                pobj.set("name",     JsonValue(p.name));
                pobj.set("dataType", JsonValue(static_cast<int32_t>(p.dataType)));
                pobj.set("index",    JsonValue(static_cast<int32_t>(p.index)));
                inArr.push(std::move(pobj));
            }
            obj.set("inputs", std::move(inArr));

            auto outArr = JsonValue::array();
            for (auto& p : n.outputs) {
                auto pobj = JsonValue::object();
                pobj.set("name",     JsonValue(p.name));
                pobj.set("dataType", JsonValue(static_cast<int32_t>(p.dataType)));
                pobj.set("index",    JsonValue(static_cast<int32_t>(p.index)));
                outArr.push(std::move(pobj));
            }
            obj.set("outputs", std::move(outArr));

            nodeArr.push(std::move(obj));
        }
        root.set("nodes", std::move(nodeArr));

        auto linkArr = JsonValue::array();
        for (auto& l : graph.links()) {
            auto obj = JsonValue::object();
            obj.set("srcNode", JsonValue(static_cast<int32_t>(l.sourceNode)));
            obj.set("srcPort", JsonValue(static_cast<int32_t>(l.sourcePort)));
            obj.set("tgtNode", JsonValue(static_cast<int32_t>(l.targetNode)));
            obj.set("tgtPort", JsonValue(static_cast<int32_t>(l.targetPort)));
            linkArr.push(std::move(obj));
        }
        root.set("links", std::move(linkArr));

        return root;
    }

    [[nodiscard]] static Graph graphFromJson(const JsonValue& json) {
        Graph graph;
        if (!json.isObject()) return graph;

        const auto& nodeArr = json["nodes"];
        for (size_t i = 0; i < nodeArr.size(); ++i) {
            const auto& obj = nodeArr[i];
            GraphNode n;
            n.id       = static_cast<GraphNodeID>(obj["id"].asInt());
            n.name     = obj["name"].asString();
            n.type     = static_cast<GraphType>(obj["type"].asInt());
            n.position = {obj["posX"].asFloat(), obj["posY"].asFloat()};

            const auto& inArr = obj["inputs"];
            for (size_t j = 0; j < inArr.size(); ++j) {
                const auto& p = inArr[j];
                GraphPort port;
                port.name     = p["name"].asString();
                port.dataType = static_cast<GraphDataType>(p["dataType"].asInt());
                port.index    = static_cast<uint32_t>(p["index"].asInt());
                n.inputs.push_back(std::move(port));
            }

            const auto& outArr = obj["outputs"];
            for (size_t j = 0; j < outArr.size(); ++j) {
                const auto& p = outArr[j];
                GraphPort port;
                port.name     = p["name"].asString();
                port.dataType = static_cast<GraphDataType>(p["dataType"].asInt());
                port.index    = static_cast<uint32_t>(p["index"].asInt());
                n.outputs.push_back(std::move(port));
            }

            graph.addNode(std::move(n));
        }

        const auto& linkArr = json["links"];
        for (size_t i = 0; i < linkArr.size(); ++i) {
            const auto& obj = linkArr[i];
            GraphLink l;
            l.sourceNode = static_cast<GraphNodeID>(obj["srcNode"].asInt());
            l.sourcePort = static_cast<uint32_t>(obj["srcPort"].asInt());
            l.targetNode = static_cast<GraphNodeID>(obj["tgtNode"].asInt());
            l.targetPort = static_cast<uint32_t>(obj["tgtPort"].asInt());
            graph.addLink(std::move(l));
        }

        return graph;
    }

private:
    [[nodiscard]] static JsonValue valueToJson(const GraphValue& v) {
        auto obj = JsonValue::object();
        if (std::holds_alternative<int32_t>(v)) {
            obj.set("type", JsonValue("int"));
            obj.set("val",  JsonValue(std::get<int32_t>(v)));
        } else if (std::holds_alternative<float>(v)) {
            obj.set("type", JsonValue("float"));
            obj.set("val",  JsonValue(std::get<float>(v)));
        } else if (std::holds_alternative<bool>(v)) {
            obj.set("type", JsonValue("bool"));
            obj.set("val",  JsonValue(std::get<bool>(v)));
        } else if (std::holds_alternative<std::string>(v)) {
            obj.set("type", JsonValue("string"));
            obj.set("val",  JsonValue(std::get<std::string>(v)));
        }
        return obj;
    }

    [[nodiscard]] static GraphValue valueFromJson(const JsonValue& json) {
        auto t = json["type"].asString();
        if (t == "int")    return GraphValue{json["val"].asInt()};
        if (t == "float")  return GraphValue{json["val"].asFloat()};
        if (t == "bool")   return GraphValue{json["val"].asBool()};
        if (t == "string") return GraphValue{json["val"].asString()};
        return GraphValue{0};
    }
};

} // namespace NF
