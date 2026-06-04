#include "tac.hpp"
#include <cctype>
#include <algorithm>

using namespace std;

TacGenerator::TacGenerator() : labelCounter(0), nextAddr(3) {}

string TacGenerator::newLabel() {
    return "L" + to_string(++labelCounter);
}

string TacGenerator::lower(const string& s) {
    string out = s;
    for (auto& c : out) c = static_cast<char>(tolower((unsigned char)c));
    return out;
}

void TacGenerator::emit(const string& op, const vector<string>& args, const string& label) {
    TacInstr i;
    i.op = op;
    i.args = args;
    i.label = label;
    code.push_back(i);
}

void TacGenerator::emitLabel(const string& labelName) {
    TacInstr i;
    i.op = "LABEL";
    i.label = labelName;
    code.push_back(i);
}

// ---------------------------------------------------------------------------
// operator string -> OPR opcode (per Milestone 4 instruction table)
// ---------------------------------------------------------------------------
int TacGenerator::oprCode(const string& op) {
    if (op == "+")   return 2;   // ADD
    if (op == "-")   return 3;   // SUB
    if (op == "*")   return 4;   // MUL
    if (op == "/")   return 5;   // DIV
    if (op == "div") return 5;   // DIV (integer division decided at runtime)
    if (op == "mod") return 6;   // MOD
    if (op == "=")   return 7;   // EQL
    if (op == "<>")  return 8;   // NEQ
    if (op == "<")   return 9;   // LSS
    if (op == ">=")  return 10;  // GEQ
    if (op == ">")   return 11;  // GTR
    if (op == "<=")  return 12;  // LEQ
    // NOTE: and/or/not have no opcode in the M4 spec set -> handled later.
    return -1;
}

// ---------------------------------------------------------------------------
// address mapping: assign every declared variable an address starting at 3
// ---------------------------------------------------------------------------
void TacGenerator::collectDecls(const AstNodePtr& program) {
    if (!program) return;
    for (auto& child : program->children) {
        if (!child) continue;
        if (child->kind != AstKind::Declarations) continue;
        for (auto& decl : child->children) {
            if (!decl) continue;
            if (decl->kind == AstKind::VarDecl && !decl->value.empty()) {
                string key = lower(decl->value);
                if (addrMap.find(key) == addrMap.end()) {
                    addrMap[key] = nextAddr++;
                }
            }
            // TODO: ConstDecl inlining / nested scopes are out of scope for M4 expr/assign.
        }
    }
}

int TacGenerator::addrOf(const AstNodePtr& varNode) const {
    if (!varNode) return -1;
    auto it = addrMap.find(lower(varNode->value));
    if (it == addrMap.end()) return -1;
    return it->second;
}

// ---------------------------------------------------------------------------
// top-level generation
// ---------------------------------------------------------------------------
void TacGenerator::generate(const AstNodePtr& root) {
    code.clear();
    errors.clear();
    labelCounter = 0;
    nextAddr = 3;
    addrMap.clear();
    labelMap.clear();

    if (!root) return;

    if (root->kind == AstKind::Program) {
        // 1) map declared variables to addresses (3, 4, 5, ...)
        collectDecls(root);

        // 2) initiate memory: 3 reserved cells + one cell per variable
        int varCount = static_cast<int>(addrMap.size());
        emit("INT", {"0", to_string(3 + varCount)});

        // 3) generate the main body (last child = main Compound)
        AstNodePtr body;
        for (auto& c : root->children) {
            if (c && c->kind == AstKind::Compound) body = c;
        }
        if (!body && !root->children.empty()) body = root->children.back();
        if (body) genNode(body);
    } else {
        genNode(root);
    }

    // 4) close program
    emit("RET", {"0", "0"});

    // 5) resolve labels to line numbers, then drop the LABEL pseudo-ops so the
    //    instruction list is directly executable (jump targets are dense line
    //    numbers; keeping LABEL entries would offset every index after them).
    resolveLabels();
    code.erase(std::remove_if(code.begin(), code.end(),
                              [](const TacInstr& i) { return i.op == "LABEL"; }),
               code.end());
}

