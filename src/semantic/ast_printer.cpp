#include "ast_printer.hpp"

// format node

static string formatNode(const AstNodePtr& node);

static string formatNode(const AstNodePtr& node) {
    if (!node) return "null";

    switch (node->kind) {
        case AstKind::Program:
            return "ProgramNode(name: '" + node->value + "')";

        case AstKind::VarDecl:
            return "VarDecl('" + node->value + "')";

        case AstKind::ConstDecl:
            return "ConstDecl('" + node->value + "')";

        case AstKind::TypeDecl:
            return "TypeDecl('" + node->value + "')";

        case AstKind::Assign:
            return "Assign(...)";

        case AstKind::BinOp:
            return "BinOp(op: '" + node->value + "')";

        case AstKind::UnaryOp:
            return "UnaryOp(op: '" + node->value + "')";

        case AstKind::ProcCall:
            return "ProcedureCall(name: '" + node->value + "')";

        case AstKind::FuncCall:
            return "FunctionCall(name: '" + node->value + "')";

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
        case AstKind::TypeRef:
            return "type: '" + node->value + "'";

        default:
            return astKindName(node->kind) + "(" + node->value + ")";
    }
}

static string labelOf(const AstNodePtr& node, const string& base = "") {
    if (!node) return "";
    string s = base.empty() ? formatNode(node) : base;
    // annotation
    vector<string> anno;
    if (!node->typeName.empty()) anno.push_back("type:" + node->typeName);
    if (node->tabIndex >= 0)     anno.push_back("tab:" + to_string(node->tabIndex));
    if (node->lev >= 0)          anno.push_back("lev:" + to_string(node->lev));
    if (node->blockIndex >= 0)   anno.push_back("blk:" + to_string(node->blockIndex));
    if (!anno.empty()) {
        s += "  -> ";
        for (size_t i = 0; i < anno.size(); i++) {
            s += anno[i];
            if (i + 1 < anno.size()) s += ", ";
        }
    }
    return s;
}

// child label
static string childLabel(const AstNodePtr& parent, size_t idx, const AstNodePtr& child) {
    if (!parent || !child) return "";

    switch (parent->kind) {
        case AstKind::Assign:
            if (idx == 0) return "target " + formatNode(child);
            if (idx == 1) return "value "  + formatNode(child);
            break;
        case AstKind::BinOp:
            if (idx == 0) return "left "  + formatNode(child);
            if (idx == 1) return "right " + formatNode(child);
            break;
        case AstKind::UnaryOp:
            if (idx == 0) return "operand " + formatNode(child);
            break;
        case AstKind::ProcCall:
        case AstKind::FuncCall:
            return formatNode(child);
        default:
            break;
    }
    return formatNode(child);
}

// leaf check
static bool isLeaf(AstKind k) {
    switch (k) {
        case AstKind::Number:
        case AstKind::RealNumber:
        case AstKind::Var:
        case AstKind::StringLit:
        case AstKind::CharLit:
        case AstKind::BoolLit:
        case AstKind::TypeRef:
            return true;
        default:
            return false;
    }
}

static void printAstImpl(const AstNodePtr& node, ostream& out, const string& prefix, bool isLast, bool isRoot, const string& customLabel = "") {
    if (!node) return;
    string display = customLabel.empty() ? labelOf(node) : labelOf(node, customLabel);
    if (isRoot) {
        out << display << "\n";
    } else {
        out << prefix;
        if (isLast) out << "\\-- ";
        else        out << "+-- ";
        out << display << "\n";
    }

    // leaf: stop
    if (isLeaf(node->kind)) return;

    string childPrefix;
    if (isRoot) {
        childPrefix = "";
    } else if (isLast) {
        childPrefix = prefix + "    ";
    } else {
        childPrefix = prefix + "|   ";
    }

    // children
    for (size_t i = 0; i < node->children.size(); i++) {
        auto& c = node->children[i];
        if (!c) continue;
        bool lastChild = (i + 1 == node->children.size());
        string clbl = childLabel(node, i, c);
        if (clbl == formatNode(c)) {
            // default label
            printAstImpl(c, out, childPrefix, lastChild, false);
        } else {
            // custom label
            printAstImpl(c, out, childPrefix, lastChild, false, clbl);
        }
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
