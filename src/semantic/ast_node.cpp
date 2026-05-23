#include "ast_node.hpp"

AstNodePtr makeAst(AstKind kind) {
    return make_shared<AstNode>(kind);
}

AstNodePtr makeAst(AstKind kind, const string& value) {
    return make_shared<AstNode>(kind, value);
}

void addAstChild(const AstNodePtr& parent, const AstNodePtr& child) {
    if (child) parent->children.push_back(child);
}

string astKindName(AstKind kind) {
    switch (kind) {
        case AstKind::Program:          return "Program";
        case AstKind::VarDecl:          return "VarDecl";
        case AstKind::ConstDecl:        return "ConstDecl";
        case AstKind::TypeDecl:         return "TypeDecl";
        case AstKind::Block:            return "Block";
        case AstKind::Declarations:     return "Declarations";
        case AstKind::Assign:           return "Assign";
        case AstKind::BinOp:            return "BinOp";
        case AstKind::UnaryOp:          return "UnaryOp";
        case AstKind::Var:              return "Var";
        case AstKind::Number:           return "Number";
        case AstKind::RealNumber:       return "RealNumber";
        case AstKind::CharLit:          return "Char";
        case AstKind::StringLit:        return "String";
        case AstKind::BoolLit:          return "Bool";
        case AstKind::ProcCall:         return "ProcCall";
        case AstKind::FuncCall:         return "FuncCall";
        case AstKind::If:               return "If";
        case AstKind::While:            return "While";
        case AstKind::For:              return "For";
        case AstKind::Repeat:           return "Repeat";
        case AstKind::Case:             return "Case";
        case AstKind::Compound:         return "Compound";
        case AstKind::Empty:            return "Empty";
        case AstKind::SubprogramDecl:   return "SubprogramDecl";
        case AstKind::FieldAccess:      return "FieldAccess";
        case AstKind::IndexAccess:      return "IndexAccess";
        case AstKind::TypeRef:          return "TypeRef";
        case AstKind::ArrayType:        return "ArrayType";
        case AstKind::RecordType:       return "RecordType";
        case AstKind::EnumType:         return "EnumType";
        case AstKind::RangeType:        return "RangeType";
        case AstKind::SubprogramHeader: return "SubprogramHeader";
    }
    return "Unknown";
}
