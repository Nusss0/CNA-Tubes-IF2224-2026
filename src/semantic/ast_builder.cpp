#include "ast_builder.hpp"

AstNodePtr AstBuilder::build(const NodePtr& parseRoot) {
    errors.clear();
    if (!parseRoot) {
        reportError("Parse tree is null");
        return nullptr;
    }
    return buildProgram(parseRoot);
}

void AstBuilder::reportError(const string& msg) {
    errors.push_back(msg);
    cerr << "[AST ERROR] " << msg << "\n";
}

//placeholder
AstNodePtr AstBuilder::buildProgram(const NodePtr&)          { return nullptr; }
AstNodePtr AstBuilder::buildDeclarationPart(const NodePtr&)  { return nullptr; }
AstNodePtr AstBuilder::buildVarDeclaration(const NodePtr&)   { return nullptr; }
AstNodePtr AstBuilder::buildConstDeclaration(const NodePtr&) { return nullptr; }
AstNodePtr AstBuilder::buildTypeDeclaration(const NodePtr&)  { return nullptr; }
AstNodePtr AstBuilder::buildCompoundStatement(const NodePtr&){ return nullptr; }
AstNodePtr AstBuilder::buildStatement(const NodePtr&)        { return nullptr; }
AstNodePtr AstBuilder::buildAssignment(const NodePtr&)       { return nullptr; }
AstNodePtr AstBuilder::buildExpression(const NodePtr&)       { return nullptr; }
AstNodePtr AstBuilder::buildSimpleExpression(const NodePtr&) { return nullptr; }
AstNodePtr AstBuilder::buildTerm(const NodePtr&)             { return nullptr; }
AstNodePtr AstBuilder::buildFactor(const NodePtr&)           { return nullptr; }
AstNodePtr AstBuilder::buildVariable(const NodePtr&)         { return nullptr; }
AstNodePtr AstBuilder::buildIfStatement(const NodePtr&)      { return nullptr; }
AstNodePtr AstBuilder::buildWhileStatement(const NodePtr&)   { return nullptr; }
AstNodePtr AstBuilder::buildForStatement(const NodePtr&)     { return nullptr; }
AstNodePtr AstBuilder::buildRepeatStatement(const NodePtr&)  { return nullptr; }
AstNodePtr AstBuilder::buildCaseStatement(const NodePtr&)    { return nullptr; }
AstNodePtr AstBuilder::buildProcFuncCall(const NodePtr&)     { return nullptr; }
AstNodePtr AstBuilder::buildType(const NodePtr&)             { return nullptr; }
AstNodePtr AstBuilder::buildSubprogram(const NodePtr&)       { return nullptr; }
