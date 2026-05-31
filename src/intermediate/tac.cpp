#include "tac.hpp"

using namespace std;

TacGenerator::TacGenerator() : tempCounter(0), labelCounter(0) {}

string TacGenerator::newTemp() {
    return "t" + to_string(++tempCounter);
}

string TacGenerator::newLabel() {
    return "L" + to_string(++labelCounter);
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

void TacGenerator::generate(const AstNodePtr& root) {
    code.clear();
    tempCounter = 0;
    labelCounter = 0;
    labelMap.clear();

    if (!root) return;
    if (root->kind == AstKind::Program) {
        if (!root->children.empty()) genNode(root->children.back());
    } else {
        genNode(root);
    }

    resolveLabels();
}

void TacGenerator::resolveLabels() {
    // record label positions
    labelMap.clear();
    int instrNo = 1;
    for (size_t i = 0; i < code.size(); ++i) {
        if (!code[i].label.empty()) {
            labelMap[code[i].label] = instrNo;
        }
        // only count non-label
        if (code[i].op != "LABEL") {
            instrNo++;
        }
    }

    // resolve jump targets into numeric targets
    for (auto& instr : code) {
        if (instr.op == "JMP" || instr.op == "JPC") {
            if (!instr.args.empty()) {
                string lbl = instr.args.back();
                auto it = labelMap.find(lbl);
                if (it != labelMap.end()) {
                    instr.target = it->second;
                } else {
                    instr.target = -1; // unresolved
                }
            }
        }
    }
}

void TacGenerator::print(ostream& out) const {
    int instrNo = 1;
    for (size_t i = 0; i < code.size(); ++i) {
        const auto& instr = code[i];
        // print label if present
        if (!instr.label.empty()) {
            out << instr.label << ":\n";
        }
        
        if (instr.op == "LABEL") {
            continue;
        }

        out << instrNo << "\t";
        if (instr.op == "ASSIGN") {
            // dest, src
            if (instr.args.size() >= 2) {
                out << instr.args[0] << " = " << instr.args[1];
            }
        } else if (instr.op == "BINOP") {
            // dest, left, op, right
            if (instr.args.size() >= 4) {
                out << instr.args[0] << " = " << instr.args[1] << " " << instr.args[2] << " " << instr.args[3];
            }
        } else if (instr.op == "UNOP") {
            // dest, op, src
            if (instr.args.size() >= 3) {
                out << instr.args[0] << " = " << instr.args[1] << " " << instr.args[2];
            }
        } else if (instr.op == "JMP") {
            if (instr.target >= 0) {
                out << "JMP " << instr.target;
            } else if (!instr.args.empty()) {
                out << "JMP " << instr.args.back();
            }
        } else if (instr.op == "JPC") {
            // cond, label
            if (instr.target >= 0 && !instr.args.empty()) {
                out << "JPC " << instr.args[0] << " -> " << instr.target;
            } else if (instr.args.size() >= 2) {
                out << "JPC " << instr.args[0] << " -> " << instr.args[1];
            }
        } else if (instr.op == "CALL") {
            if (!instr.args.empty()) {
                out << "CALL " << instr.args[0];
            }
        } else {
            out << instr.op;
            for (auto& a : instr.args) {
                out << " " << a;
            }
        }

        out << "\n";
        instrNo++;
    }
}

// generation helpers
void TacGenerator::genNode(const AstNodePtr& node) {
    if (!node) return;
    switch (node->kind) {
        case AstKind::Compound: genCompound(node); break;
        case AstKind::Assign: genAssign(node); break;
        case AstKind::If: genIf(node); break;
        case AstKind::While: genWhile(node); break;
        case AstKind::ProcCall:
            if (!node->value.empty()) emit("CALL", {node->value});
            break;
        case AstKind::Empty:
            // no-op
            break;
        default:
            // if has children, traverse
            for (auto& c : node->children) {
                genNode(c);
            }
            break;
    }
}

void TacGenerator::genCompound(const AstNodePtr& node) {
    for (auto& c : node->children) {
        genNode(c);
    }
}

void TacGenerator::genAssign(const AstNodePtr& node) {
    if (node->children.size() < 2) return;
    auto target = node->children[0];
    auto expr = node->children[1];
    string rhs = genExpr(expr);
    string lhs;
    if (target->kind == AstKind::Var) {
        lhs = target->value;
    } else {
        lhs = "?";
    }

    emit("ASSIGN", {lhs, rhs});
}

void TacGenerator::genIf(const AstNodePtr& node) {
    // cond, then, else
    if (node->children.empty()) return;
    auto cond = node->children[0];
    auto thenNode = (node->children.size() >= 2) ? node->children[1] : nullptr;
    auto elseNode = (node->children.size() >= 3) ? node->children[2] : nullptr;

    string elseLbl = newLabel();
    string endLbl = newLabel();

    string condVar = genExpr(cond);
    // jump to else if cond is false
    emit("JPC", {condVar, elseLbl});

    if (thenNode) {
        genNode(thenNode);
    }

    // jump to end (if there is else)
    if (elseNode) {
        emit("JMP", {endLbl});
    }

    emitLabel(elseLbl);
    if (elseNode) {
        genNode(elseNode);
    }

    emitLabel(endLbl);
}

void TacGenerator::genWhile(const AstNodePtr& node) {
    // cond, body
    if (node->children.empty()) return;
    auto cond = node->children[0];
    auto body = (node->children.size() >= 2) ? node->children[1] : nullptr;

    string startLbl = newLabel();
    string endLbl = newLabel();

    emitLabel(startLbl);
    string condVar = genExpr(cond);
    emit("JPC", {condVar, endLbl});
    if (body) {
        genNode(body);
    }

    emit("JMP", {startLbl});
    emitLabel(endLbl);
}

string TacGenerator::genExpr(const AstNodePtr& node) {
    if (!node) return "";
    switch (node->kind) {
        case AstKind::Number:
        case AstKind::RealNumber:
        case AstKind::CharLit:
        case AstKind::StringLit:
        case AstKind::BoolLit:
            return node->value;
        case AstKind::Var:
            return node->value;
        case AstKind::UnaryOp: {
            if (node->children.empty()) return "";
            string src = genExpr(node->children[0]);
            string dst = newTemp();
            emit("UNOP", {dst, node->value, src});
            return dst;
        }
        case AstKind::BinOp: {
            if (node->children.size() < 2) return "";
            string l = genExpr(node->children[0]);
            string r = genExpr(node->children[1]);
            string dst = newTemp();
            emit("BINOP", {dst, l, node->value, r});
            return dst;
        }
        case AstKind::ProcCall:
            if (!node->value.empty()) {
                emit("CALL", {node->value});
            }

            return "";
        default:
            // traverse children and return last
            string last;
            for (auto& c : node->children) {
                last = genExpr(c);
            }
            
            return last;
    }
}
