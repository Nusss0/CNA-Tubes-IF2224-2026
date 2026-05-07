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
    NodePtr node = makeNode("<factor>");
    Token t = currentToken();
    if (t.type == "intcon" || t.type == "realcon" || t.type == "charcon" || t.type == "string") {
        string label = t.type;
        if (t.type == "charcon" || t.type == "string") label += "('" + t.value + "')";
        else label += "(" + t.value + ")";
        addChild(node, makeNode(label));
        advance();
    } else if (t.type == "ident") {
        Token n = peek();
        if (n.type == "lparent") {
            addChild(node, parseProcedureFunctionCall());
        } else if (n.type == "lbrack" || n.type == "period") {
            addChild(node, parseVariable());
        } else {
            addChild(node, makeNode("ident(" + t.value + ")"));
            advance();
        }
    } else if (t.type == "lparent") {
        consume("lparent", "Expected '('");
        addChild(node, makeNode("lparent"));
        addChild(node, parseExpression());
        consume("rparent", "Expected ')'");
        addChild(node, makeNode("rparent"));
    } else if (t.type == "notsy") {
        consume("notsy", "Expected 'not'");
        addChild(node, makeNode("notsy"));
        addChild(node, parseFactor());
    } else {
        if (errorMessage.empty()) errorMessage = "unexpected token in <factor>: " + t.type;
    }
    return node;
}

NodePtr ExpressionParser::parseVariable() {
    NodePtr node = makeNode("<variable>");
    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } else {
        consume("ident", "Expected identifier");
    }
    while (currentToken().type == "lbrack" || currentToken().type == "period") {
        NodePtr comp = makeNode("<component-variable>");
        if (currentToken().type == "lbrack") {
            advance();
            addChild(comp, makeNode("lbrack"));
            addChild(comp, parseIndexList());
            consume("rbrack", "Expected ']'");
            addChild(comp, makeNode("rbrack"));
        } else if (currentToken().type == "period") {
            advance();
            addChild(comp, makeNode("period"));
            if (currentToken().type == "ident") {
                addChild(comp, makeNode("ident(" + currentToken().value + ")"));
                advance();
            } else {
                consume("ident", "Expected identifier");
            }
        }
        addChild(node, comp);
    }
    return node;
}

NodePtr ExpressionParser::parseProcedureFunctionCall() {
    NodePtr node = makeNode("<procedure/function-call>");
    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } else {
        consume("ident", "Expected identifier");
    }
    if (currentToken().type == "lparent") {
        advance();
        addChild(node, makeNode("lparent"));
        if (currentToken().type != "rparent") {
            addChild(node, parseExpression());
            while (currentToken().type == "comma") {
                addChild(node, makeNode("comma"));
                advance();
                addChild(node, parseExpression());
            }
        }
        consume("rparent", "Expected ')'");
        addChild(node, makeNode("rparent"));
    }
    return node;
}

NodePtr ExpressionParser::parseIndexList() {
    NodePtr node = makeNode("<index-list>");
    Token t = currentToken();
    if (t.type == "intcon" || t.type == "charcon" || t.type == "ident") {
        string label = t.type;
        if (t.type == "charcon") label += "('" + t.value + "')";
        else label += "(" + t.value + ")";
        addChild(node, makeNode(label));
        advance();
    } else {
        consume("intcon", "Expected intcon, charcon, or ident");
    }
    if (currentToken().type == "comma") {
        advance();
        addChild(node, makeNode("comma"));
        addChild(node, parseIndexList());
    }
    return node;
}

NodePtr ExpressionParser::parseTerm() {
    NodePtr node = makeNode("<term>");
    addChild(node, parseFactor());
    while (currentToken().type == "times" || currentToken().type == "idiv" || currentToken().type == "rdiv" || currentToken().type == "imod" || currentToken().type == "andsy") {
        addChild(node, makeNode(currentToken().type));
        advance();
        addChild(node, parseFactor());
    }
    return node;
}

NodePtr ExpressionParser::parseSimpleExpression() {
    NodePtr node = makeNode("<simple-expression>");
    if (currentToken().type == "plus" || currentToken().type == "minus") {
        addChild(node, makeNode(currentToken().type));
        advance();
    }
    addChild(node, parseTerm());
    while (currentToken().type == "plus" || currentToken().type == "minus" || currentToken().type == "orsy") {
        addChild(node, makeNode(currentToken().type));
        advance();
        addChild(node, parseTerm());
    }
    return node;
}

NodePtr ExpressionParser::parseExpression() {
    NodePtr node = makeNode("<expression>");
    addChild(node, parseSimpleExpression());
    if (currentToken().type == "eql" || currentToken().type == "neq" || currentToken().type == "lss" || currentToken().type == "leq" || currentToken().type == "gtr" || currentToken().type == "geq") {
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

void ExpressionParser::consume(const string& expectedType, const string& errMsg) {
    if (currentToken().type == expectedType) {
        advance();
        return;
    }
    if (errorMessage.empty()) {
        errorMessage = errMsg.empty() ? ("expected token type: " + expectedType + " but got " + currentToken().type) : (errMsg + " (got " + currentToken().type + ")");
    }
}

const string& ExpressionParser::error() const { return errorMessage; }
bool ExpressionParser::hasError() const { return !errorMessage.empty(); }
void ExpressionParser::clearError() { errorMessage.clear(); }
