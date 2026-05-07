#include "parser.hpp"
#include "declaration_parser.hpp"
#include "stmt_sbpgr_parser.hpp"

Parser::Parser(const vector<Token>& toks) : tokens(toks), pos(0) {
    skipTrivia();
}

//---- helpers ----

void Parser::skipTrivia() {
    //skip token komentar, parser ga peduli
    while (pos < (int)tokens.size() && tokens[pos].type == "comment") pos++;
}

const Token& Parser::peek(int offset) const {
    static Token eof{"eof", ""};
    int p = pos + offset;
    if (p < 0 || p >= (int)tokens.size()) return eof;
    return tokens[p];
}

bool Parser::isAtEnd() const {
    return pos >= (int)tokens.size();
}

bool Parser::check(const string& type) const {
    if (isAtEnd()) return type == "eof";
    return tokens[pos].type == type;
}

const Token& Parser::advance() {
    static Token eof{"eof", ""};
    if (isAtEnd()) return eof;
    const Token& t = tokens[pos++];
    skipTrivia(); //auto-skip komentar setelah maju
    return t;
}

bool Parser::accept(const string& type) {
    //consume kalau cocok, kalau ga biarin
    if (check(type)) { advance(); return true; }
    return false;
}

bool Parser::expect(const string& type, const string& ctx) {
    if (check(type)) { advance(); return true; }
    reportError("Expected '" + type + "' in " + ctx + " but got " + tokenDisplay(peek()));
    return false;
}

void Parser::reportError(const string& message) {
    const Token& t = peek();
    errors.push_back({message, pos, t.type, t.value});
    cerr << "[SYNTAX ERROR] " << message << " (at token #" << pos << ": " << tokenDisplay(t) << ")\n";
}

string Parser::tokenDisplay(const Token& t) {
    if (t.value.empty()) return "<" + t.type + ">";
    return "<" + t.type + ", " + t.value + ">";
}

void Parser::synchronize(const vector<string>& syncTypes) {
    //skip token sampai ketemu salah satu sync point
    while (!isAtEnd()) {
        for (const auto& s : syncTypes) {
            if (tokens[pos].type == s) return;
        }
        pos++;
    }
}

//---- grammar rules ----

NodePtr Parser::parseProgram() {
    NodePtr root = makeNode("<program>");

    NodePtr header = parseProgramHeader();
    if (header) addChild(root, header);

    NodePtr decls = parseDeclarationPart();
    if (decls) addChild(root, decls);

    NodePtr compound = parseCompoundStatement();
    if (compound) addChild(root, compound);

    //program harus diakhiri '.'
    if (expect("period", "<program> (program must end with '.')")) {
        addChild(root, makeNode("period"));
    }

    if (!isAtEnd()) reportError("Unexpected tokens after end of program");

    return root;
}

NodePtr Parser::parseProgramHeader() {
    NodePtr node = makeNode("<program-header>");

    if (!expect("programsy", "<program-header>")) {
        //lompat ke section keyword / BEGIN biar parser ga macet
        synchronize({"semicolon", "varsy", "constsy", "typesy", "proceduresy", "functionsy", "beginsy"});
        return node;
    }
    addChild(node, makeNode("programsy"));

    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else {
        reportError("Expected program name (ident) after PROGRAM");
    }

    if (expect("semicolon", "<program-header>")) addChild(node, makeNode("semicolon"));

    return node;
}

NodePtr Parser::parseDeclarationPart() {
    NodePtr node = makeNode("<declaration-part>");

    // gunakan DeclarationParser buat const, type, var
    DeclarationParser declParser(tokens);
    declParser.setPosition(pos);
    NodePtr decls = declParser.parseDeclarationPart();
    pos = declParser.getPosition();
    if (decls && !decls->children.empty()) {
        for (auto& child : decls->children) {
            addChild(node, child);
        }
    }
    if (!declParser.error().empty()) {
        reportError(declParser.error());
    }

    // subprogram declarations (procedure/function)
    while (check("proceduresy") || check("functionsy")) {
        NodePtr d = parseDeclaration();
        if (d) addChild(node, d);
        else break;
    }

    if (node->children.empty()) addChild(node, makeNode("(empty)"));
    return node;
}

NodePtr Parser::parseCompoundStatement() {
    NodePtr node = makeNode("<compound-statement>");

    if (!expect("beginsy", "<compound-statement>")) {
        synchronize({"endsy", "period"});
        return node;
    }
    addChild(node, makeNode("beginsy"));

    NodePtr list = parseStatementList();
    if (list) addChild(node, list);

    if (expect("endsy", "<compound-statement>")) addChild(node, makeNode("endsy"));
    return node;
}

NodePtr Parser::parseStatementList() {
    NodePtr node = makeNode("<statement-list>");

    NodePtr s = parseStatement();
    if (s) addChild(node, s);

    while (check("semicolon")) {
        advance();
        addChild(node, makeNode("semicolon"));
        if (check("endsy")) break; //trailing ';' sebelum END, stop
        NodePtr s2 = parseStatement();
        if (s2) addChild(node, s2);
    }

    if (node->children.empty()) addChild(node, makeNode("(empty)"));
    return node;
}

//---- stub, blm diimplementasi ----

NodePtr Parser::parseDeclaration() {
    StatementSubprogramParser subParser(tokens);
    subParser.setPosition(pos);
    NodePtr node = subParser.parseSubprogramDeclaration();
    pos = subParser.getPosition();
    return node;
}

NodePtr Parser::parseStatement() {
    StatementSubprogramParser stmtParser(tokens);
    stmtParser.setPosition(pos);
    NodePtr node = stmtParser.parseStatement();
    pos = stmtParser.getPosition();
    return node;
}
