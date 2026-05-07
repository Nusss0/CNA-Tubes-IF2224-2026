#pragma once

#include "../std.hpp"
#include "../lexical/token_processing.hpp"
#include "parse_tree.hpp"
#include "parser_base.hpp"
#include "declaration_parser.hpp"
#include "expression_parser.hpp"
#include "stmt_sbpgr_parser.hpp"

// rich error log, dipake Parser utama (sub-parser cuma punya errorMessage string)
struct ParseError {
    string message;
    int tokenIndex;
    string tokenType;
    string tokenValue;
};

// Parser utama: koordinator <program>. Pegang DeclParser/ExprParser/StmtParser
// sbg member, sync posisi via pos-sync pattern. Helper token diturunkan
// dari ParserBase, override reportError utk push ke errors[] + cerr.
class Parser : public ParserBase {
public:
    explicit Parser(const vector<Token>& toks);

    // entry point: parse <program>
    NodePtr parseProgram();

    bool hasError() const { return !errors.empty(); }
    const vector<ParseError>& getErrors() const { return errors; }

    //---- grammar rules ----
    NodePtr parseProgramHeader();
    NodePtr parseDeclarationPart();
    NodePtr parseCompoundStatement();
    NodePtr parseStatementList();
    NodePtr parseDeclaration();
    NodePtr parseStatement();

protected:
    // override base: push ke errors[] + cetak ke cerr
    void reportError(const string& message) override;

private:
    vector<ParseError> errors;

    // sub-parsers, di-construct sekali di ctor + cross-wiring pointer
    DeclarationParser declParser;
    ExpressionParser exprParser;
    StatementSubprogramParser stmtParser;

    void skipTrivia();                              // auto-skip token komentar
    void synchronize(const vector<string>& syncTypes); // panic-mode recovery

    static string tokenDisplay(const Token& t);
};
