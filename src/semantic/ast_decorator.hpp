#pragma once

#include "../std.hpp"
#include "ast_node.hpp"
#include "symbol_table.hpp"

// traversal AST pasca semantic analysis utk mengisi field
// typeName, tabIndex, lev pada tiap AstNode.
// membutuhkan symbol table yang sudah terisi penuh.
class AstDecorator {
public:
    explicit AstDecorator(const SymbolTable& sym);

    void decorate(const AstNodePtr& root);

private:
    const SymbolTable& symbols;
    int currBlock = 0;
    int currLev = 0;

    void visit(const AstNodePtr& node);

    // lookup helper
    void annotateFromTab(const AstNodePtr& node, const string& id);
    string typeNameFromCode(int code) const;
};
