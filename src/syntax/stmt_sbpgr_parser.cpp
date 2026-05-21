#include "stmt_sbpgr_parser.hpp"
#include "declaration_parser.hpp"
#include "expression_parser.hpp"

NodePtr StatementSubprogramParser::parseStatement() {
    Token t = peek();
    if (t.type == "beginsy") return parseCompoundStatement();
    if (t.type == "ifsy")    return parseIfStatement();
    if (t.type == "casesy")  return parseCaseStatement();
    if (t.type == "whilesy") return parseWhileStatement();
    if (t.type == "repeatsy")return parseRepeatStatement();
    if (t.type == "forsy")   return parseForStatement();
    if (t.type == "ident") {
        // ident bisa assignment (x := ...)
        Token nxt = peek(1);
        if (nxt.type == "lparent") return parseProcedureFunctionCall();
        if (nxt.type == "becomes" || nxt.type == "lbrack" || nxt.type == "period")
            return parseAssignmentStatement();
        return parseProcedureFunctionCall();
    }
    // empty statement (sblm ; / END / UNTIL / ELSE / EOF), valid sesuai revisi M3
    if (t.type == "semicolon" || t.type == "endsy" || t.type == "untilsy" || t.type == "elsesy" || isAtEnd()) {
        return makeNode("<empty-statement>");
    }
    // token bener2 ga dikenali, catat error biar Parser utama tau
    if (errorMessage.empty()) errorMessage = "unexpected token at start of statement: " + t.type;
    return nullptr;
}

NodePtr StatementSubprogramParser::parseAssignmentStatement() {
    NodePtr node = makeNode("<assignment-statement>");
    // kalau ident sederhana (ga ada [ atau .), langsung ident tanpa <variable>
    if (check("ident")) {
        Token nxt = peek(1);
        if (nxt.type != "lbrack" && nxt.type != "period") {
            addChild(node, makeNode("ident(" + peek().value + ")"));
            advance();
        } else {
            addChild(node, parseVariable());
        }
    } else {
        addChild(node, parseVariable());
    }
    if (match("becomes")) {
        addChild(node, makeNode("becomes"));
    } else {
        consume("becomes", "<assignment-statement>");
    }
    addChild(node, parseExpression());
    return node;
}

