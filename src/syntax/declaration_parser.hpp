#pragma once

#include "../std.hpp"
#include "parse_tree.hpp"
#include "../lexical/token_processing.hpp"
#include "parser_base.hpp"

// parser untuk const/type/var declaration & program-header.
// helper token diturunkan dari ParserBase. method2 utama di-public-kan
// supaya bisa dipanggil dari Parser utama / StatementSubprogramParser
// dgn pola pos-sync.
class DeclarationParser : public ParserBase {
public:
    DeclarationParser() = default;
    explicit DeclarationParser(const vector<Token>& tokens) : ParserBase(tokens) {}

    // parse seluruh const/type/var section secara urut
    NodePtr parseDeclarationPart();

    // dipanggil per-section dari Parser utama atau StmtParser
    NodePtr parseConstDeclaration();
    NodePtr parseTypeDeclaration();
    NodePtr parseVarDeclaration();
    NodePtr parseConstant();
    NodePtr parseIdentifierList();
    NodePtr parseArrayType();

private:
    // override base: charcon/string dikutip pake single-quote
    NodePtr makeTokenNode(const Token& token) const;

    // helper error legacy: "Syntax error: unexpected token X(value), expected Y"
    void setError(const string& expected, const Token& found);

    // cek apakah token bisa mulai constant (ident/intcon/realcon/charcon/string/+/-)
    bool isConstantStart(size_t offset = 0) const;
    // cek apakah ada pola constant '..' di posisi (curr+offset)
    bool isRangeAhead(size_t offset = 0) const;

    NodePtr parseProgramHeader();
    NodePtr parseType();
    NodePtr parseRange();       // constant '..' constant
    NodePtr parseEnumerated();
    NodePtr parseRecordType();
    NodePtr parseFieldList();
    NodePtr parseFieldPart();
};
