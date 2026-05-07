#pragma once

#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"
#include "parser_base.hpp"

class DeclarationParser;
class ExpressionParser;

// parser utk statement & subprogram declaration.
// helper token diturunkan dari ParserBase. delegasi ke DeclParser/ExprParser
// lewat pointer non-owning yg di-inject Parser utama.
class StatementSubprogramParser : public ParserBase {
public:
    StatementSubprogramParser() = default;
    explicit StatementSubprogramParser(const vector<Token>& tokens) : ParserBase(tokens) {}

    // peer parser injection (dipakai utk delegasi internal)
    void setDeclParser(DeclarationParser* p) { declParser_ = p; }
    void setExprParser(ExpressionParser* p) { exprParser_ = p; }

    // parser untuk statements
    NodePtr parseStatement();
    NodePtr parseAssignmentStatement();
    NodePtr parseIfStatement();
    NodePtr parseCaseStatement();
    NodePtr parseWhileStatement();
    NodePtr parseRepeatStatement();
    NodePtr parseForStatement();
    NodePtr parseProcedureFunctionCall();

    // parser untuk parameter
    NodePtr parseParameterList();
    NodePtr parseVariable();
    NodePtr parseComponentVariable();
    NodePtr parseIndexList();

    // parser untuk deklarasi subprogram
    NodePtr parseSubprogramDeclaration();
    NodePtr parseProcedureDeclaration();
    NodePtr parseFunctionDeclaration();
    NodePtr parseBlock();
    NodePtr parseFormalParameterList();
    NodePtr parseParameterGroup();

    // method yg di-delegasi ke peer parser (expression/declaration)
    NodePtr parseCaseBlock();
    NodePtr parseExpression();
    NodePtr parseStatementList();
    NodePtr parseConstant();
    NodePtr parseDeclarationPart();
    NodePtr parseCompoundStatement();
    NodePtr parseIdentifierList();
    NodePtr parseArrayType();

private:
    // peer parsers (non-owning, di-set dari Parser utama)
    DeclarationParser* declParser_ = nullptr;
    ExpressionParser* exprParser_ = nullptr;
};
