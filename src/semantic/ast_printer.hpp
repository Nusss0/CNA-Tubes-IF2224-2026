#pragma once
#include "../std.hpp"
#include "ast_node.hpp"

// ast printer
void printAst(const AstNodePtr& root);
void saveAstToFile(const AstNodePtr& root, const string& path);
