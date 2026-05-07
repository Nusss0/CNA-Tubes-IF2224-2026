#pragma once

#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"

class ExpressionParser {
    public:
        // konstruktor
        explicit ExpressionParser() = default;
        explicit ExpressionParser(const vector<Token> &tokens);

        // setter dan getter
        void setTokens(const vector<Token> &tokens);
        void setPosition(size_t pos);
        size_t getPosition() const;

        // error-state API (sama dgn stmtParser/declParser)
        const string& error() const;
        bool hasError() const;
        void clearError();

        // parser untuk statements
        NodePtr parseExpression();
        NodePtr parseSimpleExpression();
        NodePtr parseTerm();
        NodePtr parseFactor();

    private:
        NodePtr parseVariable();
        NodePtr parseProcedureFunctionCall();
        NodePtr parseIndexList();
        // atribut
        vector<Token> tokens;
        size_t pos = 0;
        string errorMessage;

        // helper untuk mengecek token
        Token currentToken() const;
        Token peek(int offset = 1) const;
        bool isAtEnd() const;
        void advance();
        bool matchType(const string &type);
        bool matchValue(const string &value);
        void consume(const string &expectedType, const string &errorMessage);
};