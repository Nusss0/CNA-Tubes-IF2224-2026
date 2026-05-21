#include "analyzer.hpp"

// helper
namespace {
string toLowerCopy(string text) {
    for (char& c : text){
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }

    return text;
}

bool isIdentLabel(const string& label, string& value) {
    if (label.rfind("ident(", 0) != 0) return false;
    auto begin = label.find('(');
    auto end = label.rfind(')');
    if (begin == string::npos || end == string::npos || end <= begin + 1) return false;
    value = label.substr(begin + 1, end - begin - 1);
    return true;
}

int lookupCurrentScope(SymbolTable& symbols, const string& id) {
    const auto& display = symbols.getDisplay();
    const auto& tab = symbols.getTab();
    const auto& btab = symbols.getBtab();
    int blockIdx = display[symbols.currentLevel()];
    int idx = btab[blockIdx].last;
    //case-insensitive utk konsisten sm SymbolTable::lookup
    auto eq = [](const string& a, const string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (tolower(static_cast<unsigned char>(a[i])) != tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    };
    while (idx != 0) {
        if (eq(tab[idx].id, id)) return idx;
        idx = tab[idx].link;
    }
    return -1;
}

}

SemanticAnalyzer::SemanticAnalyzer() = default;

SemanticType SemanticAnalyzer::basicType(const string& name) {
    SemanticType t;
    string lowered = toLowerCopy(name);
    if (lowered == "integer") t.type = TC_INTEGER;
    else if (lowered == "real") t.type = TC_REAL;
    else if (lowered == "char") t.type = TC_CHAR;
    else if (lowered == "boolean") t.type = TC_BOOLEAN;
    else if (lowered == "string") t.type = TC_STRING;
    else if (lowered == "void") t.type = TC_NOTYPE;
    else t.type = TC_NOTYPE;
    t.name = lowered;
    return t;
}

string SemanticAnalyzer::TypeName(int type) {
    switch (type) {
        case TC_INTEGER: return "integer";
        case TC_REAL: return "real";
        case TC_BOOLEAN: return "boolean";
        case TC_CHAR: return "char";
        case TC_STRING: return "string";
        case TC_ARRAY: return "array";
        case TC_RECORD: return "record";
        default: return "unknown";
    }
}

string SemanticAnalyzer::typeName(const SemanticType& type) {
    if (!type.name.empty()) return type.name;
    return TypeName(type.type);
}

bool SemanticAnalyzer::isNumeric(const SemanticType& type) {
    return type.type == TC_INTEGER || type.type == TC_REAL || type.name == "subrange";
}

bool SemanticAnalyzer::isBoolean(const SemanticType& type) {
    return type.type == TC_BOOLEAN;
}

bool SemanticAnalyzer::sameType(const SemanticType& lhs, const SemanticType& rhs) {
    if (lhs.type == TC_NOTYPE || rhs.type == TC_NOTYPE) return false;
    //tipe primitif: cukup banding type code (nama bisa kosong krn variabel ga simpan nama tipe)
    if (lhs.type == rhs.type && lhs.type != TC_ARRAY && lhs.type != TC_RECORD) return true;
    //tipe komposit: butuh nama match krn anonymous record/array bisa beda walau struktur sama
    if (lhs.type == rhs.type && lhs.name == rhs.name && !lhs.name.empty()) return true;
    if (lhs.name == "subrange" && rhs.type == TC_INTEGER) return true;
    if (rhs.name == "subrange" && lhs.type == TC_INTEGER) return true;
    return false;
}

bool SemanticAnalyzer::assignmentCompatible(const SemanticType& lhs, const SemanticType& rhs) {
    if (lhs.type == TC_NOTYPE || rhs.type == TC_NOTYPE) return true;
    if (sameType(lhs, rhs)) return true;
    if (lhs.type == TC_REAL && rhs.type == TC_INTEGER) return true;
    return false;
}

bool SemanticAnalyzer::isTokenLabel(const string& label, const string& tokenType, string* value) {
    if (label == tokenType) {
        if (value) value->clear();
        return true;
    }
    
    if (label.rfind(tokenType + "(", 0) != 0) return false;
    if (value) {
        auto begin = label.find('(');
        auto end = label.rfind(')');
        if (begin == string::npos || end == string::npos || end <= begin + 1) value->clear();
        else *value = label.substr(begin + 1, end - begin - 1);
    }
    return true;
}

string SemanticAnalyzer::extractTokenValue(const string& label) {
    auto begin = label.find('(');
    auto end = label.rfind(')');
    if (begin == string::npos || end == string::npos || end <= begin + 1) return "";
    return label.substr(begin + 1, end - begin - 1);
}

SemanticType SemanticAnalyzer::lookupTypeByName(const string& name) const {
    SemanticType type = basicType(name);
    if (type.type != TC_NOTYPE && (type.name == "integer" || type.name == "real" || type.name == "boolean" || type.name == "char" || type.name == "string")) {
        return type;
    }

    int idx = symbols.lookup(name);
    if (idx >= 0) {
        type.type = symbols.getTab()[idx].type;
        type.name = symbols.getTab()[idx].id;
        return type;
    }

    type.type = TC_NOTYPE;
    type.name = name;
    return type;
}

void SemanticAnalyzer::reportError(const string& message) {
    errors.push_back("[ERROR] " + message + " !");
}

void SemanticAnalyzer::annotate(const NodePtr& node, const SemanticType& type, int tabIndex) {
    if (!node) return;
    // simpan hasil analisis ke node parse tree
    node->sem_type = typeName(type);
    node->tab_index = tabIndex;
    node->lev = symbols.currentLevel();
    node->block_index = symbols.currentBlock();
}

void SemanticAnalyzer::analyze(const NodePtr& root) {
    // entry point semantic analysis
    errors.clear();
    if (!root) {
        reportError("semantic analysis received empty parse tree");
        return;
    }
    visit(root);
}

SemanticType SemanticAnalyzer::visit(const NodePtr& node) {
    if (!node) return SemanticType{};

    const string& label = node->label;
    if (label == "<program>") return visitProgram(node);
    if (label == "<program-header>") return visitProgramHeader(node);
    if (label == "<declaration-part>") return visitDeclarationPart(node);
    if (label == "<const-declaration>") return visitConstDeclaration(node);
    if (label == "<type-declaration>") return visitTypeDeclaration(node);
    if (label == "<var-declaration>") return visitVarDeclaration(node);
    if (label == "<subprogram-declaration>") return visitSubprogramDeclaration(node);
    if (label == "<procedure-declaration>") return visitProcedureDeclaration(node);
    if (label == "<function-declaration>") return visitFunctionDeclaration(node);
    if (label == "<block>") return visitBlock(node, false);
    if (label == "<compound-statement>") return visitCompoundStatement(node);
    if (label == "<statement-list>") return visitStatementList(node);
    if (label == "<assignment-statement>") return visitAssignmentStatement(node);
    if (label == "<if-statement>") return visitIfStatement(node);
    if (label == "<while-statement>") return visitWhileStatement(node);
    if (label == "<for-statement>") return visitForStatement(node);
    if (label == "<procedure/function-call>") return visitProcedureFunctionCall(node);
    if (label == "<variable>") return visitVariable(node);
    if (label == "<component-variable>") return visitComponentVariable(node, SemanticType{});
    if (label == "<expression>") return visitExpression(node);
    if (label == "<simple-expression>") return visitSimpleExpression(node);
    if (label == "<term>") return visitTerm(node);
    if (label == "<factor>") return visitFactor(node);
    if (label == "<constant>") return visitConstant(node);
    if (label == "<type>") return visitTypeNode(node);
    if (label == "<range>") return visitRange(node);
    if (label == "<enumerated>") return visitEnumerated(node);
    if (label == "<array-type>") return visitArrayType(node);
    if (label == "<record-type>") return visitRecordType(node);
    if (label == "<formal-parameter-list>") return visitFormalParameterList(node);
    if (label == "<parameter-group>") return visitParameterGroup(node);
    if (label == "<identifier-list>") return visitIdentifierList(node, SemanticType{}, "");

    if (label == "programsy" || label == "beginsy" || label == "endsy" || label == "semicolon" ||
        label == "lparent" || label == "rparent" || label == "lbrack" || label == "rbrack" ||
        label == "comma" || label == "period" || label == "colon" || label == "becomes" ||
        label == "ifsy" || label == "thensy" || label == "elsesy" || label == "whilesy" ||
        label == "dosy" || label == "forsy" || label == "tosy" || label == "downtosy" ||
        label == "ofsy" || label == "casesy" || label == "notsy" || label == "plus" ||
        label == "minus" || label == "times" || label == "idiv" || label == "rdiv" ||
        label == "imod" || label == "andsy" || label == "orsy" || label == "eql" ||
        label == "neq" || label == "lss" || label == "leq" || label == "gtr" || label == "geq") {
        annotate(node, SemanticType{});
        return SemanticType{};
    }

    string value;
    if (isTokenLabel(label, "ident", &value)) {
        int index = symbols.lookup(value);
        SemanticType type;
        if (index >= 0) {
            type.type = symbols.getTab()[index].type;
            type.name.clear(); // gunakan kode tipe, jangan nama identifier
            annotate(node, type, index);
        } else {
            type.name = value;
            reportError("undeclared identifier '" + value + "'");
            annotate(node, type);
        }
        return type;
    }

    if (isTokenLabel(label, "intcon", &value)) {
        SemanticType type = basicType("integer");
        annotate(node, type);
        return type;
    }

    if (isTokenLabel(label, "realcon", &value)) {
        SemanticType type = basicType("real");
        annotate(node, type);
        return type;
    }

    if (isTokenLabel(label, "charcon", &value)) {
        SemanticType type = basicType("char");
        annotate(node, type);
        return type;
    }

    if (isTokenLabel(label, "string", &value)) {
        SemanticType type = basicType("string");
        annotate(node, type);
        return type;
    }

    SemanticType result;
    for (const auto& child : node->children) result = visit(child);
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitProgram(const NodePtr& node) {
    // proses node program utama
    for (const auto& child : node->children){
        visit(child);
    }

    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitProgramHeader(const NodePtr& node) {
    // daftar nama program dan cek duplicate
    string programName;
    for (const auto& child : node->children) {
        if (isIdentLabel(child->label, programName)) break;
    }

    if (!programName.empty()) {
        int existing = lookupCurrentScope(symbols, programName);
        if (existing >= 0){
            reportError("redeclared program '" + programName + "'");
        } else {
            int idx = symbols.enter(programName, (int)ObjClass::PROGRAM, TC_NOTYPE, 0, true, symbols.currentLevel(), 0);
            annotate(node, basicType("void"), idx);
        }
    }

    for (const auto& child : node->children){
        visit(child);
    }

    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitDeclarationPart(const NodePtr& node) {
    // read all declaration sebelum blok program
    for (const auto& child : node->children){
        visit(child);
    }

    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitConstDeclaration(const NodePtr& node) {
    // masukkan konstanta ke symbol table
    for (const auto& child : node->children) {
        if (child->label == "<const-item>") {
            string name;
            SemanticType valueType;
            for (const auto& itemChild : child->children) {
                if (isIdentLabel(itemChild->label, name)) continue;
                if (itemChild->label == "<constant>") valueType = visitConstant(itemChild);
            }

            if (!name.empty()) {
                if (lookupCurrentScope(symbols, name) >= 0) reportError("redeclared constant '" + name + "'");
                else symbols.enter(name, (int)ObjClass::CONSTANT, valueType.type, 0, true, symbols.currentLevel(), 0);
            }
        }
    }
    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitTypeDeclaration(const NodePtr& node) {
    // masukkan tipe baru ke symbol table
    for (const auto& child : node->children) {
        if (child->label == "<type-item>") {
            string name;
            SemanticType resolved;
            for (const auto& itemChild : child->children) {
                if (isIdentLabel(itemChild->label, name)) continue;
                if (itemChild->label == "<type>") resolved = visitTypeNode(itemChild);
            }

            if (!name.empty()) {
                if (lookupCurrentScope(symbols, name) >= 0) reportError("redeclared type '" + name + "'");
                else symbols.enter(name, (int)ObjClass::TYPE_DECL, resolved.type, resolved.ref, true, symbols.currentLevel(), 0);
            }
        }
    }
    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitVarDeclaration(const NodePtr& node) {
    // masukkan variabel ke symbol table
    for (const auto& child : node->children) {
        if (child->label == "<var-item>") {
            vector<string> names;
            SemanticType declared;
            for (const auto& itemChild : child->children) {
                if (itemChild->label == "<identifier-list>") {
                    for (const auto& identNode : itemChild->children) {
                        string name;
                        if (isIdentLabel(identNode->label, name)) names.push_back(name);
                    }
                
                } else if (itemChild->label == "<type>") {
                    declared = visitTypeNode(itemChild);
                }
            }

            for (const auto& name : names) {
                if (lookupCurrentScope(symbols, name) >= 0) reportError("redeclared variable '" + name + "'");
                else symbols.enter(name, (int)ObjClass::VARIABLE, declared.type, declared.ref, true, symbols.currentLevel(), 0);
            }
        }
    }
    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitSubprogramDeclaration(const NodePtr& node) {
    // proses semua subprogram yang ditemukan
    for (const auto& child : node->children){
        visit(child);
    }

    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitProcedureDeclaration(const NodePtr& node) {
    // analisis procedure & scope parameternya
    string name;
    NodePtr formalParams;
    NodePtr blockNode;
    for (const auto& child : node->children) {
        if (isIdentLabel(child->label, name)) continue;
        if (child->label == "<formal-parameter-list>") formalParams = child;
        if (child->label == "<block>") blockNode = child;
    }

    SemanticType procType = basicType("void");
    if (!name.empty() && lookupCurrentScope(symbols, name) >= 0){
        reportError("redeclared procedure '" + name + "'");
    } else if (!name.empty()){
        symbols.enter(name, (int)ObjClass::PROCEDURE, procType.type, 0, true, symbols.currentLevel(), 0);
    }

    symbols.pushBlock();
    if (formalParams){
        visitFormalParameterList(formalParams);
    }

    if (blockNode){
        visitBlock(blockNode, false);
    }

    symbols.popBlock();

    annotate(node, procType);
    return procType;
}

SemanticType SemanticAnalyzer::visitFunctionDeclaration(const NodePtr& node) {
    // analisis function & tipe baliknya
    string name;
    string returnTypeName;
    NodePtr formalParams;
    NodePtr blockNode;
    bool afterColon = false;
    for (const auto& child : node->children) {
        if (isIdentLabel(child->label, name)) {
            if (afterColon && returnTypeName.empty()) returnTypeName = name;
            continue;
        }

        if (child->label == "colon") {
            afterColon = true;
            continue;
        }

        if (child->label == "<formal-parameter-list>") formalParams = child;
        if (child->label == "<block>") blockNode = child;
    }

    SemanticType returnType = returnTypeName.empty() ? basicType("void") : lookupTypeByName(returnTypeName);
    if (!name.empty() && lookupCurrentScope(symbols, name) >= 0){
        reportError("redeclared function '" + name + "'");
    } else if (!name.empty()){
        symbols.enter(name, (int)ObjClass::FUNCTION, returnType.type, returnType.ref, true, symbols.currentLevel(), 0);
    }

    symbols.pushBlock();
    if (formalParams){
        visitFormalParameterList(formalParams);
    }

    if (blockNode){
        visitBlock(blockNode, false);
    }

    symbols.popBlock();

    annotate(node, returnType);
    return returnType;
}

SemanticType SemanticAnalyzer::visitBlock(const NodePtr& node, bool pushScope) {
    // masuk/keluar scope sesuai konteks blok
    if (pushScope) symbols.pushBlock();
    for (const auto& child : node->children){
        visit(child);
    }

    if (pushScope) symbols.popBlock();
    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitCompoundStatement(const NodePtr& node) {
    // anggap new block
    return visitBlock(node, true);
}

SemanticType SemanticAnalyzer::visitStatementList(const NodePtr& node) {
    SemanticType result;
    for (const auto& child : node->children) result = visit(child);
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitAssignmentStatement(const NodePtr& node) {
    // cek lhs & rhs
    SemanticType lhs;
    SemanticType rhs;
    for (const auto& child : node->children) {
        if (child->label == "<variable>" || (child->label.rfind("ident(", 0) == 0 && lhs.type == TC_NOTYPE)) lhs = visit(child);
        if (child->label == "<expression>") rhs = visit(child);
    }

    if (!assignmentCompatible(lhs, rhs)) {
        reportError("assignment incompatible: cannot assign '" + typeName(rhs) + "' to '" + typeName(lhs) + "'");
    }

    SemanticType result = basicType("void");
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitIfStatement(const NodePtr& node) {
    if (node->children.size() > 1) {
        SemanticType condition = visit(node->children[1]);
        if (!isBoolean(condition) && condition.type != TC_NOTYPE){
            reportError("if-condition must be boolean");
        }
    }

    for (size_t i = 2; i < node->children.size(); ++i){
        visit(node->children[i]);
    }

    SemanticType result = basicType("void");
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitWhileStatement(const NodePtr& node) {
    if (node->children.size() > 1) {
        SemanticType condition = visit(node->children[1]);
        if (!isBoolean(condition) && condition.type != TC_NOTYPE){
            reportError("while-condition must be boolean");
        }
    }
    for (size_t i = 2; i < node->children.size(); ++i){
        visit(node->children[i]);
    }

    SemanticType result = basicType("void");
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitForStatement(const NodePtr& node) {
    string loopVar;
    SemanticType startType;
    SemanticType endType;
    bool sawBecomes = false;
    bool sawDirection = false;
    for (const auto& child : node->children) {
        if (isIdentLabel(child->label, loopVar) && !sawBecomes) continue;
        if (child->label == "becomes") {
            sawBecomes = true;
            continue;
        }

        if (child->label == "tosy" || child->label == "downtosy") {
            sawDirection = true;
            continue;
        }

        if (child->label == "<expression>") {
            if (!sawBecomes) startType = visit(child);
            else if (sawDirection) endType = visit(child);
            else startType = visit(child);
        } else {
            visit(child);
        }
    }
    if (!loopVar.empty()) {
        int idx = symbols.lookup(loopVar);
        if (idx < 0){
            reportError("undeclared loop variable '" + loopVar + "'");
        }
    }

    if (!isNumeric(startType) && startType.type != TC_NOTYPE){
        reportError("for start expression must be numeric");
    }

    if (!isNumeric(endType) && endType.type != TC_NOTYPE){
        reportError("for end expression must be numeric");
    }

    SemanticType result = basicType("void");
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitProcedureFunctionCall(const NodePtr& node) {
    string name;
    vector<SemanticType> args;
    for (const auto& child : node->children) {
        if (isIdentLabel(child->label, name)) continue;
        if (child->label == "<expression>") args.push_back(visit(child));
        else visit(child);
    }

    int idx = name.empty() ? -1 : symbols.lookup(name);
    if (idx < 0) reportError("undeclared procedure/function '" + name + "'");
    SemanticType result = basicType("void");

    if (idx >= 0) {
        result.type = symbols.getTab()[idx].type;
        result.name.clear(); // tampilkan tipe balik, bukan nama fungsi/prosedur
        annotate(node, result, idx);
    } else {
        annotate(node, result);
    }

    return result;
}

SemanticType SemanticAnalyzer::visitVariable(const NodePtr& node) {
    string name;
    NodePtr component;
    for (const auto& child : node->children) {
        if (isIdentLabel(child->label, name)) continue;
        if (child->label == "<component-variable>") component = child;
    }

    int idx = name.empty() ? -1 : symbols.lookup(name);
    SemanticType resolved;
    if (idx < 0) {
        reportError("undeclared identifier '" + name + "'");
    } else {
        resolved.type = symbols.getTab()[idx].type;
        resolved.name.clear(); // variabel: tunjukkan tipe, bukan nama variabel
        annotate(node, resolved, idx);
    }

    if (component) {
        resolved = visitComponentVariable(component, resolved);
        annotate(node, resolved, idx);
    }

    if (idx < 0) annotate(node, resolved);
    return resolved;
}

SemanticType SemanticAnalyzer::visitComponentVariable(const NodePtr& node, SemanticType baseType) {
    for (const auto& child : node->children) {
        if (child->label == "<index-list>") {
            visit(child);
        } else if (child->label.rfind("ident(", 0) == 0) {
            string field = extractTokenValue(child->label);
            int idx = symbols.lookup(field);
            if (idx < 0){
                reportError("undeclared field '" + field + "'");
            } else {
                SemanticType ftype;
                ftype.type = symbols.getTab()[idx].type;
                ftype.name.clear();
                annotate(child, ftype, idx);
            }
        } else {
            visit(child);
        }
    }
    annotate(node, baseType);
    return baseType;
}

SemanticType SemanticAnalyzer::visitExpression(const NodePtr& node) {
    SemanticType left = node->children.empty() ? SemanticType{} : visit(node->children[0]);
    if (node->children.size() >= 3) {
        SemanticType right = visit(node->children[2]);
        SemanticType result = basicType("boolean");
        if (!(sameType(left, right) || (isNumeric(left) && isNumeric(right)))) {
            reportError("relational expression has incompatible operands");
        }

        annotate(node, result);
        return result;
    }

    annotate(node, left);
    return left;
}

SemanticType SemanticAnalyzer::visitSimpleExpression(const NodePtr& node) {
    // ambil tipe dari term 1
    SemanticType result;
    vector<SemanticType> terms;
    for (const auto& child : node->children) {
        if (child->label == "<term>") terms.push_back(visit(child));
        else visit(child);
    }

    if (!terms.empty()) result = terms.front();
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitTerm(const NodePtr& node) {
    // ambil tipe dari factor 1
    SemanticType result;
    vector<SemanticType> factors;
    for (const auto& child : node->children) {
        if (child->label == "<factor>") factors.push_back(visit(child));
        else visit(child);
    }

    if (!factors.empty()) result = factors.front();
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitFactor(const NodePtr& node) {
    if (!node) return SemanticType{};
    for (const auto& child : node->children) {
        if (!child) continue;
        if (child->label == "<variable>") return visitVariable(child);
        if (child->label == "<procedure/function-call>") return visitProcedureFunctionCall(child);
        if (child->label == "<expression>") return visitExpression(child);
        if (child->label == "notsy") {
            SemanticType inner;
            for (const auto& other : node->children) {
                if (other.get() != child.get()) inner = visit(other);
            }

            if (!isBoolean(inner) && inner.type != TC_NOTYPE) reportError("not operand must be boolean");
            SemanticType result = basicType("boolean");
            annotate(node, result);
            return result;
        }
        if (child->label == "<constant>") return visitConstant(child);
        if (child->label.rfind("ident(", 0) == 0) return visit(child);
        if (child->label.rfind("intcon(", 0) == 0 || child->label.rfind("realcon(", 0) == 0 ||
            child->label.rfind("charcon(", 0) == 0 || child->label.rfind("string(", 0) == 0) {
            return visit(child);
        }
    }

    SemanticType result;
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitConstant(const NodePtr& node) {
    SemanticType result;
    for (const auto& child : node->children) {
        if (child->label == "plus" || child->label == "minus") continue;
        result = visit(child);
    }

    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitTypeNode(const NodePtr& node) {
    SemanticType result;
    for (const auto& child : node->children) {
        if (child->label == "<range>") result = visitRange(child);
        else if (child->label == "<enumerated>") result = visitEnumerated(child);
        else if (child->label == "<array-type>") result = visitArrayType(child);
        else if (child->label == "<record-type>") result = visitRecordType(child);
        else if (child->label.rfind("ident(", 0) == 0) {
            string name = extractTokenValue(child->label);
            result = lookupTypeByName(name);
            if (result.type == TC_NOTYPE && result.name == name) reportError("unknown type '" + name + "'");
        }
    }
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitRange(const NodePtr& node) {
    // range dipakai untuk subrange / array index
    SemanticType result = basicType("integer");
    result.name = "subrange";
    vector<SemanticType> bounds;
    for (const auto& child : node->children) {
        if (child->label == "<constant>") bounds.push_back(visitConstant(child));
        else visit(child);
    }

    if (bounds.size() == 2 && (bounds[0].type == TC_REAL || bounds[1].type == TC_REAL)) {
        reportError("range bounds cannot be real");
    }

    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitEnumerated(const NodePtr& node) {
    for (const auto& child : node->children){
        visit(child);
    }

    SemanticType result = basicType("integer");
    result.name = "enumerated";
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitArrayType(const NodePtr& node) {
    // analisis tipe array dan elemennya
    SemanticType indexType;
    SemanticType elementType;
    for (const auto& child : node->children) {
        if (child->label == "<range>") indexType = visitRange(child);
        else if (child->label == "<type>") elementType = visitTypeNode(child);
        else if (child->label.rfind("ident(", 0) == 0) {
            string name = extractTokenValue(child->label);
            if (indexType.type == TC_NOTYPE && indexType.name.empty()) indexType = lookupTypeByName(name);
            else elementType = lookupTypeByName(name);
        }
    }

    if (indexType.type == TC_REAL) reportError("array index type cannot be real");
    SemanticType result = basicType("array");
    result.type = TC_ARRAY;
    result.ref = 0;
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitRecordType(const NodePtr& node) {
    for (const auto& child : node->children){
        visit(child);
    }

    SemanticType result = basicType("record");
    result.type = TC_RECORD;
    result.anonymous = true;
    annotate(node, result);
    return result;
}

SemanticType SemanticAnalyzer::visitFormalParameterList(const NodePtr& node) {
    for (const auto& child : node->children) {
        if (child->label == "<parameter-group>") visitParameterGroup(child);
        else visit(child);
    }

    annotate(node, SemanticType{});
    return SemanticType{};
}

SemanticType SemanticAnalyzer::visitParameterGroup(const NodePtr& node) {
    // 1 grup param deengan tipe sama
    vector<string> names;
    SemanticType paramType;
    for (const auto& child : node->children) {
        if (child->label == "<identifier-list>") {
            for (const auto& identNode : child->children) {
                string name;
                if (isIdentLabel(identNode->label, name)) names.push_back(name);
            }
        } else if (child->label == "<array-type>") {
            paramType = visitArrayType(child);
        } else if (child->label == "<type>") {
            paramType = visitTypeNode(child);
        } else if (child->label.rfind("ident(", 0) == 0) {
            paramType = lookupTypeByName(extractTokenValue(child->label));
        }
    }

    for (const auto& name : names) {
        if (lookupCurrentScope(symbols, name) >= 0) reportError("redeclared parameter '" + name + "'");
        else symbols.enter(name, (int)ObjClass::VARIABLE, paramType.type, paramType.ref, true, symbols.currentLevel(), 0);
    }
    
    annotate(node, paramType);
    return paramType;
}

SemanticType SemanticAnalyzer::visitIdentifierList(const NodePtr& node, const SemanticType& type, const string& obj) {
    // helper untuk daftar identifier
    (void)type;
    (void)obj;
    for (const auto& child : node->children) visit(child);
    annotate(node, SemanticType{});
    return SemanticType{};
}

void SemanticAnalyzer::printDecoratedTree(const NodePtr& root, ostream& out) const {
    // tampilkan parse tree dengan anotasi semantic
    printDecoratedTreeImpl(root, out, "", true, true);
}

string SemanticAnalyzer::decoratedLabel(const NodePtr& node) const {
    if (!node) return "";
    string label = node->label;
    vector<string> parts;
    if (!node->sem_type.empty() && node->sem_type != "unknown") parts.push_back("type=" + node->sem_type);
    if (node->tab_index >= 0) parts.push_back("tab=" + to_string(node->tab_index));
    if (node->lev >= 0) parts.push_back("lev=" + to_string(node->lev));
    if (node->block_index >= 0) parts.push_back("blk=" + to_string(node->block_index));
    if (!parts.empty()) {
        label += " {";
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i) label += ", ";
            label += parts[i];
        }
        label += "}";
    }
    return label;
}

void SemanticAnalyzer::printDecoratedTreeImpl(const NodePtr& node, ostream& out, const string& prefix, bool isLast, bool isRoot) const {
    // cetak tree dengan penanda cabang
    if (!node) return;
    if (isRoot) out << decoratedLabel(node) << "\n";
    else out << prefix << (isLast ? "└── " : "├── ") << decoratedLabel(node) << "\n";

    string childPrefix = isRoot ? "" : prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < node->children.size(); ++i) {
        printDecoratedTreeImpl(node->children[i], out, childPrefix, i + 1 == node->children.size(), false);
    }
}

void SemanticAnalyzer::printTabDump(ostream& out) const {
    // tampilkan isi tab table
    symbols.dumpTab(out);
}

void SemanticAnalyzer::printBTabDump(ostream& out) const {
    // tampilkan isi block table
    symbols.dumpBtab(out);
}

void SemanticAnalyzer::printATabDump(ostream& out) const {
    // tampilkan isi address table
    symbols.dumpAtab(out);
}