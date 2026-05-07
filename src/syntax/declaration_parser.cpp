#include "declaration_parser.hpp"

DeclarationParser::DeclarationParser(const vector<Token>& tokens) : tokens(tokens), pos(0) {}

const string& DeclarationParser::error() const {
    return errorMessage;
}

void DeclarationParser::setPosition(size_t p) {
    pos = p;
}

size_t DeclarationParser::getPosition() const {
    return pos;
}

const Token* DeclarationParser::peek(size_t offset) const {
    // Lookahead token tanpa memindahkan posisi parser
    size_t index = pos + offset;
    if (index >= tokens.size()){
        return nullptr;
    }

    return &tokens[index];
}

bool DeclarationParser::end() const {
    return pos >= tokens.size();
}

bool DeclarationParser::check(const string& type) const {
    const Token* token = peek();
    return token != nullptr && token->type == type;
}

bool DeclarationParser::match(const string& type) {
    if (!check(type)){
        return false;
    }

    next();
    return true;
}

const Token& DeclarationParser::next() {
    if (end()) {
        return tokens.back();
    }
    
    return tokens[pos++];
}

const Token& DeclarationParser::prev() const {
    return tokens[pos - 1];
}

NodePtr DeclarationParser::makeTokenNode(const Token& token) const {
    if (token.value.empty()) {
        return makeNode(token.type);
    }

    return makeNode(token.type + "(" + token.value + ")");
}

void DeclarationParser::setError(const string& expected, const Token& found) {
    errorMessage = "Syntax error: unexpected token " + found.type;
    if (!found.value.empty()) {
        errorMessage += "(" + found.value + ")";
    }

    errorMessage += ", expected " + expected;
}

bool DeclarationParser::isConstantStart(size_t offset) const {
    const Token* token = peek(offset);
    if (!token){
        return false;
    }

    return token->type == "ident" || token->type == "intcon" || token->type == "realcon" || token->type == "charcon" || token->type == "string" || token->type == "plus" || token->type == "minus";
}

bool DeclarationParser::isRangeAhead(size_t offset) const {
    const Token* first = peek(offset);
    const Token* second = peek(offset + 1);
    const Token* third = peek(offset + 2);
    return first && second && third && isConstantStart(offset) && second->type == "period" && third->type == "period";
}

NodePtr DeclarationParser::parseDeclarationPart() {
    // baca deklarasi sampai ketemu begin
    errorMessage.clear();

    auto root = makeNode("<declaration-part>");

    if (check("programsy")) {
        addChild(root, parseProgramHeader());
    }

    while (!end() && !check("beginsy")) {
        size_t before = pos;

        // sequential (harus urut)
        if (check("constsy")) {
            addChild(root, parseConstDeclaration());
        }

        if (!errorMessage.empty()){
            return root;
        }

        if (check("typesy")) {
            addChild(root, parseTypeDeclaration());
        } else if (check("varsy")) {
            addChild(root, parseVarDeclaration());
        } else {
            // token lain (misal proceduresy/functionsy), stop parsing declarations
            break;
        }

        if (!errorMessage.empty()){
            return root;
        }

        // safety: kalau tidak ada progress (token bukan const/type/var/begin), keluar
        if (pos == before) break;
    }

    return root;
}

