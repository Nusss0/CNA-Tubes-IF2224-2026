#pragma once

#include "../std.hpp"
#include "parse_tree.hpp"
#include "../lexical/token_processing.hpp"
#include "parser_base.hpp"

// decl parser
class DeclarationParser : public ParserBase {
public:
    DeclarationParser() = default;
    explicit DeclarationParser(const vector<Token>& tokens) : ParserBase(tokens) {}

    // sections
    NodePtr parseDeclarationPart();

    // per-section
    NodePtr parseConstDeclaration();
    NodePtr parseTypeDeclaration();
    NodePtr parseVarDeclaration();
    NodePtr parseConstant();
    NodePtr parseIdentifierList();
    NodePtr parseArrayType();

private:
    // quote char/string
    NodePtr makeTokenNode(const Token& token) const;

    // legacy error fmt
    void setError(const string& expected, const Token& found);

    bool isConstantStart(size_t offset = 0) const;
    bool isRangeAhead(size_t offset = 0) const;

    NodePtr parseProgramHeader();
    NodePtr parseType();
    NodePtr parseRange();       // constant '..' constant
    NodePtr parseEnumerated();
    NodePtr parseRecordType();
    NodePtr parseFieldList();
    NodePtr parseFieldPart();
};
