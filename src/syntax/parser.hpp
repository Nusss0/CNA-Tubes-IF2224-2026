#pragma once

#include "../std.hpp"
#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"
#include "declaration_parser.hpp"
#include "expression_parser.hpp"
#include "stmt_sbpgr_parser.hpp"

struct ParseError {
    string message;
    int tokenIndex;
    string tokenType;
    string tokenValue;
};

class Parser {
public:
    Parser(const vector<Token>& toks);

    //entry point: parse <program>
    NodePtr parseProgram();

    bool hasError() const { return !errors.empty(); }
    const vector<ParseError>& getErrors() const { return errors; }

    //---- core engine ----
    //token consumption + lookahead, dipakai sama semua sub-parser
    const Token& peek(int offset = 0) const;
    bool check(const string& type) const;
    bool accept(const string& type);                     //consume kalau cocok
    bool expect(const string& type, const string& ctx);  //error kalau ga cocok
    const Token& advance();
    bool isAtEnd() const;

    void reportError(const string& message);

    //---- grammar rules ----
    NodePtr parseProgramHeader();
    NodePtr parseDeclarationPart();
    NodePtr parseCompoundStatement();
    NodePtr parseStatementList();

    //---- stub, blm diimplementasi ----
    NodePtr parseDeclaration();
    NodePtr parseStatement();

private:
    const vector<Token>& tokens;
    int pos;
    vector<ParseError> errors;

    DeclarationParser declParser;
    ExpressionParser exprParser;
    StatementSubprogramParser stmtParser;

    void skipTrivia(); //auto-skip token komentar
    void synchronize(const vector<string>& syncTypes); //panic-mode recovery

    static string tokenDisplay(const Token& t);
};
