#include "stmt_sbpgr_parser.hpp"
#include <iostream>

StatementSubprogramParser::StatementSubprogramParser(const vector<Token>& tokens) {
    this->tokens = tokens;
    this->pos = 0;
}

void StatementSubprogramParser::setTokens(const vector<Token>& tokens) {
    this->tokens = tokens;
    this->pos = 0;
}

void StatementSubprogramParser::setPosition(size_t pos) {
    this->pos = pos;
}

size_t StatementSubprogramParser::getPosition() const {
    return pos;
}

void StatementSubprogramParser::setDeclParser(DeclarationParser* p) {
    declParser_ = p;
}

void StatementSubprogramParser::setExprParser(ExpressionParser* p) {
    exprParser_ = p;
}

NodePtr StatementSubprogramParser::parseStatement() {
    Token t = currentToken();
    if (t.type == "beginsy") {
        return parseCompoundStatement();
    }
    if (t.type == "ifsy") {
        return parseIfStatement();
    }
    if (t.type == "casesy") {
        return parseCaseStatement();
    }
    if (t.type == "whilesy") {
        return parseWhileStatement();
    }
    if (t.type == "repeatsy") {
        return parseRepeatStatement();
    }
    if (t.type == "forsy") {
        return parseForStatement();
    }
    if (t.type == "ident") {
        Token next = peek();
        if (next.type == "lparent") {
            return parseProcedureFunctionCall();
        }
        if (next.type == "becomes" || next.type == "lbrack" || next.type == "period") {
            return parseAssignmentStatement();
        }
        return parseProcedureFunctionCall();
    }
    return nullptr;
}

NodePtr StatementSubprogramParser::parseAssignmentStatement() {
    NodePtr node = makeNode("<assignment-statement>");
    addChild(node, parseVariable());
    if (matchType("becomes")) {
        addChild(node, makeNode("becomes"));
    } 
    else {
        consume("becomes", "Expected ':='");
    }
    addChild(node, parseExpression());
    return node;
}

NodePtr StatementSubprogramParser::parseIfStatement() {
    NodePtr node = makeNode("<if-statement>");
    consume("ifsy", "Expected 'if'");
    addChild(node, makeNode("ifsy"));
    addChild(node, parseExpression());
    consume("thensy", "Expected 'then'");
    addChild(node, makeNode("thensy"));
    addChild(node, parseStatement());
    if (currentToken().type == "elsesy") {
        consume("elsesy", "");
        addChild(node, makeNode("elsesy"));
        addChild(node, parseStatement());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseCaseStatement() {
    NodePtr node = makeNode("<case-statement>");
    consume("casesy", "Expected 'case'");
    addChild(node, makeNode("casesy"));
    addChild(node, parseExpression());
    consume("ofsy", "Expected 'of'");
    addChild(node, makeNode("ofsy"));
    addChild(node, parseCaseBlock());
    consume("endsy", "Expected 'end'");
    addChild(node, makeNode("endsy"));
    return node;
}

NodePtr StatementSubprogramParser::parseCaseBlock() {
    NodePtr node = makeNode("<case-block>");
    addChild(node, parseConstant());
    while (currentToken().type == "comma") {
        consume("comma", "");
        addChild(node, makeNode("comma"));
        addChild(node, parseConstant());
    }
    consume("colon", "Expected ':'");
    addChild(node, makeNode("colon"));
    addChild(node, parseStatement());
    while (currentToken().type == "semicolon") {
        consume("semicolon", "");
        addChild(node, makeNode("semicolon"));
        Token t = currentToken();
        if (t.type == "intcon" || t.type == "realcon" || t.type == "charcon" || t.type == "string" || t.type == "plus" || t.type == "minus" || t.type == "ident") {
            addChild(node, parseCaseBlock());
        }
    }
    return node;
}

NodePtr StatementSubprogramParser::parseWhileStatement() {
    NodePtr node = makeNode("<while-statement>");
    consume("whilesy", "Expected 'while'");
    addChild(node, makeNode("whilesy"));
    addChild(node, parseExpression());
    consume("dosy", "Expected 'do'");
    addChild(node, makeNode("dosy"));
    addChild(node, parseStatement());
    return node;
}

NodePtr StatementSubprogramParser::parseRepeatStatement() {
    NodePtr node = makeNode("<repeat-statement>");
    consume("repeatsy", "Expected 'repeat'");
    addChild(node, makeNode("repeatsy"));
    addChild(node, parseStatementList());
    consume("untilsy", "Expected 'until'");
    addChild(node, makeNode("untilsy"));
    addChild(node, parseExpression());
    return node;
}

NodePtr StatementSubprogramParser::parseForStatement() {
    NodePtr node = makeNode("<for-statement>");
    consume("forsy", "Expected 'for'");
    addChild(node, makeNode("forsy"));
    
    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } 
    else consume("ident", "Expected identifier");

    consume("becomes", "Expected ':='");
    addChild(node, makeNode("becomes"));
    addChild(node, parseExpression());

    if (currentToken().type == "tosy" || currentToken().type == "downtosy") {
        addChild(node, makeNode(currentToken().type));
        advance();
    } 
    else {
        consume("tosy", "Expected 'to' or 'downto'");
    }

    addChild(node, parseExpression());
    consume("dosy", "Expected 'do'");
    addChild(node, makeNode("dosy"));
    addChild(node, parseStatement());
    return node;
}

NodePtr StatementSubprogramParser::parseProcedureFunctionCall() {
    NodePtr node = makeNode("<procedure/function-call>");
    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } 
    else consume("ident", "Expected identifier");

    if (currentToken().type == "lparent") {
        advance();
        addChild(node, makeNode("lparent"));
        if (currentToken().type != "rparent") {
            addChild(node, parseParameterList());
        }
        consume("rparent", "Expected ')'");
        addChild(node, makeNode("rparent"));
    }
    return node;
}

