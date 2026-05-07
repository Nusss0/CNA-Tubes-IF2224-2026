#include "expression_parser.hpp"
#include <iostream>

ExpressionParser::ExpressionParser(const vector<Token>& tokens) {
    this->tokens = tokens;
    this->pos = 0;
}

void ExpressionParser::setTokens(const vector<Token>& tokens) {
    this->tokens = tokens;
    this->pos = 0;
}

void ExpressionParser::setPosition(size_t pos) {
    this->pos = pos;
}

size_t ExpressionParser::getPosition() const {
    return pos;
}

NodePtr ExpressionParser::parseFactor(){
    NodePtr node = makeNode("factor");
    Token t = currentToken();
    if (t.type == "intcon" || t.type == "realcon" || t.type == "charcon" || t.type == "string") {
        addChild(node, makeNode(t.value));
        advance();
    } 
    else if (t.type == "ident") {
        addChild(node, makeNode("ident(" + t.value + ")"));
        advance();
    } 
    else if (t.type == "lparent") {
        consume("lparent", "Expected '('");
        addChild(node, makeNode("lparent"));
        addChild(node, parseExpression());
        consume("rparent", "Expected ')'");
        addChild(node, makeNode("rparent"));
    }
    else if (t.type == "notsy") {
        consume("notsy", "Expected 'not'");
        addChild(node, makeNode("notsy"));
        addChild(node, parseFactor());
    }
    else {
        cout << "[ERROR] Unexpected token: " << t.type << " with value: " << t.value << "\n";
    } 
    return node;
}

NodePtr ExpressionParser::parseTerm() {
    NodePtr node = makeNode("term");
    addChild(node, parseFactor());
    while (currentToken().type == "times" || currentToken().type == "idiv" || currentToken().type == "rdiv" || currentToken().type == "imod") {
        addChild(node, makeNode(currentToken().type));
        advance();
        addChild(node, parseFactor());
    }
    return node;
}


NodePtr ExpressionParser::parseSimpleExpression() {
    NodePtr node = makeNode("simple-expression");
    addChild(node, parseTerm());
    while (currentToken().type == "plus" || currentToken().type == "minus") {
        addChild(node, makeNode(currentToken().type));
        advance();
        addChild(node, parseTerm());
    }
    return node;
}

NodePtr ExpressionParser::parseExpression() {
    NodePtr node = makeNode("expression");
    addChild(node, parseSimpleExpression());
    if (currentToken().type == "eql" || currentToken().type == "notsy" || currentToken().type == "lss" || currentToken().type == "leq" || currentToken().type == "gtr" || currentToken().type == "geq") {
        addChild(node, makeNode(currentToken().type));
        advance();
        addChild(node, parseSimpleExpression());
    }
    return node;
}


Token ExpressionParser::currentToken() const {
    if (pos < tokens.size()) {
        return tokens[pos];
    }
    return {"eof", ""};
}

Token ExpressionParser::peek(int offset) const {
    if (pos + offset < tokens.size()) {
        return tokens[pos + offset];
    }
    return {"eof", ""};
}

bool ExpressionParser::isAtEnd() const {
    return pos >= tokens.size();
}

void ExpressionParser::advance() {
    if (!isAtEnd()) {
        pos++;
    }
}

bool ExpressionParser::matchType(const string& type) {
    if (currentToken().type == type) {
        advance();
        return true;
    }
    return false;
}

bool ExpressionParser::matchValue(const string& value) {
    if (currentToken().value == value) {
        advance();
        return true;
    }
    return false;
}

void ExpressionParser::consume(const string& expectedType, const string& errorMessage) {
    if (currentToken().type == expectedType) {
        advance();
    } 
    else {
        cout << "[ERROR] " << errorMessage << " - Expected token type: " << expectedType << " but got " << currentToken().type << "\n";
    }
}
