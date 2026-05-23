#pragma once

#include "../std.hpp"
#include "../syntax/parse_tree.hpp"
#include "symbol_table.hpp"
#include "type_check.hpp"

// semantic type

class SemanticAnalyzer {
public:
    // ctor
    SemanticAnalyzer();

    // analyze
    void analyze(const NodePtr& root);

    // error check
    bool hasErrors() const { return !errors.empty(); }

    // errors
    const vector<string>& getErrors() const { return errors; }

    // symbol table
    const SymbolTable& getSymbols() const { return symbols; }

    // print decorated
    void printDecoratedTree(const NodePtr& root, ostream& out) const;

    // dump
    void printTabDump(ostream& out) const;


    void printBTabDump(ostream& out) const;


    void printATabDump(ostream& out) const;

private:
    SymbolTable symbols;
    vector<string> errors;

    // visit
    SemanticType visit(const NodePtr& node);

    // visit nodes
    SemanticType visitProgram(const NodePtr& node);

    SemanticType visitProgramHeader(const NodePtr& node);

    SemanticType visitDeclarationPart(const NodePtr& node);
    SemanticType visitConstDeclaration(const NodePtr& node);

    SemanticType visitTypeDeclaration(const NodePtr& node);
    SemanticType visitVarDeclaration(const NodePtr& node);

    SemanticType visitSubprogramDeclaration(const NodePtr& node);

    SemanticType visitProcedureDeclaration(const NodePtr& node);

    SemanticType visitFunctionDeclaration(const NodePtr& node);

    SemanticType visitBlock(const NodePtr& node, bool pushScope);

    SemanticType visitCompoundStatement(const NodePtr& node);

    SemanticType visitStatementList(const NodePtr& node);

    SemanticType visitAssignmentStatement(const NodePtr& node);

    SemanticType visitIfStatement(const NodePtr& node);

    SemanticType visitWhileStatement(const NodePtr& node);
    SemanticType visitForStatement(const NodePtr& node);

    SemanticType visitRepeatStatement(const NodePtr& node);
    SemanticType visitCaseStatement(const NodePtr& node);

    SemanticType visitProcedureFunctionCall(const NodePtr& node);
    SemanticType visitVariable(const NodePtr& node);

    SemanticType visitComponentVariable(const NodePtr& node, SemanticType baseType);

    SemanticType visitExpression(const NodePtr& node);

    SemanticType visitSimpleExpression(const NodePtr& node);

    SemanticType visitTerm(const NodePtr& node);

    SemanticType visitFactor(const NodePtr& node);
    SemanticType visitConstant(const NodePtr& node);

    SemanticType visitTypeNode(const NodePtr& node);

    SemanticType visitRange(const NodePtr& node);

    SemanticType visitEnumerated(const NodePtr& node);

    SemanticType visitArrayType(const NodePtr& node);

    SemanticType visitRecordType(const NodePtr& node);

    SemanticType visitFormalParameterList(const NodePtr& node);

    SemanticType visitParameterGroup(const NodePtr& node);

    SemanticType visitIdentifierList(const NodePtr& node, const SemanticType& type, const string& obj);

    // token check
    static bool isTokenLabel(const string& label, const string& tokenType, string* value = nullptr);

    // token value
    static string extractTokenValue(const string& label);

    // type name
    static string typeName(const SemanticType& type);

    // type code name
    static string TypeName(int type);

    // basic type
    static SemanticType basicType(const string& name);

    // numeric check
    static bool isNumeric(const SemanticType& type);

    // boolean check
    static bool isBoolean(const SemanticType& type);

    // same type
    static bool sameType(const SemanticType& lhs, const SemanticType& rhs);

    // assign compat
    static bool assignmentCompatible(const SemanticType& lhs, const SemanticType& rhs);

    // binary op
    SemanticType applyBinary(const string& op, const SemanticType& a, const SemanticType& b);

    // lookup type
    SemanticType lookupTypeByName(const string& name) const;

    // report error
    void reportError(const string& message);

    // annotate
    void annotate(const NodePtr& node, const SemanticType& type, int tabIndex = -1);

    // print impl
    void printDecoratedTreeImpl(const NodePtr& node, ostream& out, const string& prefix, bool isLast, bool isRoot) const;
    
    // decorated label
    string decoratedLabel(const NodePtr& node) const;
};