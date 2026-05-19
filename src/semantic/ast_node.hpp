#pragma once
#include "../std.hpp"

// kind tag utk dispatch visitor (switch lbh murah drpd dynamic_cast)
enum class AstKind {
    Program,
    VarDecl,
    ConstDecl,
    TypeDecl,
    Block,
    Assign,
    BinOp,
    UnaryOp,
    Var,
    Number,
    RealNumber,
    CharLit,
    StringLit,
    BoolLit,
    ProcCall,
    FuncCall,
    If,
    While,
    For,
    Repeat,
    Case,
    Compound,
    Empty,
    SubprogramDecl,
    FieldAccess,
    IndexAccess,
    TypeRef,
    ArrayType,
    RecordType,
    EnumType,
    RangeType,
    SubprogramHeader
};

struct AstNode;
using AstNodePtr = shared_ptr<AstNode>;

struct AstNode {
    AstKind kind;

    //semantic 
    string typeName;   // hasil inferensi tipe 
    int tabIndex = -1; // referensi ke entry symbol table
    int lev = -1;      // lexical level

    // payload string
    string value;

    // payload extra
    string extra;

    vector<AstNodePtr> children;

    explicit AstNode(AstKind k) : kind(k) {}
    AstNode(AstKind k, const string& val) : kind(k), value(val) {}
};

// factory + helper
AstNodePtr makeAst(AstKind kind);
AstNodePtr makeAst(AstKind kind, const string& value);
void addAstChild(const AstNodePtr& parent, const AstNodePtr& child);

// nama human-readable utk kind
string astKindName(AstKind kind);
