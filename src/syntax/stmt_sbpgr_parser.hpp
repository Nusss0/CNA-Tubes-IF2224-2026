#pragma once

#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"

class StatementSubprogramParser {
    public:
        // konstruktor
        explicit StatementSubprogramParser() = default;
        explicit StatementSubprogramParser(const vector<Token> &tokens);

        // setter dan getter
        void setTokens(const vector<Token> &tokens);
        void setPosition(size_t pos);
        size_t getPosition() const;

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

        // parse untuk deklarasi program
        NodePtr parseSubprogramDeclaration();
        NodePtr parseProcedureDeclaration();
        NodePtr parseFunctionDeclaration();
        NodePtr parseBlock();
        NodePtr parseFormalParameterList();
        NodePtr parseParameterGroup();

        // method untuk parse eksternal
        NodePtr parseCaseBlock();
        NodePtr parseExpression();
        NodePtr parseStatementList();
        NodePtr parseConstant();
        NodePtr parseDeclarationPart();
        NodePtr parseCompoundStatement();
        NodePtr parseIdentifierList();
        NodePtr parseArrayType();

    private:
        // atribut
        vector<Token> tokens;
        size_t pos = 0;

        // helper untuk mengecek token
        Token currentToken() const;
        Token peek(int offset = 1) const;
        bool isAtEnd() const;
        void advance();
        bool matchType(const string &type);
        bool matchValue(const string &value);
        void consume(const string &expectedType, const string &errorMessage);
};