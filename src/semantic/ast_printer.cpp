#include "ast_printer.hpp"

static string labelOf(const AstNodePtr& node) {
    string s = astKindName(node->kind);
    if (!node->value.empty()) s += "(" + node->value + ")";
    if (!node->extra.empty()) s += "[" + node->extra + "]";
    // anotasi (cuma muncul kalau udah didekorasi)
    vector<string> anno;
    if (!node->typeName.empty()) anno.push_back("type:" + node->typeName);
    if (node->tabIndex >= 0)     anno.push_back("tab:" + to_string(node->tabIndex));
    if (node->lev >= 0)          anno.push_back("lev:" + to_string(node->lev));
    if (!anno.empty()) {
        s += "  → ";
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
        if (isLast) out << "└── ";
        else out << "├── ";
        out << labelOf(node) << "\n";
    }
    string childPrefix;
    if (isRoot) {
        childPrefix = "";
    } else if (isLast) {
        childPrefix = prefix + "    ";
    } else {
        childPrefix = prefix + "│   ";
    }
    for (size_t i = 0; i < node->children.size(); i++) {
        bool lastChild = (i == node->children.size() - 1);
        printAstImpl(node->children[i], out, childPrefix, lastChild, false);
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
