#include "parser_base.hpp"

Token ParserBase::peek(size_t offset) const {
    // lookahead tanpa memindahkan posisi parser
    size_t p = pos + offset;
    if (p >= tokens.size()) return Token{"eof", ""};
    return tokens[p];
}

Token ParserBase::advance() {
    // konsumsi token skrg dan kembalikan; eof kalau habis
    if (isAtEnd()) return Token{"eof", ""};
    return tokens[pos++];
}

bool ParserBase::check(const string& type) const {
    // cek tipe token skrg tanpa mengkonsumsi
    if (isAtEnd()) return type == "eof";
    return tokens[pos].type == type;
}

bool ParserBase::match(const string& type) {
    // konsumsi kalau cocok, biarin kalau ga
    if (!check(type)) return false;
    pos++;
    return true;
}

bool ParserBase::consume(const string& type, const string& ctx) {
    // sama kayak match tapi catat error kalau ga cocok.
    // format: "Expected '<type>' in <ctx> but got <token>"
    if (match(type)) return true;
    Token t = peek();
    string got = t.value.empty() ? ("<" + t.type + ">") : ("<" + t.type + ", " + t.value + ">");
    reportError("Expected '" + type + "' in " + ctx + " but got " + got);
    return false;
}

NodePtr ParserBase::makeTokenNode(const Token& t) const {
    // ubah token jadi node parse tree
    if (t.value.empty()) return makeNode(t.type);
    return makeNode(t.type + "(" + t.value + ")");
}

void ParserBase::reportError(const string& message) {
    // default: simpen error pertama biar Parser bisa baca via error()
    if (errorMessage.empty()) errorMessage = message;
}
