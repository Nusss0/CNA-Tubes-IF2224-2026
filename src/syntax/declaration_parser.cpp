#include "declaration_parser.hpp"

// override base utk ngasih kutip single-quote di charcon/string,
// supaya output parse tree konsisten sm ExprParser/StmtParser
NodePtr DeclarationParser::makeTokenNode(const Token& token) const {
    if (token.value.empty()) {
        return makeNode(token.type);
    }
    if (token.type == "charcon" || token.type == "string") {
        return makeNode(token.type + "('" + token.value + "')");
    }
    return makeNode(token.type + "(" + token.value + ")");
}

// format error legacy sub-parser: "Syntax error: unexpected token X(value), expected Y"
void DeclarationParser::setError(const string& expected, const Token& found) {
    errorMessage = "Syntax error: unexpected token " + found.type;
    if (!found.value.empty()) {
        errorMessage += "(" + found.value + ")";
    }
    errorMessage += ", expected " + expected;
}

bool DeclarationParser::isConstantStart(size_t offset) const {
    if (pos + offset >= tokens.size()) return false;
    const string& type = tokens[pos + offset].type;
    return type == "ident" || type == "intcon" || type == "realcon"
        || type == "charcon" || type == "string" || type == "plus" || type == "minus";
}

bool DeclarationParser::isRangeAhead(size_t offset) const {
    // pola: constant '..' constant -> butuh 3 token kedepan
    if (pos + offset + 2 >= tokens.size()) return false;
    return isConstantStart(offset)
        && tokens[pos + offset + 1].type == "period"
        && tokens[pos + offset + 2].type == "period";
}

NodePtr DeclarationParser::parseDeclarationPart() {
    // baca const/type/var section secara urut. subprogram (proc/func)
    // di-handle Parser utama, bukan disini.
    errorMessage.clear();
    auto root = makeNode("<declaration-part>");

    // const section (opsional, harus paling awal)
    while (check("constsy")) {
        addChild(root, parseConstDeclaration());
        if (!errorMessage.empty()) return root;
    }
    // type section (opsional, setelah const)
    while (check("typesy")) {
        addChild(root, parseTypeDeclaration());
        if (!errorMessage.empty()) return root;
    }
    // var section (opsional, setelah type)
    while (check("varsy")) {
        addChild(root, parseVarDeclaration());
        if (!errorMessage.empty()) return root;
    }

    return root;
}

