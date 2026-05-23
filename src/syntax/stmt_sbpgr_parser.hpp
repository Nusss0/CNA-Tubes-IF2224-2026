#pragma once

#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"
#include "parser_base.hpp"

class DeclarationParser;
class ExpressionParser;

// stmt/subprogram parser
class StatementSubprogramParser : public ParserBase {
public:
    StatementSubprogramParser() = default;
    explicit StatementSubprogramParser(const vector<Token>& tokens) : ParserBase(tokens) {}

    // peer parsers
    void setDeclParser(DeclarationParser* p) { declParser_ = p; }
    void setExprParser(ExpressionParser* p) { exprParser_ = p; }

    // statements
    NodePtr parseStatement();
    NodePtr parseAssignmentStatement();
    NodePtr parseIfStatement();
    NodePtr parseCaseStatement();
    NodePtr parseWhileStatement();
    NodePtr parseRepeatStatement();
    NodePtr parseForStatement();
    NodePtr parseProcedureFunctionCall();

    // params
    NodePtr parseParameterList();
    NodePtr parseVariable();
    NodePtr parseComponentVariable();
    NodePtr parseIndexList();

    // subprograms
    NodePtr parseSubprogramDeclaration();
    NodePtr parseProcedureDeclaration();
    NodePtr parseFunctionDeclaration();
    NodePtr parseBlock();
    NodePtr parseFormalParameterList();
    NodePtr parseParameterGroup();

    // delegated
    NodePtr parseCaseBlock();
    NodePtr parseExpression();
    NodePtr parseStatementList();
    NodePtr parseConstant();
    NodePtr parseDeclarationPart();
    NodePtr parseCompoundStatement();
    NodePtr parseIdentifierList();
    NodePtr parseArrayType();

private:
    // peer ptrs
    DeclarationParser* declParser_ = nullptr;
    ExpressionParser* exprParser_ = nullptr;
};
