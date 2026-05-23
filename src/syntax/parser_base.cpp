#include "parser_base.hpp"

Token ParserBase::peek(size_t offset) const {
    // peek
    size_t p = pos + offset;
    if (p >= tokens.size()) return Token{"eof", ""};
    return tokens[p];
}

Token ParserBase::advance() {
    // advance
    if (isAtEnd()) return Token{"eof", ""};
    return tokens[pos++];
}

bool ParserBase::check(const string& type) const {
    // check
    if (isAtEnd()) return type == "eof";
    return tokens[pos].type == type;
}

bool ParserBase::match(const string& type) {
    // match
    if (!check(type)) return false;
    pos++;
    return true;
}

bool ParserBase::consume(const string& type, const string& ctx) {
    // consume
    if (match(type)) return true;
    Token t = peek();
    string got = t.value.empty() ? ("<" + t.type + ">") : ("<" + t.type + ", " + t.value + ">");
    reportError("Expected '" + type + "' in " + ctx + " but got " + got);
    return false;
}

NodePtr ParserBase::makeTokenNode(const Token& t) const {
    // token -> node
    if (t.value.empty()) return makeNode(t.type);
    return makeNode(t.type + "(" + t.value + ")");
}

void ParserBase::reportError(const string& message) {
    // store first error
    if (errorMessage.empty()) errorMessage = message;
}