NodePtr DeclarationParser::parseProgramHeader() {
    auto node = makeNode("<program-header>");

    if (!match("programsy")) {
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    if (!check("ident")) {
        setError("ident", *peek());
        return node;
    }
    addChild(node, makeTokenNode(next()));

    if (!match("semicolon")) {
        setError("semicolon", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    return node;
}

NodePtr DeclarationParser::parseConstDeclaration() {
    errorMessage.clear();
    auto node = makeNode("<const-declaration>");

    if (!match("constsy")) {
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    while (check("ident")) {
        auto item = makeNode("<const-item>");
        addChild(item, makeTokenNode(next()));

        if (!match("eql")) {
            setError("eql", *peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(prev()));

        addChild(item, parseConstant());

        if (!match("semicolon")) {
            setError("semicolon", *peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(prev()));
        addChild(node, item);
    }

    if (node->children.size() == 1) {
        setError("ident", *peek());
    }

    return node;
}

NodePtr DeclarationParser::parseTypeDeclaration() {
    errorMessage.clear();
    auto node = makeNode("<type-declaration>");

    if (!match("typesy")) {
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    while (check("ident")) {
        auto item = makeNode("<type-item>");
        addChild(item, makeTokenNode(next()));

        if (!match("eql")) {
            setError("eql", *peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(prev()));

        addChild(item, parseType());

        if (!match("semicolon")) {
            setError("semicolon", *peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(prev()));
        addChild(node, item);
    }

    if (node->children.size() == 1) {
        setError("ident", *peek());
    }

    return node;
}

NodePtr DeclarationParser::parseVarDeclaration() {
    errorMessage.clear();
    auto node = makeNode("<var-declaration>");

    if (!match("varsy")) {
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    while (check("ident")) {
        auto item = makeNode("<var-item>");
        addChild(item, parseIdentifierList());

        if (!match("colon")) {
            setError("colon", *peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(prev()));

        addChild(item, parseType());

        if (!match("semicolon")) {
            setError("semicolon", *peek());
            addChild(node, item);
            return node;
        }
        addChild(item, makeTokenNode(prev()));
        addChild(node, item);
    }

    if (node->children.size() == 1) {
        setError("ident", *peek());
    }

    return node;
}

NodePtr DeclarationParser::parseIdentifierList() {
    errorMessage.clear();
    auto node = makeNode("<identifier-list>");

    if (!check("ident")) {
        setError("ident", *peek());
        return node;
    }

    addChild(node, makeTokenNode(next()));

    while (match("comma")) {
        addChild(node, makeTokenNode(prev()));
        if (!check("ident")) {
            setError("ident", *peek());
            return node;
        }
        addChild(node, makeTokenNode(next()));
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
        addChild(node, makeTokenNode(next()));
        return node;
    }

    setError("type", *peek());
    return node;
}

NodePtr DeclarationParser::parseArrayType() {
    errorMessage.clear();
    auto node = makeNode("<array-type>");

    if (!match("arraysy")) {
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    if (!match("lbrack")) {
        setError("lbrack", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    if (isRangeAhead()) {
        addChild(node, parseRange());
    } else if (check("ident")) {
        addChild(node, makeTokenNode(next()));
    } else {
        setError("range or ident", *peek());
        return node;
    }

    if (!match("rbrack")) {
        setError("rbrack", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    if (!match("ofsy")) {
        setError("ofsy", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    addChild(node, parseType());
    return node;
}

NodePtr DeclarationParser::parseRange() {
    auto node = makeNode("<range>");

    addChild(node, parseConstant());

    if (!match("period")) {
        setError("period", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    if (!match("period")) {
        setError("period", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    addChild(node, parseConstant());
    return node;
}

NodePtr DeclarationParser::parseEnumerated() {
    auto node = makeNode("<enumerated>");

    if (!match("lparent")) {
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    if (!check("ident")) {
        setError("ident", *peek());
        return node;
    }
    addChild(node, makeTokenNode(next()));

    while (match("comma")) {
        addChild(node, makeTokenNode(prev()));
        if (!check("ident")) {
            setError("ident", *peek());
            return node;
        }
        addChild(node, makeTokenNode(next()));
    }

    if (!match("rparent")) {
        setError("rparent", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    return node;
}

NodePtr DeclarationParser::parseRecordType() {
    auto node = makeNode("<record-type>");

    if (!match("recordsy")) {
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    addChild(node, parseFieldList());

    if (!match("endsy")) {
        setError("endsy", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    return node;
}

NodePtr DeclarationParser::parseFieldList() {
    auto node = makeNode("<field-list>");

    if (!check("ident")) {
        setError("ident", *peek());
        return node;
    }

    addChild(node, parseFieldPart());

    while (match("semicolon")) {
        if (check("endsy")) {
            // addChild(node, makeTokenNode(prev()));
            break;
        }

        addChild(node, makeTokenNode(prev()));
        addChild(node, parseFieldPart());
    }

    return node;
}

NodePtr DeclarationParser::parseFieldPart() {
    auto node = makeNode("<field-part>");

    addChild(node, parseIdentifierList());

    if (!match("colon")) {
        setError("colon", *peek());
        return node;
    }
    addChild(node, makeTokenNode(prev()));

    addChild(node, parseType());
    return node;
}

NodePtr DeclarationParser::parseConstant() {
    errorMessage.clear();
    auto node = makeNode("<constant>");

    if (check("charcon") || check("string")) {
        addChild(node, makeTokenNode(next()));
        return node;
    }

    if (check("plus") || check("minus")) {
        addChild(node, makeTokenNode(next()));
    }

    if (!peek()) {
        setError("constant", Token{"<eof>", ""});
        return node;
    }

    if (check("ident") || check("intcon") || check("realcon") || check("charcon") || check("string")) {
        addChild(node, makeTokenNode(next()));
        return node;
    }

    setError("constant", *peek());
    return node;
}