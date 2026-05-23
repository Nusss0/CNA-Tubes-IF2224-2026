#include "ast_decorator.hpp"

AstDecorator::AstDecorator(const SymbolTable& sym) : symbols(sym) {}

void AstDecorator::decorate(const AstNodePtr& root) {
    currBlock = 0;
    currLev = 0;
    visit(root);
}

string AstDecorator::typeNameFromCode(int code) const {
    switch (code) {
        case TC_INTEGER: return "integer";
        case TC_REAL:    return "real";
        case TC_BOOLEAN: return "boolean";
        case TC_CHAR:    return "char";
        case TC_STRING:  return "string";
        case TC_ARRAY:   return "array";
        case TC_RECORD:  return "record";
        default:         return "unknown";
    }
}

void AstDecorator::annotateFromTab(const AstNodePtr& node, const string& id) {
    int idx = symbols.lookup(id);
    if (idx >= 0) {
        const auto& e = symbols.getTab()[idx];
        node->tabIndex = idx;
        node->typeName = typeNameFromCode(e.type);
        node->lev = e.lev;
        // predefined entry
        if (idx < symbols.predefinedCutoff()) {
            node->predefined = true;
        }
    }
}

void AstDecorator::visit(const AstNodePtr& node) {
    if (!node) return;

    AstKind k = node->kind;

    // lookup nodes
    if (k == AstKind::Var) {
        annotateFromTab(node, node->value);
    } else if (k == AstKind::VarDecl || k == AstKind::ConstDecl || k == AstKind::TypeDecl) {
        annotateFromTab(node, node->value);
        // no block/lev propagation
    } else if (k == AstKind::ProcCall || k == AstKind::FuncCall) {
        annotateFromTab(node, node->value);
        // promote func call
        if (node->tabIndex >= 0) {
            const auto& e = symbols.getTab()[node->tabIndex];
            if (e.obj == (int)ObjClass::FUNCTION) {
                node->kind = AstKind::FuncCall;
                // return type already set
            } else if (e.obj == (int)ObjClass::PROCEDURE) {
                node->kind = AstKind::ProcCall;
                node->typeName = "void";
            }
        } else {
            // predefined procedure
            if (k == AstKind::ProcCall) node->typeName = "void";
        }
    } else if (k == AstKind::SubprogramDecl) {
        annotateFromTab(node, node->value);
    }
    // literals
    else if (k == AstKind::Number)     { node->typeName = "integer"; }
    else if (k == AstKind::RealNumber) { node->typeName = "real"; }
    else if (k == AstKind::CharLit)    { node->typeName = "char"; }
    else if (k == AstKind::StringLit)  { node->typeName = "string"; }
    else if (k == AstKind::BoolLit)    { node->typeName = "boolean"; }
    // control flow
    else if (k == AstKind::Assign || k == AstKind::If || k == AstKind::While ||
             k == AstKind::For || k == AstKind::Repeat || k == AstKind::Case ||
             k == AstKind::Compound || k == AstKind::Empty) {
        node->typeName = "void";
    }
    // type ref
    else if (k == AstKind::TypeRef) {
        node->typeName = node->value;
    }

    // compound block
    if (k == AstKind::Compound) {
        int savedBlock = currBlock;
        int savedLev = currLev;
        currBlock++;
        currLev++;
        node->lev = currLev;
        node->blockIndex = currBlock;
        for (auto& c : node->children) visit(c);
        currBlock = savedBlock;
        currLev = savedLev;
        return;
    }

    // type inference
    if (k == AstKind::BinOp) {
        for (auto& c : node->children) visit(c);
        string leftT, rightT;
        if (!node->children.empty()) leftT = node->children[0]->typeName;
        if (node->children.size() >= 2) rightT = node->children[1]->typeName;
        const string& op = node->value;
        if (op == "and" || op == "or" || op == "=" || op == "<>" || op == "<" ||
            op == "<=" || op == ">" || op == ">=") {
            node->typeName = "boolean";
        } else if (op == "div" || op == "mod") {
            node->typeName = "integer";
        } else if (op == "/") {
            node->typeName = "real";
        } else if (op == "+" || op == "-" || op == "*") {
            if (op == "+" && leftT == "string" && rightT == "string") {
                node->typeName = "string";
            } else if (leftT == "real" || rightT == "real") {
                node->typeName = "real";
            } else {
                node->typeName = "integer";
            }
        }
        return;
    }

    if (k == AstKind::UnaryOp) {
        for (auto& c : node->children) visit(c);
        if (node->value == "not") {
            node->typeName = "boolean";
        } else if (!node->children.empty()) {
            node->typeName = node->children[0]->typeName;
        }
        return;
    }

    // default: traverse children
    for (auto& c : node->children) visit(c);
}
