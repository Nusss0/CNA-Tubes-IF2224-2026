#pragma once

#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"
#include "parser_base.hpp"

// expr parser
class ExpressionParser : public ParserBase {
public:
    ExpressionParser() = default;
    explicit ExpressionParser(const vector<Token>& tokens) : ParserBase(tokens) {}

    NodePtr parseExpression();
    NodePtr parseSimpleExpression();
    NodePtr parseTerm();
    NodePtr parseFactor();

private:
    // factor: call/var mirror
    NodePtr parseVariable();
    NodePtr parseProcedureFunctionCall();
    NodePtr parseIndexList();
};
