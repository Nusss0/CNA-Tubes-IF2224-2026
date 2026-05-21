#pragma once
#include "../std.hpp"
#include "ast_node.hpp"

// box-drawing ala parse_tree::printTree, tp utk AST node.
// kalau decorated bakal ditambahin anotasi.
void printAst(const AstNodePtr& root);
void saveAstToFile(const AstNodePtr& root, const string& path);