NodePtr DeclarationParser::parseProgramHeader() {
    auto node = makeNode("<program-header>");

    if (!check("programsy")) return node;
    addChild(node, makeTokenNode(advance()));

    if (!check("ident")) {
        setError("ident", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    if (!check("semicolon")) {
        setError("semicolon", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    return node;
}

NodePtr DeclarationParser::parseConstDeclaration() {
    errorMessage.clear();
    auto node = makeNode("<const-declaration>");

    if (!check("constsy")) return node;
    addChild(node, makeTokenNode(advance()));

    while (check("ident")) {
        auto item = makeNode("<const-item>");
        addChild(item, makeTokenNode(advance()));

        if (!check("eql")) {
            setError("eql", peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(advance()));

        addChild(item, parseConstant());

        if (!check("semicolon")) {
            setError("semicolon", peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(advance()));
        addChild(node, item);
    }

    if (node->children.size() == 1) {
        setError("ident", peek());
    }

    return node;
}

NodePtr DeclarationParser::parseTypeDeclaration() {
    errorMessage.clear();
    auto node = makeNode("<type-declaration>");

    if (!check("typesy")) return node;
    addChild(node, makeTokenNode(advance()));

    while (check("ident")) {
        auto item = makeNode("<type-item>");
        addChild(item, makeTokenNode(advance()));

        if (!check("eql")) {
            setError("eql", peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(advance()));

        addChild(item, parseType());

        if (!check("semicolon")) {
            setError("semicolon", peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(advance()));
        addChild(node, item);
    }

    if (node->children.size() == 1) {
        setError("ident", peek());
    }

    return node;
}

NodePtr DeclarationParser::parseVarDeclaration() {
    errorMessage.clear();
    auto node = makeNode("<var-declaration>");

    if (!check("varsy")) return node;
    addChild(node, makeTokenNode(advance()));

    while (check("ident")) {
        auto item = makeNode("<var-item>");
        addChild(item, parseIdentifierList());

        if (!check("colon")) {
            setError("colon", peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(advance()));

        addChild(item, parseType());

        if (!check("semicolon")) {
            setError("semicolon", peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(advance()));
        addChild(node, item);
    }

    if (node->children.size() == 1) {
        setError("ident", peek());
    }

    return node;
}

NodePtr DeclarationParser::parseIdentifierList() {
    errorMessage.clear();
    auto node = makeNode("<identifier-list>");

    if (!check("ident")) {
        setError("ident", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    while (check("comma")) {
        addChild(node, makeTokenNode(advance()));
        if (!check("ident")) {
            setError("ident", peek());
            return node;
        }
        addChild(node, makeTokenNode(advance()));
    }

    return node;
}

NodePtr DeclarationParser::parseType() {
    auto node = makeNode("<type>");

    if (isRangeAhead()) {
        addChild(node, parseRange());
        return node;
    }
    if (check("arraysy")) {
        addChild(node, parseArrayType());
        return node;
    }
    if (check("recordsy")) {
        addChild(node, parseRecordType());
        return node;
    }
    if (check("lparent")) {
        addChild(node, parseEnumerated());
        return node;
    }
    if (check("ident")) {
        addChild(node, makeTokenNode(advance()));
        return node;
    }

    setError("type", peek());
    return node;
}

NodePtr DeclarationParser::parseArrayType() {
    errorMessage.clear();
    auto node = makeNode("<array-type>");

    if (!check("arraysy")) return node;
    addChild(node, makeTokenNode(advance()));

    if (!check("lbrack")) {
        setError("lbrack", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    if (isRangeAhead()) {
        addChild(node, parseRange());
    } else if (check("ident")) {
        addChild(node, makeTokenNode(advance()));
    } else {
        setError("range or ident", peek());
        return node;
    }

    if (!check("rbrack")) {
        setError("rbrack", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    if (!check("ofsy")) {
        setError("ofsy", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    addChild(node, parseType());
    return node;
}

NodePtr DeclarationParser::parseRange() {
    auto node = makeNode("<range>");

    addChild(node, parseConstant());

    if (!check("period")) {
        setError("period", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    if (!check("period")) {
        setError("period", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    addChild(node, parseConstant());
    return node;
}

NodePtr DeclarationParser::parseEnumerated() {
    auto node = makeNode("<enumerated>");

    if (!check("lparent")) return node;
    addChild(node, makeTokenNode(advance()));

    if (!check("ident")) {
        setError("ident", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    while (check("comma")) {
        addChild(node, makeTokenNode(advance()));
        if (!check("ident")) {
            setError("ident", peek());
            return node;
        }
        addChild(node, makeTokenNode(advance()));
    }

    if (!check("rparent")) {
        setError("rparent", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    return node;
}

NodePtr DeclarationParser::parseRecordType() {
    auto node = makeNode("<record-type>");

    if (!check("recordsy")) return node;
    addChild(node, makeTokenNode(advance()));

    addChild(node, parseFieldList());

    if (!check("endsy")) {
        setError("endsy", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    return node;
}

NodePtr DeclarationParser::parseFieldList() {
    auto node = makeNode("<field-list>");

    if (!check("ident")) {
        setError("ident", peek());
        return node;
    }

    addChild(node, parseFieldPart());

    while (check("semicolon")) {
        Token semi = advance();
        if (check("endsy")) break;
        addChild(node, makeTokenNode(semi));
        addChild(node, parseFieldPart());
    }

    return node;
}

NodePtr DeclarationParser::parseFieldPart() {
    auto node = makeNode("<field-part>");

    addChild(node, parseIdentifierList());

    if (!check("colon")) {
        setError("colon", peek());
        return node;
    }
    addChild(node, makeTokenNode(advance()));

    addChild(node, parseType());
    return node;
}

NodePtr DeclarationParser::parseConstant() {
    errorMessage.clear();
    auto node = makeNode("<constant>");

    if (check("charcon") || check("string")) {
        addChild(node, makeTokenNode(advance()));
        return node;
    }

    if (check("plus") || check("minus")) {
        addChild(node, makeTokenNode(advance()));
    }

    if (isAtEnd()) {
        setError("constant", Token{"<eof>", ""});
        return node;
    }

    if (check("ident") || check("intcon") || check("realcon") || check("charcon") || check("string")) {
        addChild(node, makeTokenNode(advance()));
        return node;
    }

    setError("constant", peek());
    return node;
}
