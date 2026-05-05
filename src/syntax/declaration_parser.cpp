#include "declaration_parser.hpp"

DeclarationParser::DeclarationParser(const vector<Token>& tokens) : tokens(tokens), pos(0) {}

const string& DeclarationParser::error() const {
    return errorMessage;
}

const Token* DeclarationParser::peek(size_t offset) const {
    // Lookahead token tanpa memindahkan posisi parser
    size_t index = pos + offset;
    if (index >= tokens.size()){
        return nullptr;
    }

    return &tokens[index];
}

bool DeclarationParser::end() const {
    return pos >= tokens.size();
}

bool DeclarationParser::check(const string& type) const {
    const Token* token = peek();
    return token != nullptr && token->type == type;
}

bool DeclarationParser::match(const string& type) {
    if (!check(type)){
        return false;
    }

    next();
    return true;
}

const Token& DeclarationParser::next() {
    if (end()) {
        return tokens.back();
    }
    
    return tokens[pos++];
}

const Token& DeclarationParser::prev() const {
    return tokens[pos - 1];
}

NodePtr DeclarationParser::makeTokenNode(const Token& token) const {
    if (token.value.empty()) {
        return makeNode(token.type);
    }

    return makeNode(token.type + "(" + token.value + ")");
}

void DeclarationParser::setError(const string& expected, const Token& found) {
    errorMessage = "Syntax error: unexpected token " + found.type;
    if (!found.value.empty()) {
        errorMessage += "(" + found.value + ")";
    }

    errorMessage += ", expected " + expected;
}

bool DeclarationParser::isConstantStart(size_t offset) const {
    const Token* token = peek(offset);
    if (!token){
        return false;
    }

    return token->type == "ident" || token->type == "intcon" || token->type == "realcon" || token->type == "charcon" || token->type == "string" || token->type == "plus" || token->type == "minus";
}

bool DeclarationParser::isRangeAhead(size_t offset) const {
    const Token* first = peek(offset);
    const Token* second = peek(offset + 1);
    const Token* third = peek(offset + 2);
    return first && second && third && isConstantStart(offset) && second->type == "period" && third->type == "period";
}

bool DeclarationParser::isTypeStart(size_t offset) const {
    const Token* token = peek(offset);
    if (!token){
        return false;
    }

    if (token->type == "arraysy" || token->type == "recordsy" || token->type == "lparent" || token->type == "ident") {
        return true;
    }

    return isRangeAhead(offset);
}