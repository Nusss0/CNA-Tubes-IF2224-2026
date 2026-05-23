#pragma once
#include "../std.hpp"
#include "../syntax/parse_tree.hpp"
#include "ast_node.hpp"

class AstBuilder {
public:
    // build ast
    AstNodePtr build(const NodePtr& parseRoot);

    bool hasError() const { return !errors.empty(); }
    const vector<string>& getErrors() const { return errors; }

private:
    vector<string> errors;

    AstNodePtr buildProgram(const NodePtr& node);
    AstNodePtr buildDeclarationPart(const NodePtr& node);
    AstNodePtr buildVarDeclaration(const NodePtr& node);
    AstNodePtr buildConstDeclaration(const NodePtr& node);
    AstNodePtr buildTypeDeclaration(const NodePtr& node);
    AstNodePtr buildCompoundStatement(const NodePtr& node);
    AstNodePtr buildStatement(const NodePtr& node);
    AstNodePtr buildAssignment(const NodePtr& node);
    AstNodePtr buildExpression(const NodePtr& node);
    AstNodePtr buildSimpleExpression(const NodePtr& node);
    AstNodePtr buildTerm(const NodePtr& node);
    AstNodePtr buildFactor(const NodePtr& node);
    AstNodePtr buildVariable(const NodePtr& node);
    AstNodePtr buildIfStatement(const NodePtr& node);
    AstNodePtr buildWhileStatement(const NodePtr& node);
    AstNodePtr buildForStatement(const NodePtr& node);
    AstNodePtr buildRepeatStatement(const NodePtr& node);
    AstNodePtr buildCaseStatement(const NodePtr& node);
    AstNodePtr buildProcFuncCall(const NodePtr& node);
    AstNodePtr buildType(const NodePtr& node);
    AstNodePtr buildSubprogram(const NodePtr& node);

    void reportError(const string& msg);
};