void TacGenerator::resolveLabels() {
    // record label positions (1-based instruction line, skipping LABEL pseudo-ops)
    labelMap.clear();
    int instrNo = 1;
    for (size_t i = 0; i < code.size(); ++i) {
        if (!code[i].label.empty()) {
            labelMap[code[i].label] = instrNo;
        }
        if (code[i].op != "LABEL") {
            instrNo++;
        }
    }

    // resolve jump targets into numeric line numbers
    for (auto& instr : code) {
        if (instr.op == "JMP" || instr.op == "JPC") {
            if (!instr.args.empty()) {
                string lbl = instr.args.back();
                auto it = labelMap.find(lbl);
                instr.target = (it != labelMap.end()) ? it->second : -1;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// printing: "<line> <OP> <lev> <operand>"   (0-based line numbers, sesuai spek:
// "0 INT 0 5", "1 LIT 0 10", ...). Target jump dicetak 0-based pula agar
// konsisten; eksekusi internal tetap memakai nomor 1-based (lihat resolveJump).
// ---------------------------------------------------------------------------
void TacGenerator::print(ostream& out) const {
    int instrNo = 0;
    for (const auto& instr : code) {
        if (instr.op == "LABEL") continue;  // labels resolved to line numbers, not printed

        out << instrNo << " ";
        if (instr.op == "JMP" || instr.op == "JPC") {
            out << instr.op << " 0 " << (instr.target - 1);  // 1-based -> 0-based display
        } else {
            out << instr.op;
            for (const auto& a : instr.args) out << " " << a;
        }
        out << "\n";
        instrNo++;
    }
}

// ---------------------------------------------------------------------------
// node dispatch
// ---------------------------------------------------------------------------
void TacGenerator::genNode(const AstNodePtr& node) {
    if (!node) return;
    switch (node->kind) {
        case AstKind::Compound: genCompound(node); break;
        case AstKind::Assign:   genAssign(node);   break;
        case AstKind::If:       genIf(node);       break;
        case AstKind::While:    genWhile(node);    break;
        case AstKind::ProcCall: {
            string name = lower(node->value);
            if (name == "writeln") { genWrite(node, 14); break; }
            if (name == "write")   { genWrite(node, 13); break; }
            // TODO: user-defined procedures (CAL) are out of scope here.
            break;
        }
        case AstKind::Empty:
            break;
        default:
            for (auto& c : node->children) genNode(c);
            break;
    }
}

void TacGenerator::genCompound(const AstNodePtr& node) {
    for (auto& c : node->children) genNode(c);
}

void TacGenerator::genAssign(const AstNodePtr& node) {
    if (node->children.size() < 2) return;
    auto target = node->children[0];
    auto expr   = node->children[1];

    genExpr(expr);                          // push RHS value onto the stack
    emit("STO", {"0", to_string(addrOf(target))});
}

void TacGenerator::genWrite(const AstNodePtr& node, int oprNo) {
    // ProcCall children are the argument expressions. Each argument is pushed
    // then consumed by a WRT (13). For writeln, only the final argument is
    // terminated with WRTLN (14) so the newline appears once, after all values.
    const size_t n = node->children.size();
    if (n == 0) return;  // bare write()/writeln(): nothing to push -> no-op (avoid underflow)
    for (size_t i = 0; i < n; ++i) {
        genExpr(node->children[i]);
        bool last = (i + 1 == n);
        int code = (oprNo == 14 && last) ? 14 : 13;  // newline only after the last writeln arg
        emit("OPR", {"0", to_string(code)});
    }
}

// ---------------------------------------------------------------------------
// expression generation: emits instructions, leaves one value on top of stack
// ---------------------------------------------------------------------------
void TacGenerator::genExpr(const AstNodePtr& node) {
    if (!node) return;
    switch (node->kind) {
        case AstKind::Number:
        case AstKind::RealNumber:
        case AstKind::CharLit:
        case AstKind::StringLit:
        case AstKind::BoolLit:
            emit("LIT", {"0", node->value});
            break;

        case AstKind::Var:
            emit("LOD", {"0", to_string(addrOf(node))});
            break;

        case AstKind::UnaryOp: {
            if (node->children.empty()) break;
            genExpr(node->children[0]);
            if (node->value == "-") {
                emit("OPR", {"0", "1"});      // NEG
            } else if (node->value == "not") {
                // "not" tak punya opcode di set instruksi M4
                errors.push_back("[ERROR] operator 'not' has no opcode in the M4 instruction set !");
            }
            break;
        }

        case AstKind::BinOp: {
            if (node->children.size() < 2) break;
            genExpr(node->children[0]);        // left pushed first
            genExpr(node->children[1]);        // right pushed second
            int code = oprCode(node->value);
            if (code >= 0) {
                emit("OPR", {"0", to_string(code)});
            } else {
                // operator tanpa opcode di set instruksi M4 (mis. and/or)
                errors.push_back("[ERROR] operator '" + node->value
                    + "' has no opcode in the M4 instruction set !");
            }
            break;
        }

        default:
            // unsupported expression kinds (FuncCall, IndexAccess, ...) -> traverse for now
            for (auto& c : node->children) genExpr(c);
            break;
    }
}

// ---------------------------------------------------------------------------
// control flow (light-touch: condition is left on the stack, JPC pops it)
// ---------------------------------------------------------------------------
void TacGenerator::genIf(const AstNodePtr& node) {
    if (node->children.empty()) return;
    auto cond     = node->children[0];
    auto thenNode = (node->children.size() >= 2) ? node->children[1] : nullptr;
    auto elseNode = (node->children.size() >= 3) ? node->children[2] : nullptr;

    string elseLbl = newLabel();
    string endLbl  = newLabel();

    genExpr(cond);                       // push condition
    emit("JPC", {"0", elseLbl});         // jump to else if false

    if (thenNode) genNode(thenNode);
    if (elseNode) emit("JMP", {"0", endLbl});

    emitLabel(elseLbl);
    if (elseNode) genNode(elseNode);

    emitLabel(endLbl);
}

void TacGenerator::genWhile(const AstNodePtr& node) {
    if (node->children.empty()) return;
    auto cond = node->children[0];
    auto body = (node->children.size() >= 2) ? node->children[1] : nullptr;

    string startLbl = newLabel();
    string endLbl   = newLabel();

    emitLabel(startLbl);
    genExpr(cond);                       // push condition
    emit("JPC", {"0", endLbl});          // exit loop if false
    if (body) genNode(body);

    emit("JMP", {"0", startLbl});        // loop back
    emitLabel(endLbl);
}
