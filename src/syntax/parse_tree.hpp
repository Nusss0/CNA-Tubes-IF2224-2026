#pragma once
#include "../std.hpp"

struct ParseNode {
    string label;
    vector<shared_ptr<ParseNode>> children;

    explicit ParseNode(const string& lbl) : label(lbl) {}
};

using NodePtr = shared_ptr<ParseNode>;

NodePtr makeNode(const string& label);
void addChild(const NodePtr& parent, const NodePtr& child);

void printTree(const NodePtr& root);
void saveTreeToFile(const NodePtr& root, const string& path);