NodePtr StatementSubprogramParser::parseIfStatement() {
    NodePtr node = makeNode("<if-statement>");
    consume("ifsy", "<if-statement>");
    addChild(node, makeNode("ifsy"));
    addChild(node, parseExpression());
    consume("thensy", "<if-statement>");
    addChild(node, makeNode("thensy"));
    addChild(node, parseStatement());
    if (check("elsesy")) {
        advance();
        addChild(node, makeNode("elsesy"));
        addChild(node, parseStatement());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseCaseStatement() {
    NodePtr node = makeNode("<case-statement>");
    consume("casesy", "<case-statement>");
    addChild(node, makeNode("casesy"));
    addChild(node, parseExpression());
    consume("ofsy", "<case-statement>");
    addChild(node, makeNode("ofsy"));
    addChild(node, parseCaseBlock());
    consume("endsy", "<case-statement>");
    addChild(node, makeNode("endsy"));
    return node;
}

NodePtr StatementSubprogramParser::parseCaseBlock() {
    NodePtr node = makeNode("<case-block>");
    addChild(node, parseConstant());
    while (check("comma")) {
        advance();
        addChild(node, makeNode("comma"));
        addChild(node, parseConstant());
    }
    consume("colon", "<case-block>");
    addChild(node, makeNode("colon"));
    addChild(node, parseStatement());
    while (check("semicolon")) {
        advance();
        addChild(node, makeNode("semicolon"));
        Token t = peek();
        if (t.type == "intcon" || t.type == "realcon" || t.type == "charcon" || t.type == "string" || t.type == "plus" || t.type == "minus" || t.type == "ident") {
            addChild(node, parseCaseBlock());
        }
    }
    return node;
}

NodePtr StatementSubprogramParser::parseWhileStatement() {
    NodePtr node = makeNode("<while-statement>");
    consume("whilesy", "<while-statement>");
    addChild(node, makeNode("whilesy"));
    addChild(node, parseExpression());
    consume("dosy", "<while-statement>");
    addChild(node, makeNode("dosy"));
    // revisi M3: body wajib compound-statement, bkn single statement
    addChild(node, parseCompoundStatement());
    // semicolon trailing bagian dari produksi <while-statement>
    if (match("semicolon")) addChild(node, makeNode("semicolon"));
    else consume("semicolon", "<while-statement>");
    return node;
}

NodePtr StatementSubprogramParser::parseRepeatStatement() {
    NodePtr node = makeNode("<repeat-statement>");
    consume("repeatsy", "<repeat-statement>");
    addChild(node, makeNode("repeatsy"));
    addChild(node, parseStatementList());
    consume("untilsy", "<repeat-statement>");
    addChild(node, makeNode("untilsy"));
    addChild(node, parseExpression());
    return node;
}

NodePtr StatementSubprogramParser::parseForStatement() {
    NodePtr node = makeNode("<for-statement>");
    consume("forsy", "<for-statement>");
    addChild(node, makeNode("forsy"));

    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else consume("ident", "<for-statement>");

    consume("becomes", "<for-statement>");
    addChild(node, makeNode("becomes"));
    addChild(node, parseExpression());

    if (check("tosy") || check("downtosy")) {
        addChild(node, makeNode(peek().type));
        advance();
    } else {
        consume("tosy", "<for-statement>");
    }

    addChild(node, parseExpression());
    consume("dosy", "<for-statement>");
    addChild(node, makeNode("dosy"));
    // revisi M3: body wajib compound-statement, bkn single statement
    addChild(node, parseCompoundStatement());
    // semicolon trailing bagian dari produksi <for-statement>
    if (match("semicolon")) addChild(node, makeNode("semicolon"));
    else consume("semicolon", "<for-statement>");
    return node;
}

NodePtr StatementSubprogramParser::parseProcedureFunctionCall() {
    NodePtr node = makeNode("<procedure/function-call>");
    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else consume("ident", "<procedure/function-call>");

    if (check("lparent")) {
        advance();
        addChild(node, makeNode("lparent"));
        if (!check("rparent")) {
            addChild(node, parseParameterList());
        }
        consume("rparent", "<procedure/function-call>");
        addChild(node, makeNode("rparent"));
    }
    return node;
}

NodePtr StatementSubprogramParser::parseParameterList() {
    NodePtr node = makeNode("<parameter-list>");
    addChild(node, parseExpression());
    while (check("comma")) {
        advance();
        addChild(node, makeNode("comma"));
        addChild(node, parseExpression());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseVariable() {
    NodePtr node = makeNode("<variable>");
    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else consume("ident", "<variable>");

    while (check("lbrack") || check("period")) {
        addChild(node, parseComponentVariable());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseComponentVariable() {
    NodePtr node = makeNode("<component-variable>");
    if (check("lbrack")) {
        advance();
        addChild(node, makeNode("lbrack"));
        addChild(node, parseIndexList());
        consume("rbrack", "<component-variable>");
        addChild(node, makeNode("rbrack"));
    } else if (check("period")) {
        advance();
        addChild(node, makeNode("period"));
        if (check("ident")) {
            addChild(node, makeNode("ident(" + peek().value + ")"));
            advance();
        } else consume("ident", "<component-variable>");
    } else {
        consume("lbrack", "<component-variable>");
    }
    return node;
}

NodePtr StatementSubprogramParser::parseIndexList() {
    NodePtr node = makeNode("<index-list>");
    Token t = peek();
    if (t.type == "intcon" || t.type == "charcon" || t.type == "ident") {
        string label = t.type;
        if (t.type == "charcon") label += "('" + t.value + "')";
        else if (!t.value.empty()) label += "(" + t.value + ")";
        addChild(node, makeNode(label));
        advance();
    } else {
        consume("intcon", "<index-list>");
    }

    if (check("comma")) {
        advance();
        addChild(node, makeNode("comma"));
        addChild(node, parseIndexList());
    }
    return node;
}

NodePtr StatementSubprogramParser::parseSubprogramDeclaration() {
    NodePtr node = makeNode("<subprogram-declaration>");
    if (check("proceduresy")) {
        addChild(node, parseProcedureDeclaration());
    } else if (check("functionsy")) {
        addChild(node, parseFunctionDeclaration());
    } else {
        consume("proceduresy", "<subprogram-declaration>");
    }
    return node;
}

NodePtr StatementSubprogramParser::parseProcedureDeclaration() {
    NodePtr node = makeNode("<procedure-declaration>");
    consume("proceduresy", "<procedure-declaration>");
    addChild(node, makeNode("proceduresy"));

    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else consume("ident", "<procedure-declaration>");

    if (check("lparent")) {
        addChild(node, parseFormalParameterList());
    }

    consume("semicolon", "<procedure-declaration>");
    addChild(node, makeNode("semicolon"));

    addChild(node, parseBlock());

    consume("semicolon", "<procedure-declaration>");
    addChild(node, makeNode("semicolon"));
    return node;
}

NodePtr StatementSubprogramParser::parseFunctionDeclaration() {
    NodePtr node = makeNode("<function-declaration>");
    consume("functionsy", "<function-declaration>");
    addChild(node, makeNode("functionsy"));

    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else consume("ident", "<function-declaration>");

    if (check("lparent")) {
        addChild(node, parseFormalParameterList());
    }

    consume("colon", "<function-declaration>");
    addChild(node, makeNode("colon"));

    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else consume("ident", "<function-declaration>");

    consume("semicolon", "<function-declaration>");
    addChild(node, makeNode("semicolon"));

    addChild(node, parseBlock());

    consume("semicolon", "<function-declaration>");
    addChild(node, makeNode("semicolon"));
    return node;
}

NodePtr StatementSubprogramParser::parseBlock() {
    NodePtr node = makeNode("<block>");
    addChild(node, parseDeclarationPart());
    while (check("proceduresy") || check("functionsy")) {
        addChild(node, parseSubprogramDeclaration());
    }
    addChild(node, parseCompoundStatement());
    return node;
}

NodePtr StatementSubprogramParser::parseFormalParameterList() {
    NodePtr node = makeNode("<formal-parameter-list>");
    consume("lparent", "<formal-parameter-list>");
    addChild(node, makeNode("lparent"));

    addChild(node, parseParameterGroup());

    while (check("semicolon")) {
        advance();
        addChild(node, makeNode("semicolon"));
        addChild(node, parseParameterGroup());
    }

    consume("rparent", "<formal-parameter-list>");
    addChild(node, makeNode("rparent"));
    return node;
}

NodePtr StatementSubprogramParser::parseParameterGroup() {
    NodePtr node = makeNode("<parameter-group>");
    addChild(node, parseIdentifierList());

    consume("colon", "<parameter-group>");
    addChild(node, makeNode("colon"));

    if (check("arraysy")) {
        addChild(node, parseArrayType());
    } else if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else {
        consume("ident", "<parameter-group>");
    }

    return node;
}

NodePtr StatementSubprogramParser::parseExpression() {
    if (!exprParser_) return makeNode("<expression>");
    exprParser_->clearError();
    exprParser_->setPosition(pos);
    NodePtr n = exprParser_->parseExpression();
    pos = exprParser_->getPosition();
    if (exprParser_->hasError() && errorMessage.empty()) errorMessage = exprParser_->error();
    return n;
}

NodePtr StatementSubprogramParser::parseConstant() {
    if (!declParser_) return makeNode("<constant>");
    declParser_->setPosition(pos);
    NodePtr n = declParser_->parseConstant();
    pos = declParser_->getPosition();
    return n;
}

NodePtr StatementSubprogramParser::parseDeclarationPart() {
    if (!declParser_) return makeNode("<declaration-part>");
    declParser_->setPosition(pos);
    NodePtr n = declParser_->parseDeclarationPart();
    pos = declParser_->getPosition();
    return n;
}

NodePtr StatementSubprogramParser::parseIdentifierList() {
    if (!declParser_) return makeNode("<identifier-list>");
    declParser_->setPosition(pos);
    NodePtr n = declParser_->parseIdentifierList();
    pos = declParser_->getPosition();
    return n;
}

NodePtr StatementSubprogramParser::parseArrayType() {
    if (!declParser_) return makeNode("<array-type>");
    declParser_->setPosition(pos);
    NodePtr n = declParser_->parseArrayType();
    pos = declParser_->getPosition();
    return n;
}

NodePtr StatementSubprogramParser::parseStatementList() {
    NodePtr node = makeNode("<statement-list>");
    size_t before = pos;
    NodePtr prev = parseStatement();
    addChild(node, prev);
    if (pos == before && !isAtEnd() && !check("semicolon")) {
        return node;
    }
    while (true) {
        // while/for udah consume ';' sendiri (revisi M3)
        bool ownsSemicolon = prev && (prev->label == "<while-statement>" || prev->label == "<for-statement>")
                             && !prev->children.empty() && prev->children.back()->label == "semicolon";
        if (!ownsSemicolon) {
            if (!check("semicolon")) break;
            advance();
            addChild(node, makeNode("semicolon"));
        }
        if (check("endsy") || check("untilsy") || isAtEnd()) break;
        size_t b = pos;
        prev = parseStatement();
        addChild(node, prev);
        if (pos == b && !check("semicolon")) break;
    }
    return node;
}

NodePtr StatementSubprogramParser::parseCompoundStatement() {
    NodePtr node = makeNode("<compound-statement>");
    consume("beginsy", "<compound-statement>");
    addChild(node, makeNode("beginsy"));
    addChild(node, parseStatementList());
    consume("endsy", "<compound-statement>");
    addChild(node, makeNode("endsy"));
    return node;
}
