#pragma once

#include "../std.hpp"
#include "parse_tree.hpp"
#include "../lexical/token_processing.hpp"

class DeclarationParser {
public:
    // New parser dari list token hasil lexer
    explicit DeclarationParser(const vector<Token>& tokens);

    // Parsing deklarasi
    NodePtr parseDeclarationPart();
    
    // Ambil last error msg
    const string& error() const;

private:
    const vector<Token>& tokens;
    size_t pos;
    string errorMessage;

    // Intip token
    const Token* peek(size_t offset = 0) const;

    // Cek apakah habis
    bool isAtEnd() const;

    // Cek tipe token skrg
    bool check(const string& type) const;
    
    // Match token dan gunakan jika sesuai
    bool match(const string& type);

    // Geser posisi parser 1 token
    const Token& advance();

    // Ambil token yang terakhir digunakan
    const Token& previous() const;

    // Ubah token jadi node parse tree
    NodePtr makeTokenNode(const Token& token) const;

    // Simpen error
    void setError(const string& expected, const Token& found);

    // Cek apakah token bisa mulai constant (curr pos)
    bool isConstantStart(size_t offset = 0) const;

    // Cek apakah token membentuk range (curr pos)
    bool isRangeAhead(size_t offset = 0) const;

    // Cek apakah token valid (curr pos)
    bool isTypeStart(size_t offset = 0) const;

    NodePtr parseProgramHeader(); // program header
    NodePtr parseConstDeclaration(); // const dec
    NodePtr parseTypeDeclaration(); // type dec
    NodePtr parseVarDeclaration(); // var dec
    NodePtr parseIdentifierList(); // identifier
    NodePtr parseType(); // type
    NodePtr parseArrayType(); // array type
    NodePtr parseRange(); // range constant..constant
    NodePtr parseEnumerated(); // enumerated
    NodePtr parseRecordType(); // record type
    NodePtr parseFieldList(); // field list
    NodePtr parseFieldPart(); // field part
    NodePtr parseConstant(); // constant untuk deklarasi dan range
};