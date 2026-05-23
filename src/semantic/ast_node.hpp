#pragma once
#include "../std.hpp"

// ast kind
enum class AstKind {
    Program,
    VarDecl,
    ConstDecl,
    TypeDecl,
    Block,
    Declarations,
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

    // semantic
    string typeName;   // hasil inferensi tipe
    int tabIndex = -1; // referensi ke entry symbol table
    int lev = -1;      // lexical level
    int blockIndex = -1; // referensi ke entry btab (utk Compound block)
    bool predefined = false; // true utk predefined identifier (writeln/readln/dll)

    // payload string
    string value;

    // payload extra
    string extra;

    vector<AstNodePtr> children;

    explicit AstNode(AstKind k) : kind(k) {}
    AstNode(AstKind k, const string& val) : kind(k), value(val) {}
};

// factory
AstNodePtr makeAst(AstKind kind);
AstNodePtr makeAst(AstKind kind, const string& value);
void addAstChild(const AstNodePtr& parent, const AstNodePtr& child);

// kind name
string astKindName(AstKind kind);