NodePtr StatementSubprogramParser::parseParameterList() {
    NodePtr node = makeNode("<parameter-list>");
    addChild(node, parseExpression());
    while (currentToken().type == "comma") {
        consume("comma", "");
        addChild(node, makeNode("comma"));
        addChild(node, parseExpression());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseVariable() {
    NodePtr node = makeNode("<variable>");
    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } 
    else consume("ident", "Expected identifier");

    while (currentToken().type == "lbrack" || currentToken().type == "period") {
        addChild(node, parseComponentVariable());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseComponentVariable() {
    NodePtr node = makeNode("<component-variable>");
    if (currentToken().type == "lbrack") {
        consume("lbrack", "");
        addChild(node, makeNode("lbrack"));
        addChild(node, parseIndexList());
        consume("rbrack", "Expected ']'");
        addChild(node, makeNode("rbrack"));
    } 
    else if (currentToken().type == "period") {
        consume("period", "");
        addChild(node, makeNode("period"));
        if (currentToken().type == "ident") {
            addChild(node, makeNode("ident(" + currentToken().value + ")"));
            advance();
        } 
        else consume("ident", "Expected identifier");
    } 
    else {
        consume("lbrack", "Expected '[' or '.'");
    }
    return node;
}

NodePtr StatementSubprogramParser::parseIndexList() {
    NodePtr node = makeNode("<index-list>");
    Token t = currentToken();
    if (t.type == "intcon" || t.type == "charcon" || t.type == "ident") {
        if (t.value != "") addChild(node, makeNode(t.type + "(" + t.value + ")"));
        else addChild(node, makeNode(t.type));
        advance();
    } 
    else {
        consume("intcon", "Expected intcon, charcon, or ident");
    }

    if (currentToken().type == "comma") {
        consume("comma", "");
        addChild(node, makeNode("comma"));
        addChild(node, parseIndexList());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseSubprogramDeclaration() {
    NodePtr node = makeNode("<subprogram-declaration>");
    if (currentToken().type == "proceduresy") {
        addChild(node, parseProcedureDeclaration());
    } 
    else if (currentToken().type == "functionsy") {
        addChild(node, parseFunctionDeclaration());
    } 
    else {
        consume("proceduresy", "Expected 'procedure' or 'function'");
    }
    return node;
}

NodePtr StatementSubprogramParser::parseProcedureDeclaration() {
    NodePtr node = makeNode("<procedure-declaration>");
    consume("proceduresy", "Expected 'procedure'");
    addChild(node, makeNode("proceduresy"));
    
    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } 
    else consume("ident", "Expected identifier");

    if (currentToken().type == "lparent") {
        addChild(node, parseFormalParameterList());
    }

    consume("semicolon", "Expected ';'");
    addChild(node, makeNode("semicolon"));
    
    addChild(node, parseBlock());
    
    consume("semicolon", "Expected ';'");
    addChild(node, makeNode("semicolon"));
    return node;
}

NodePtr StatementSubprogramParser::parseFunctionDeclaration() {
    NodePtr node = makeNode("<function-declaration>");
    consume("functionsy", "Expected 'function'");
    addChild(node, makeNode("functionsy"));
    
    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    }    
    else consume("ident", "Expected identifier");

    if (currentToken().type == "lparent") {
        addChild(node, parseFormalParameterList());
    }

    consume("colon", "Expected ':'");
    addChild(node, makeNode("colon"));

    if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } 
    else consume("ident", "Expected identifier");

    consume("semicolon", "Expected ';'");
    addChild(node, makeNode("semicolon"));
    
    addChild(node, parseBlock());
    
    consume("semicolon", "Expected ';'");
    addChild(node, makeNode("semicolon"));
    return node;
}

NodePtr StatementSubprogramParser::parseBlock() {
    NodePtr node = makeNode("<block>");
    addChild(node, parseDeclarationPart());
    while (currentToken().type == "proceduresy" || currentToken().type == "functionsy") {
        addChild(node, parseSubprogramDeclaration());
    }
    addChild(node, parseCompoundStatement());
    return node;
}

NodePtr StatementSubprogramParser::parseFormalParameterList() {
    NodePtr node = makeNode("<formal-parameter-list>");
    consume("lparent", "Expected '('");
    addChild(node, makeNode("lparent"));
    
    addChild(node, parseParameterGroup());
    
    while (currentToken().type == "semicolon") {
        consume("semicolon", "");
        addChild(node, makeNode("semicolon"));
        addChild(node, parseParameterGroup());
    }
    
    consume("rparent", "Expected ')'");
    addChild(node, makeNode("rparent"));
    return node;
}

NodePtr StatementSubprogramParser::parseParameterGroup() {
    NodePtr node = makeNode("<parameter-group>");
    addChild(node, parseIdentifierList());
    
    consume("colon", "Expected ':'");
    addChild(node, makeNode("colon"));

    if (currentToken().type == "arraysy") {
        addChild(node, parseArrayType());
    } 
    else if (currentToken().type == "ident") {
        addChild(node, makeNode("ident(" + currentToken().value + ")"));
        advance();
    } 
    else {
        consume("ident", "Expected 'ident' or 'array'");
    }
    
    return node;
}

NodePtr StatementSubprogramParser::parseExpression() {
    // placeholder untuk parser expression
    return makeNode("expression");
}

NodePtr StatementSubprogramParser::parseStatementList() {
    // placeholder untuk parser statement list
    return makeNode("statement-list");
}

NodePtr StatementSubprogramParser::parseConstant() {
    // placeholder untuk parser constant
    return makeNode("constant");
}

NodePtr StatementSubprogramParser::parseDeclarationPart() {
    // placeholder untuk parser declaration part
    return makeNode("declaration-part");
}

NodePtr StatementSubprogramParser::parseCompoundStatement() {
    // placeholder untuk parser compound statement
    return makeNode("compound-statement");
}

NodePtr StatementSubprogramParser::parseIdentifierList() {
    // placeholder untuk parser identifier list
    return makeNode("identifier-list");
}

NodePtr StatementSubprogramParser::parseArrayType() {
    // placeholder untuk parser array type
    return makeNode("array-type");
}

Token StatementSubprogramParser::currentToken() const {
    if (pos < tokens.size()) {
        return tokens[pos];
    }
    return {"eof", ""};
}

Token StatementSubprogramParser::peek(int offset) const {
    if (pos + offset < tokens.size()) {
        return tokens[pos + offset];
    }
    return {"eof", ""};
}

bool StatementSubprogramParser::isAtEnd() const {
    return pos >= tokens.size();
}

void StatementSubprogramParser::advance() {
    if (!isAtEnd()) {
        pos++;
    }
}

bool StatementSubprogramParser::matchType(const string& type) {
    if (currentToken().type == type) {
        advance();
        return true;
    }
    return false;
}

bool StatementSubprogramParser::matchValue(const string& value) {
    if (currentToken().value == value) {
        advance();
        return true;
    }
    return false;
}

void StatementSubprogramParser::consume(const string& expectedType, const string& errorMessage) {
    if (currentToken().type == expectedType) {
        advance();
    } 
    else {
        cout << "[ERROR] " << errorMessage << " - Expected token type: " << expectedType << " but got " << currentToken().type << "\n";
    }
}
