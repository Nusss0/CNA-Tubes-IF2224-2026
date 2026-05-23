#include "ast_printer.hpp"

//role node dlm konteks parent (utk labeling sesuai spec hal. 23)
enum class Role {
    Root,
    AssignTarget,    // 'target a' di Assign
    AssignValue,     // 'value 5' di Assign
    BinOpOperand,    // ''a'' / 10 (no kind prefix) di BinOp/UnaryOp
    Generic
};

static string formatLiteralValue(const string& kind, const string& val) {
    if (kind == "Number" || kind == "RealNumber") return val;
    if (kind == "Char" || kind == "String")      return "'" + val + "'";
    if (kind == "Bool")                          return val;
    return val;
}

static string nodeLabel(const AstNodePtr& node, Role role) {
    string kindStr = astKindName(node->kind);
    AstKind k = node->kind;
    string& v = node->value;

    //label spesifik per-kind & role
    switch (k) {
        case AstKind::Program:
            return "ProgramNode(name: '" + v + "')";
        case AstKind::Block:
            return "Declarations";
        case AstKind::Compound:
            return "Block";
        case AstKind::VarDecl:
            return "VarDecl('" + v + "')";
        case AstKind::ConstDecl:
            return "ConstDecl('" + v + "')";
        case AstKind::TypeDecl:
            return "TypeDecl('" + v + "')";
        case AstKind::Assign: {
            string label = "Assign";
            //ringkasan 'a' := <value> hanya utk kasus sederhana (target ident, value literal/Var/simple BinOp)
            if (node->children.size() >= 2) {
                auto& target = node->children[0];
                auto& value = node->children[1];
                string tgt = (target->kind == AstKind::Var) ? ("'" + target->value + "'") : "";
                string val;
                auto leafVal = [](const AstNodePtr& n) -> string {
                    if (!n) return "";
                    if (n->kind == AstKind::Number || n->kind == AstKind::RealNumber) return n->value;
                    if (n->kind == AstKind::Var) return n->value;
                    if (n->kind == AstKind::BoolLit) return n->value;
                    return "";
                };
                if (value->kind == AstKind::Number || value->kind == AstKind::RealNumber || value->kind == AstKind::Var || value->kind == AstKind::BoolLit) {
                    val = leafVal(value);
                } else if (value->kind == AstKind::UnaryOp && value->value == "-" && value->children.size() == 1) {
                    string inner = leafVal(value->children[0]);
                    if (!inner.empty()) val = "-" + inner;
                } else if (value->kind == AstKind::BinOp && value->children.size() >= 2) {
                    string lv = leafVal(value->children[0]);
                    string rv = leafVal(value->children[1]);
                    if (!lv.empty() && !rv.empty()) val = lv + value->value + rv;
                }
                if (!tgt.empty() && !val.empty()) label += "(" + tgt + " := " + val + ")";
            }
            return label;
        }
        case AstKind::BinOp:
            return "BinOp '" + v + "'";
        case AstKind::UnaryOp:
            return "UnaryOp '" + v + "'";
        case AstKind::Var:
            if (role == Role::AssignTarget) return "target '" + v + "'";
            if (role == Role::BinOpOperand) return "'" + v + "'";
            return "Var '" + v + "'";
        case AstKind::Number:
        case AstKind::RealNumber:
        case AstKind::CharLit:
        case AstKind::StringLit:
        case AstKind::BoolLit: {
            string val = formatLiteralValue(astKindName(k), v);
            if (role == Role::AssignValue)  return "value " + val;
            if (role == Role::BinOpOperand) return val;
            return kindStr + "(" + val + ")";
        }
        case AstKind::ProcCall:
            return v + "(...)";
        case AstKind::FuncCall:
            return v + "(...)";
        case AstKind::If:               return "If";
        case AstKind::While:            return "While";
        case AstKind::For:              return "For(" + v + ")[" + node->extra + "]";
        case AstKind::Repeat:           return "Repeat";
        case AstKind::Case:             return "Case";
        case AstKind::Empty:            return "Empty";
        case AstKind::SubprogramDecl: {
            string kind = node->extra.empty() ? "Subprogram" : (node->extra == "function" ? "FuncDecl" : "ProcDecl");
            return kind + "('" + v + "')";
        }
        case AstKind::FieldAccess:      return "FieldAccess(" + v + ")";
        case AstKind::IndexAccess:      return "IndexAccess";
        case AstKind::TypeRef:          return "TypeRef(" + v + ")";
        case AstKind::ArrayType:        return "ArrayType";
        case AstKind::RecordType:       return "RecordType";
        case AstKind::EnumType:         return "EnumType";
        case AstKind::RangeType:        return "RangeType";
        case AstKind::SubprogramHeader: return "SubprogramHeader";
    }
    return kindStr;
}

static string annotation(const AstNodePtr& node) {
    //predefined: tampilkan flag + tab index aja, tanpa type/lev
    if (node->predefined && node->tabIndex >= 0) {
        return " → predefined, tab_index:" + to_string(node->tabIndex);
    }
    //urutan spec: tab_index, type, lev, block_index
    vector<string> parts;
    if (node->tabIndex >= 0)   parts.push_back("tab_index:" + to_string(node->tabIndex));
    if (!node->typeName.empty()) parts.push_back("type:" + node->typeName);
    if (node->lev >= 0)        parts.push_back("lev:" + to_string(node->lev));
    if (node->blockIndex >= 0) parts.push_back("block_index:" + to_string(node->blockIndex));
    if (parts.empty()) return "";
    string out = " → ";
    for (size_t i = 0; i < parts.size(); i++) {
        out += parts[i];
        if (i + 1 < parts.size()) out += ", ";
    }
    return out;
}

//tentukan role child berdasarkan parent kind & posisi
static Role childRole(AstKind parentKind, size_t childIdx) {
    if (parentKind == AstKind::Assign) {
        if (childIdx == 0) return Role::AssignTarget;
        if (childIdx == 1) return Role::AssignValue;
    }
    if (parentKind == AstKind::BinOp || parentKind == AstKind::UnaryOp) {
        return Role::BinOpOperand;
    }
    return Role::Generic;
}

static void printAstImpl(const AstNodePtr& node, ostream& out, const string& prefix,
                         bool isLast, bool isRoot, Role role) {
    if (!node) return;
    string label = nodeLabel(node, role) + annotation(node);
    if (isRoot) {
        out << label << "\n";
    } else {
        out << prefix;
        out << (isLast ? "└─ " : "├─ ");
        out << label << "\n";
    }
    string childPrefix;
    if (isRoot) childPrefix = "";
    else if (isLast) childPrefix = prefix + "   ";
    else childPrefix = prefix + "│  ";

    for (size_t i = 0; i < node->children.size(); i++) {
        bool lastChild = (i == node->children.size() - 1);
        Role r = childRole(node->kind, i);
        printAstImpl(node->children[i], out, childPrefix, lastChild, false, r);
    }
}

void printAst(const AstNodePtr& root) {
    printAstImpl(root, cout, "", true, true, Role::Root);
}

void saveAstToFile(const AstNodePtr& root, const string& path) {
    ofstream file(path);
    if (!file.is_open()) {
        cerr << "[ERROR] Cannot open output file: " << path << " !\n";
        return;
    }
    printAstImpl(root, file, "", true, true, Role::Root);
    file.close();
}
