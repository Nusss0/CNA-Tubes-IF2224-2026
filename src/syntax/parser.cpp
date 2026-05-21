#include "parser.hpp"

Parser::Parser(const vector<Token>& toks)
    : ParserBase(toks),
      declParser(toks), exprParser(toks), stmtParser(toks) {
    // cross-wiring: stmtParser delegasi ke declParser/exprParser
    stmtParser.setDeclParser(&declParser);
    stmtParser.setExprParser(&exprParser);
    skipTrivia();
}

//---- helpers ----

void Parser::skipTrivia() {
    // skip token komentar, parser ga peduli
    while (pos < tokens.size() && tokens[pos].type == "comment") pos++;
}

void Parser::reportError(const string& message) {
    // override base: simpen ke errors[] + cetak ke cerr,
    // format identik dgn versi lama (Expected '...' (at token #N: <type, value>))
    Token t = peek();
    errors.push_back({message, (int)pos, t.type, t.value});
    cerr << "[SYNTAX ERROR] " << message << " (at token #" << pos << ": " << tokenDisplay(t) << ")\n";
    errorMessage = message;
}

string Parser::tokenDisplay(const Token& t) {
    if (t.value.empty()) return "<" + t.type + ">";
    return "<" + t.type + ", " + t.value + ">";
}

void Parser::synchronize(const vector<string>& syncTypes) {
    // skip token sampai ketemu salah satu sync point
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

    // program harus diakhiri '.'
    if (consume("period", "<program>")) {
        addChild(root, makeNode("period"));
    }

    if (!isAtEnd()) reportError("Unexpected tokens after end of program");

    return root;
}

NodePtr Parser::parseProgramHeader() {
    NodePtr node = makeNode("<program-header>");

    if (!consume("programsy", "<program-header>")) {
        // ga ada keyword PROGRAM, kembaliin node kosong dan biar caller lanjut
        return node;
    }
    addChild(node, makeNode("programsy"));

    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else {
        reportError("Expected program name (ident) after PROGRAM");
    }

    if (consume("semicolon", "<program-header>")) addChild(node, makeNode("semicolon"));

    return node;
}

NodePtr Parser::parseDeclarationPart() {
    NodePtr node = makeNode("<declaration-part>");

    // const/type/var pertama, urut sesuai grammar (delegasi ke DeclarationParser)
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

    // subprogram (procedure/function) hanya boleh setelah const/type/var
    while (check("proceduresy") || check("functionsy")) {
        size_t before = pos;
        NodePtr d = parseDeclaration();
        if (d) addChild(node, d);
        if (pos == before) break; // safety: cegah infinite loop
    }

    return node;
}

NodePtr Parser::parseCompoundStatement() {
    NodePtr node = makeNode("<compound-statement>");

    if (!consume("beginsy", "<compound-statement>")) {
        synchronize({"endsy", "period"});
        return node;
    }
    addChild(node, makeNode("beginsy"));

    NodePtr list = parseStatementList();
    if (list) addChild(node, list);

    if (consume("endsy", "<compound-statement>")) addChild(node, makeNode("endsy"));
    return node;
}

NodePtr Parser::parseStatementList() {
    NodePtr node = makeNode("<statement-list>");

    NodePtr prev = parseStatement();
    addChild(node, prev);

    while (true) {
        // while/for udah consume ';' sendiri (revisi M3) → loop lanjut tanpa advance
        bool ownsSemicolon = prev && (prev->label == "<while-statement>" || prev->label == "<for-statement>")
                             && !prev->children.empty() && prev->children.back()->label == "semicolon";
        if (!ownsSemicolon) {
            if (!check("semicolon")) break;
            advance();
            NodePtr semi = makeNode("semicolon");
            if (check("endsy")) {
                if (prev && prev->label == "<procedure/function-call>") addChild(prev, semi);
                else addChild(node, semi);
                break;
            }
            if (prev && prev->label == "<procedure/function-call>") addChild(prev, semi);
            else addChild(node, semi);
        } else {
            // statement udah self-terminate, stop kalau abis itu ketemu END
            if (check("endsy")) break;
        }
        NodePtr s2 = parseStatement();
        addChild(node, s2);
        prev = s2;
    }

    return node;
}

//---- delegated to sub-parsers ----

NodePtr Parser::parseDeclaration() {
    // skrg cuma dipanggil dari subprogram-loop, jadi cuma proc/func
    if (!check("proceduresy") && !check("functionsy")) {
        reportError("Expected 'procedure' or 'function' in <declaration>");
        advance();
        return nullptr;
    }

    stmtParser.clearError();
    stmtParser.setPosition(pos);
    NodePtr inner = stmtParser.parseSubprogramDeclaration();
    pos = stmtParser.getPosition();
    if (stmtParser.hasError()) reportError(stmtParser.error());
    return inner;
}

NodePtr Parser::parseStatement() {
    // revisi M3: empty statement valid, muncul sbg <empty-statement> di tree
    if (check("endsy") || check("semicolon") || check("elsesy") || check("untilsy") || isAtEnd()) {
        return makeNode("<empty-statement>");
    }

    if (check("beginsy")) {
        // compound statement bersarang
        return parseCompoundStatement();
    }

    size_t before = pos;
    stmtParser.clearError();
    stmtParser.setPosition(pos);
    NodePtr inner = stmtParser.parseStatement();
    pos = stmtParser.getPosition();
    if (stmtParser.hasError()) reportError(stmtParser.error());
    // jaga2: kalau ga ada progress dan ga ada error, paksa error + advance biar ga deadlock
    if (pos == before && !inner && !stmtParser.hasError()) {
        reportError("Could not parse statement at " + tokenDisplay(peek()));
        advance();
    }
    return inner;
}
