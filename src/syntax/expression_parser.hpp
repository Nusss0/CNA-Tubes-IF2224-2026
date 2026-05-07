#pragma once

#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"
#include "parser_base.hpp"

// parser khusus expression: <expression>, <simple-expression>, <term>, <factor>.
// helper token (peek/check/advance/match/consume) diturunkan dari ParserBase.
class ExpressionParser : public ParserBase {
public:
    ExpressionParser() = default;
    explicit ExpressionParser(const vector<Token>& tokens) : ParserBase(tokens) {}

    NodePtr parseExpression();
    NodePtr parseSimpleExpression();
    NodePtr parseTerm();
    NodePtr parseFactor();

private:
    // factor bisa ngandung ident yg sebenernya call/variabel,
    // jadi parse logic-nya di-mirror sebagian dari StmtParser
    NodePtr parseVariable();
    NodePtr parseProcedureFunctionCall();
    NodePtr parseIndexList();
};
