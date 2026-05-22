#pragma once

#include "../std.hpp"
#include "../syntax/parse_tree.hpp"
#include "symbol_table.hpp"
#include "type_check.hpp"

// SemanticType didefinisikan di type_check.hpp (modul aturan tipe).

class SemanticAnalyzer {
public:
    // analyzer semantic
    SemanticAnalyzer();

    // run analisis
    void analyze(const NodePtr& root);

    // cek apakah ada error
    bool hasErrors() const { return !errors.empty(); }

    // ambil daftar error
    const vector<string>& getErrors() const { return errors; }

    // print parse tree yang sudah dianotasi
    void printDecoratedTree(const NodePtr& root, ostream& out) const;

    // dump symbol table
    void printTabDump(ostream& out) const;

    // dump block table
    void printBTabDump(ostream& out) const;

    // dump address table
    void printATabDump(ostream& out) const;

private:
    SymbolTable symbols;
    vector<string> errors;

    // traversal utama
    SemanticType visit(const NodePtr& node);

    // analisis node program
    SemanticType visitProgram(const NodePtr& node);
    // analisis header program
    SemanticType visitProgramHeader(const NodePtr& node);
    // analisis bagian deklarasi
    SemanticType visitDeclarationPart(const NodePtr& node);
    // analisis konstanta
    SemanticType visitConstDeclaration(const NodePtr& node);
    // analisis tipe
    SemanticType visitTypeDeclaration(const NodePtr& node);
    // analisis variabel
    SemanticType visitVarDeclaration(const NodePtr& node);
    // analisis subprogram
    SemanticType visitSubprogramDeclaration(const NodePtr& node);
    // analisis procedure
    SemanticType visitProcedureDeclaration(const NodePtr& node);
    // analisis function
    SemanticType visitFunctionDeclaration(const NodePtr& node);
    // analisis blok
    SemanticType visitBlock(const NodePtr& node, bool pushScope);
    // analisis compound statement
    SemanticType visitCompoundStatement(const NodePtr& node);
    // analisis daftar statement
    SemanticType visitStatementList(const NodePtr& node);
    // analisis assignment
    SemanticType visitAssignmentStatement(const NodePtr& node);
    // analisis if
    SemanticType visitIfStatement(const NodePtr& node);
    // analisis while
    SemanticType visitWhileStatement(const NodePtr& node);
    // analisis for
    SemanticType visitForStatement(const NodePtr& node);
    // analisis pemanggilan procedure/function
    SemanticType visitProcedureFunctionCall(const NodePtr& node);
    // analisis variabel
    SemanticType visitVariable(const NodePtr& node);
    // analisis field/index variabel
    SemanticType visitComponentVariable(const NodePtr& node, SemanticType baseType);
    // analisis ekspresi
    SemanticType visitExpression(const NodePtr& node);
    // analisis simple expression
    SemanticType visitSimpleExpression(const NodePtr& node);
    // analisis term
    SemanticType visitTerm(const NodePtr& node);
    // analisis factor
    SemanticType visitFactor(const NodePtr& node);
    // analisis konstanta
    SemanticType visitConstant(const NodePtr& node);
    // analisis node tipe
    SemanticType visitTypeNode(const NodePtr& node);
    // analisis range
    SemanticType visitRange(const NodePtr& node);
    // analisis enumerated type
    SemanticType visitEnumerated(const NodePtr& node);
    // analisis array type
    SemanticType visitArrayType(const NodePtr& node);
    // analisis record type
    SemanticType visitRecordType(const NodePtr& node);
    // analisis formal parameter list
    SemanticType visitFormalParameterList(const NodePtr& node);
    // analisis grup parameter
    SemanticType visitParameterGroup(const NodePtr& node);
    // analisis identifier list
    SemanticType visitIdentifierList(const NodePtr& node, const SemanticType& type, const string& obj);

    // cek label token
    static bool isTokenLabel(const string& label, const string& tokenType, string* value = nullptr);

    // ambil nilai token
    static string extractTokenValue(const string& label);

    // ubah tipe jadi nama
    static string typeName(const SemanticType& type);

    // ubah kode tipe jadi nama
    static string TypeName(int type);

    // bentuk tipe dasar
    static SemanticType basicType(const string& name);

    // cek numeric
    static bool isNumeric(const SemanticType& type);

    // cek boolean
    static bool isBoolean(const SemanticType& type);

    // cek tipe sama
    static bool sameType(const SemanticType& lhs, const SemanticType& rhs);

    // cek kompatibilitas assignment
    static bool assignmentCompatible(const SemanticType& lhs, const SemanticType& rhs);

    // gabungkan dua operand lewat operator biner + lapor error bila tak cocok
    SemanticType applyBinary(const string& op, const SemanticType& a, const SemanticType& b);

    // cari tipe berdasarkan nama
    SemanticType lookupTypeByName(const string& name) const;

    // simpan error semantic
    void reportError(const string& message);

    // tulis anotasi ke node
    void annotate(const NodePtr& node, const SemanticType& type, int tabIndex = -1);

    // helper cetak tree
    void printDecoratedTreeImpl(const NodePtr& node, ostream& out, const string& prefix, bool isLast, bool isRoot) const;
    
    // label tree yang sudah diberi info
    string decoratedLabel(const NodePtr& node) const;
};