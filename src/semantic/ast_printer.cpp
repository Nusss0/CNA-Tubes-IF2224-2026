#include "ast_printer.hpp"

// ---- format node sesuai spec M3 ---- //

static string formatNode(const AstNodePtr& node);

static string formatNode(const AstNodePtr& node) {
    if (!node) return "null";

    switch (node->kind) {
        case AstKind::Program:
            return "ProgramNode(name: '" + node->value + "')";

        case AstKind::VarDecl: {
            string type = "unknown";
            if (!node->children.empty() && node->children[0]->kind == AstKind::TypeRef)
                type = node->children[0]->value;
            return "VarDecl(name: '" + node->value + "', type: '" + type + "')";
        }

        case AstKind::ConstDecl: {
            string val = node->children.empty() ? "null" : formatNode(node->children[0]);
            return "ConstDecl(name: '" + node->value + "', value: " + val + ")";
        }

        case AstKind::TypeDecl:
            return "TypeDecl(name: '" + node->value + "')";

        case AstKind::Assign: {
            string target = node->children.size() > 0 ? formatNode(node->children[0]) : "null";
            string value  = node->children.size() > 1 ? formatNode(node->children[1]) : "null";
            return "Assign(target: " + target + ", value: " + value + ")";
        }

        case AstKind::BinOp: {
            string left  = node->children.size() > 0 ? formatNode(node->children[0]) : "null";
            string right = node->children.size() > 1 ? formatNode(node->children[1]) : "null";
            return "BinOp(op: '" + node->value + "', left: " + left + ", right: " + right + ")";
        }

        case AstKind::UnaryOp: {
            string operand = node->children.empty() ? "null" : formatNode(node->children[0]);
            return "UnaryOp(op: '" + node->value + "', operand: " + operand + ")";
        }

        case AstKind::ProcCall: {
            string args;
            for (size_t i = 0; i < node->children.size(); ++i) {
                if (i) args += ", ";
                args += formatNode(node->children[i]);
            }
            return "ProcedureCall(name: '" + node->value + "', args: [" + args + "])";
        }

        case AstKind::FuncCall: {
            string args;
            for (size_t i = 0; i < node->children.size(); ++i) {
                if (i) args += ", ";
                args += formatNode(node->children[i]);
            }
            return "FunctionCall(name: '" + node->value + "', args: [" + args + "])";
        }

        case AstKind::Number:
            return "Num(" + node->value + ")";

        case AstKind::RealNumber:
            return "Num(" + node->value + ")";

        case AstKind::Var:
            return "Var('" + node->value + "')";

        case AstKind::StringLit:
            return "String('" + node->value + "')";

        case AstKind::CharLit:
            return "Char('" + node->value + "')";

        case AstKind::BoolLit:
            return "Bool(" + node->value + ")";

        case AstKind::TypeRef:
            return "type: '" + node->value + "'";

        case AstKind::FieldAccess: {
            string base = node->children.empty() ? "null" : formatNode(node->children[0]);
            return "FieldAccess(field: '" + node->value + "', base: " + base + ")";
        }

        case AstKind::IndexAccess: {
            string base = node->children.size() > 0 ? formatNode(node->children[0]) : "null";
            string idx  = node->children.size() > 1 ? formatNode(node->children[1]) : "null";
            return "IndexAccess(base: " + base + ", index: " + idx + ")";
        }

        case AstKind::If:
            return "If";
        case AstKind::While:
            return "While";
        case AstKind::For:
            return "For(var: '" + node->value + "', dir: '" + node->extra + "')";
        case AstKind::Repeat:
            return "Repeat";
        case AstKind::Case:
            return "Case";
        case AstKind::Compound:
            return "Block";
        case AstKind::Block:
            return "Block";
        case AstKind::Declarations:
            return "Declarations";
        case AstKind::Empty:
            return "Empty";
        case AstKind::SubprogramDecl:
            return "SubprogramDecl('" + node->value + "')";
        case AstKind::ArrayType:
            return "ArrayType";
        case AstKind::RecordType:
            return "RecordType";
        case AstKind::EnumType:
            return "EnumType";
        case AstKind::RangeType:
            return "RangeType";
        case AstKind::SubprogramHeader:
            return "SubprogramHeader";

        default:
            return astKindName(node->kind) + "(" + node->value + ")";
    }
}

// node yang child-nya sudah di-inline di label, jangan expand lagi
static bool skipChildren(AstKind k) {
    switch (k) {
        case AstKind::Assign:
        case AstKind::BinOp:
        case AstKind::UnaryOp:
        case AstKind::ProcCall:
        case AstKind::FuncCall:
        case AstKind::VarDecl:
        case AstKind::ConstDecl:
        case AstKind::TypeDecl:
        case AstKind::FieldAccess:
        case AstKind::IndexAccess:
        case AstKind::Number:
        case AstKind::RealNumber:
        case AstKind::Var:
        case AstKind::StringLit:
        case AstKind::CharLit:
        case AstKind::BoolLit:
        case AstKind::TypeRef:
        case AstKind::SubprogramHeader:
            return true;
        default:
            return false;
    }
}

static string labelOf(const AstNodePtr& node) {
    if (!node) return "";
    string s = formatNode(node);
    // anotasi (cuma muncul kalau udah didekorasi)
    vector<string> anno;
    if (!node->typeName.empty()) anno.push_back("type:" + node->typeName);
    if (node->tabIndex >= 0)     anno.push_back("tab:" + to_string(node->tabIndex));
    if (node->lev >= 0)          anno.push_back("lev:" + to_string(node->lev));
    if (!anno.empty()) {
        s += "  -> ";
        for (size_t i = 0; i < anno.size(); i++) {
            s += anno[i];
            if (i + 1 < anno.size()) s += ", ";
        }
    }
    return s;
}

static void printAstImpl(const AstNodePtr& node, ostream& out, const string& prefix, bool isLast, bool isRoot) {
    if (!node) return;
    if (isRoot) {
        out << labelOf(node) << "\n";
    } else {
        out << prefix;
        if (isLast) out << "\\-- ";
        else        out << "+-- ";
        out << labelOf(node) << "\n";
    }

    // kalau node ini child-nya sudah inline, berhenti
    if (skipChildren(node->kind)) return;

    string childPrefix;
    if (isRoot) {
        childPrefix = "";
    } else if (isLast) {
        childPrefix = prefix + "    ";
    } else {
        childPrefix = prefix + "|   ";
    }

    for (size_t i = 0; i < node->children.size(); i++) {
        auto& c = node->children[i];
        if (!c) continue;
        bool lastChild = (i + 1 == node->children.size());
        printAstImpl(c, out, childPrefix, lastChild, false);
    }
}

void printAst(const AstNodePtr& root) {
    printAstImpl(root, cout, "", true, true);
}

void saveAstToFile(const AstNodePtr& root, const string& path) {
    ofstream file(path);
    if (!file.is_open()) {
        cerr << "[ERROR] Cannot open output file: " << path << " !\n";
        return;
    }
    printAstImpl(root, file, "", true, true);
    file.close();
}
