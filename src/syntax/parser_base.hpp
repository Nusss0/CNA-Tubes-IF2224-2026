#pragma once

#include "../std.hpp"
#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"

// base class semua parser: nyimpen state token + helper umum
// (tokens, pos, error msg, peek/advance/check/match/consume).
// subclass cuma fokus ke grammar production.
class ParserBase {
public:
    ParserBase() = default;
    explicit ParserBase(const vector<Token>& toks) : tokens(toks) {}
    virtual ~ParserBase() = default;

    // setter & getter umum, dipake utk pos-sync antar parser
    void setTokens(const vector<Token>& toks) { tokens = toks; pos = 0; }
    void setPosition(size_t p) { pos = p; }
    size_t getPosition() const { return pos; }

    // error-state API, dibaca Parser utama utk gabungin ke errors[]
    const string& error() const { return errorMessage; }
    bool hasError() const { return !errorMessage.empty(); }
    void clearError() { errorMessage.clear(); }

protected:
    vector<Token> tokens;
    size_t pos = 0;
    string errorMessage;

    // ---- helper umum buat semua parser ----
    Token peek(size_t offset = 0) const;          // intip token tanpa maju (eof kalau OOB)
    Token currentToken() const { return peek(0); }// alias buat token skrg
    bool  isAtEnd() const { return pos >= tokens.size(); }
    Token advance();                              // konsumsi + return token skrg
    bool  check(const string& type) const;        // cek tipe token skrg, ga maju
    bool  match(const string& type);              // konsumsi kalau cocok
    bool  consume(const string& type, const string& ctx); // match atau report error
    NodePtr makeTokenNode(const Token& t) const;  // token -> node ("type" atau "type(value)")

    // virtual: Parser override utk push ke errors[] + cerr
    virtual void reportError(const string& message);
};
