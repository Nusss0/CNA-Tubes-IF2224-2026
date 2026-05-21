#pragma once
#include "../std.hpp"

struct ParseNode {
    string label;
    vector<shared_ptr<ParseNode>> children;
    //semantic
    int tab_index = -1;
    int lev = -1;
    int block_index = -1;
    string sem_type;   // tipe semantic

    explicit ParseNode(const string& lbl) : label(lbl) {}
};

using NodePtr = shared_ptr<ParseNode>;

NodePtr makeNode(const string& label);
void addChild(const NodePtr& parent, const NodePtr& child);

void printTree(const NodePtr& root);
void saveTreeToFile(const NodePtr& root, const string& path);
