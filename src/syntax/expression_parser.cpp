#include "expression_parser.hpp"

NodePtr ExpressionParser::parseFactor() {
    NodePtr node = makeNode("<factor>");
    Token t = peek();
    if (t.type == "intcon" || t.type == "realcon" || t.type == "charcon" || t.type == "string") {
        // literal: charcon/string dikutip pake single-quote sesuai output lama
        string label = t.type;
        if (t.type == "charcon" || t.type == "string") label += "('" + t.value + "')";
        else label += "(" + t.value + ")";
        addChild(node, makeNode(label));
        advance();
    } else if (t.type == "ident") {
        // ident bisa call (foo(...)) atau variabel (foo[..]/foo.x) atau plain
        Token n = peek(1);
        if (n.type == "lparent") {
            addChild(node, parseProcedureFunctionCall());
        } else if (n.type == "lbrack" || n.type == "period") {
            addChild(node, parseVariable());
        } else {
            addChild(node, makeNode("ident(" + t.value + ")"));
            advance();
        }
    } else if (t.type == "lparent") {
        // ( expression )
        consume("lparent", "<factor>");
        addChild(node, makeNode("lparent"));
        addChild(node, parseExpression());
        consume("rparent", "<factor>");
        addChild(node, makeNode("rparent"));
    } else if (t.type == "notsy") {
        // not factor
        consume("notsy", "<factor>");
        addChild(node, makeNode("notsy"));
        addChild(node, parseFactor());
    } else {
        // token ga dikenali, simpen error pertama
        if (errorMessage.empty()) errorMessage = "unexpected token in <factor>: " + t.type;
    }
    return node;
}

NodePtr ExpressionParser::parseVariable() {
    NodePtr node = makeNode("<variable>");
    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else {
        consume("ident", "<variable>");
    }
    // variabel komposit: array indexing (foo[i]) atau record field (foo.x)
    while (check("lbrack") || check("period")) {
        NodePtr comp = makeNode("<component-variable>");
        if (check("lbrack")) {
            advance();
            addChild(comp, makeNode("lbrack"));
            addChild(comp, parseIndexList());
            consume("rbrack", "<component-variable>");
            addChild(comp, makeNode("rbrack"));
        } else if (check("period")) {
            advance();
            addChild(comp, makeNode("period"));
            if (check("ident")) {
                addChild(comp, makeNode("ident(" + peek().value + ")"));
                advance();
            } else {
                consume("ident", "<component-variable>");
            }
        }
        addChild(node, comp);
    }
    return node;
}

NodePtr ExpressionParser::parseProcedureFunctionCall() {
    NodePtr node = makeNode("<procedure/function-call>");
    if (check("ident")) {
        addChild(node, makeNode("ident(" + peek().value + ")"));
        advance();
    } else {
        consume("ident", "<procedure/function-call>");
    }
    if (check("lparent")) {
        advance();
        addChild(node, makeNode("lparent"));
        if (!check("rparent")) {
            // parameter list
            addChild(node, parseExpression());
            while (check("comma")) {
                addChild(node, makeNode("comma"));
                advance();
                addChild(node, parseExpression());
            }
        }
        consume("rparent", "<procedure/function-call>");
        addChild(node, makeNode("rparent"));
    }
    return node;
}

NodePtr ExpressionParser::parseIndexList() {
    NodePtr node = makeNode("<index-list>");
    Token t = peek();
    if (t.type == "intcon" || t.type == "charcon" || t.type == "ident") {
        string label = t.type;
        if (t.type == "charcon") label += "('" + t.value + "')";
        else label += "(" + t.value + ")";
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

NodePtr ExpressionParser::parseTerm() {
    NodePtr node = makeNode("<term>");
    addChild(node, parseFactor());
    // operator multiplicative + 'and'
    while (check("times") || check("idiv") || check("rdiv") || check("imod") || check("andsy")) {
        addChild(node, makeNode(peek().type));
        advance();
        addChild(node, parseFactor());
    }
    return node;
}

NodePtr ExpressionParser::parseSimpleExpression() {
    NodePtr node = makeNode("<simple-expression>");
    // unary +/- di awal (opsional)
    if (check("plus") || check("minus")) {
        addChild(node, makeNode(peek().type));
        advance();
    }
    addChild(node, parseTerm());
    // operator additive + 'or'
    while (check("plus") || check("minus") || check("orsy")) {
        addChild(node, makeNode(peek().type));
        advance();
        addChild(node, parseTerm());
    }
    return node;
}

NodePtr ExpressionParser::parseExpression() {
    NodePtr node = makeNode("<expression>");
    addChild(node, parseSimpleExpression());
    // operator relational (opsional, satu kali)
    if (check("eql") || check("neq") || check("lss") || check("leq") || check("gtr") || check("geq")) {
        addChild(node, makeNode(peek().type));
        advance();
        addChild(node, parseSimpleExpression());
    }
    return node;
}
