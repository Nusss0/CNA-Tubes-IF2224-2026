#include "parse_tree.hpp"

NodePtr makeNode(const string& label) {
    return make_shared<ParseNode>(label);
}

void addChild(const NodePtr& parent, const NodePtr& child) {
    if (child) parent->children.push_back(child);
}

static void printTreeImpl(const NodePtr& node, ostream& out, const string& prefix, bool isLast, bool isRoot) {
    if (!node) return;

    if (isRoot) {
        out << node->label << "\n";
    } else {
        out << prefix << (isLast ? "└── " : "├── ") << node->label << "\n";
    }

    string childPrefix = isRoot ? "" : prefix + (isLast ? "    " : "│   ");

    for (size_t i = 0; i < node->children.size(); i++) {
        bool lastChild = (i == node->children.size() - 1);
        printTreeImpl(node->children[i], out, childPrefix, lastChild, false);
    }
}

void printTree(const NodePtr& root) {
    printTreeImpl(root, cout, "", true, true);
}

void saveTreeToFile(const NodePtr& root, const string& path) {
    ofstream file(path);
    if (!file.is_open()) {
        cerr << "[ERROR] Cannot open output file: " << path << "\n";
        return;
    }
    printTreeImpl(root, file, "", true, true);
    file.close();
}
