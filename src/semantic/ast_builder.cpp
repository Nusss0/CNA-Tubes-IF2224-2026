#include "ast_builder.hpp"

// utils

static bool startsWith(const string& s, const string& p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

// extract inner
static string extractInside(const string& label) {
    auto lp = label.find('(');
    auto rp = label.rfind(')');
    if (lp == string::npos || rp == string::npos || rp <= lp + 1) return "";
    string inner = label.substr(lp + 1, rp - lp - 1);
    //buang single-quote utk charcon/string ('Hello' -> Hello)
    if (inner.size() >= 2 && inner.front() == '\'' && inner.back() == '\'') {
        inner = inner.substr(1, inner.size() - 2);
    }
    return inner;
}

// leaf type
static string leafType(const string& label) {
    auto lp = label.find('(');
    if (lp == string::npos) return label;
    return label.substr(0, lp);
}

// find child
static NodePtr findChild(const NodePtr& node, const string& label) {
    if (!node) return nullptr;
    for (auto& c : node->children) {
        if (c->label == label) return c;
    }
    return nullptr;
}

// op map
static string opSymbol(const string& tokType) {
    if (tokType == "plus")   return "+";
    if (tokType == "minus")  return "-";
    if (tokType == "times")  return "*";
    if (tokType == "rdiv")   return "/";
    if (tokType == "idiv")   return "div";
    if (tokType == "imod")   return "mod";
    if (tokType == "andsy")  return "and";
    if (tokType == "orsy")   return "or";
    if (tokType == "notsy")  return "not";
    if (tokType == "eql")    return "=";
    if (tokType == "neq")    return "<>";
    if (tokType == "lss")    return "<";
    if (tokType == "leq")    return "<=";
    if (tokType == "gtr")    return ">";
    if (tokType == "geq")    return ">=";
    return tokType;
}

static bool isOpToken(const string& lbl) {
    static const vector<string> ops = {
        "plus","minus","times","rdiv","idiv","imod","andsy","orsy",
        "eql","neq","lss","leq","gtr","geq"
    };
    for (auto& o : ops) if (lbl == o) return true;
    return false;
}

// entry

AstNodePtr AstBuilder::build(const NodePtr& parseRoot) {
    errors.clear();
    if (!parseRoot) {
        reportError("Parse tree is null");
        return nullptr;
    }
    return buildProgram(parseRoot);
}

void AstBuilder::reportError(const string& msg) {
    errors.push_back(msg);
    cerr << "[AST ERROR] " << msg << " !\n";
}

// program & decls

AstNodePtr AstBuilder::buildProgram(const NodePtr& node) {
    auto prog = makeAst(AstKind::Program);
    // program name
    auto header = findChild(node, "<program-header>");
    if (header) {
        for (auto& c : header->children) {
            if (startsWith(c->label, "ident(")) {
                prog->value = extractInside(c->label);
                break;
            }
        }
    }
    // declarations
    auto decls = findChild(node, "<declaration-part>");
    if (decls) {
        auto declList = buildDeclarationPart(decls);
        if (declList) addAstChild(prog, declList);
    }
    // body
    auto compound = findChild(node, "<compound-statement>");
    if (compound) {
        auto body = buildCompoundStatement(compound);
        if (body) addAstChild(prog, body);
    }
    return prog;
}

AstNodePtr AstBuilder::buildDeclarationPart(const NodePtr& node) {
    auto block = makeAst(AstKind::Declarations);
    for (auto& c : node->children) {
        AstNodePtr v;
        if (c->label == "<var-declaration>")        v = buildVarDeclaration(c);
        else if (c->label == "<const-declaration>") v = buildConstDeclaration(c);
        else if (c->label == "<type-declaration>")  v = buildTypeDeclaration(c);
        else if (c->label == "<subprogram-declaration>") v = buildSubprogram(c);
        if (!v) continue;
        // flatten block
        if (v->kind == AstKind::Block) {
            for (auto& ch : v->children) addAstChild(block, ch);
        } else {
            addAstChild(block, v);
        }
    }
    return block;
}

AstNodePtr AstBuilder::buildVarDeclaration(const NodePtr& node) {
    // var decl
    auto group = makeAst(AstKind::Block);
    for (auto& item : node->children) {
        if (item->label != "<var-item>") continue;
        auto identList = findChild(item, "<identifier-list>");
        auto typeNode  = findChild(item, "<type>");
        AstNodePtr typeAst;
        if (typeNode) typeAst = buildType(typeNode);
        if (!identList) continue;
        for (auto& id : identList->children) {
            if (!startsWith(id->label, "ident(")) continue;
            auto vd = makeAst(AstKind::VarDecl, extractInside(id->label));
            if (typeAst) {
                // share type
                addAstChild(vd, typeAst);
            }
            addAstChild(group, vd);
        }
    }
    // block container
    return group;
}

AstNodePtr AstBuilder::buildConstDeclaration(const NodePtr& node) {
    auto group = makeAst(AstKind::Block);
    for (auto& item : node->children) {
        if (item->label != "<const-item>") continue;
        // const decl
        string name;
        AstNodePtr val;
        for (auto& c : item->children) {
            if (startsWith(c->label, "ident(") && name.empty()) {
                name = extractInside(c->label);
            } else if (c->label == "<constant>") {
                val = buildExpression(c);
                if (!val && !c->children.empty()) {
                    //fallback: ambil literal langsung
                    val = buildFactor(c);
                }
            }
        }
        auto cd = makeAst(AstKind::ConstDecl, name);
        if (val) addAstChild(cd, val);
        addAstChild(group, cd);
    }
    return group;
}

AstNodePtr AstBuilder::buildTypeDeclaration(const NodePtr& node) {
    auto group = makeAst(AstKind::Block);
    for (auto& item : node->children) {
        if (item->label != "<type-item>") continue;
        string name;
        AstNodePtr typ;
        for (auto& c : item->children) {
            if (startsWith(c->label, "ident(") && name.empty()) {
                name = extractInside(c->label);
            } else if (c->label == "<type>") {
                typ = buildType(c);
            }
        }
        auto td = makeAst(AstKind::TypeDecl, name);
        if (typ) addAstChild(td, typ);
        addAstChild(group, td);
    }
    return group;
}

AstNodePtr AstBuilder::buildType(const NodePtr& node) {
    // type
    if (!node || node->children.empty()) return makeAst(AstKind::TypeRef, "unknown");
    auto& c = node->children[0];
    if (startsWith(c->label, "ident(")) {
        return makeAst(AstKind::TypeRef, extractInside(c->label));
    }
    if (c->label == "<array-type>") {
        auto arr = makeAst(AstKind::ArrayType);
        // array type
        for (auto& ch : c->children) {
            if (ch->label == "<range>") {
                auto r = makeAst(AstKind::RangeType);
                for (auto& rc : ch->children) {
                    if (rc->label == "<constant>") {
                        auto v = buildFactor(rc);
                        if (v) addAstChild(r, v);
                    }
                }
                addAstChild(arr, r);
            } else if (startsWith(ch->label, "ident(")) {
                addAstChild(arr, makeAst(AstKind::TypeRef, extractInside(ch->label)));
            } else if (ch->label == "<type>") {
                addAstChild(arr, buildType(ch));
            }
        }
        return arr;
    }
    if (c->label == "<record-type>") {
        auto rec = makeAst(AstKind::RecordType);
        // record type
        auto fl = findChild(c, "<field-list>");
        if (fl) {
            for (auto& fp : fl->children) {
                if (fp->label != "<field-part>") continue;
                // field part
                auto il = findChild(fp, "<identifier-list>");
                auto tn = findChild(fp, "<type>");
                AstNodePtr ftype = tn ? buildType(tn) : nullptr;
                if (!il) continue;
                for (auto& id : il->children) {
                    if (!startsWith(id->label, "ident(")) continue;
                    auto field = makeAst(AstKind::VarDecl, extractInside(id->label));
                    if (ftype) addAstChild(field, ftype);
                    addAstChild(rec, field);
                }
            }
        }
        return rec;
    }
    if (c->label == "<enumerated>") {
        auto en = makeAst(AstKind::EnumType);
        for (auto& ch : c->children) {
            if (startsWith(ch->label, "ident(")) {
                addAstChild(en, makeAst(AstKind::Var, extractInside(ch->label)));
            }
        }
        return en;
    }
    if (c->label == "<range>") {
        auto r = makeAst(AstKind::RangeType);
        for (auto& ch : c->children) {
            if (ch->label == "<constant>") {
                auto v = buildFactor(ch);
                if (v) addAstChild(r, v);
            }
        }
        return r;
    }
    return makeAst(AstKind::TypeRef, leafType(c->label));
}

AstNodePtr AstBuilder::buildSubprogram(const NodePtr& node) {
    auto sd = makeAst(AstKind::SubprogramDecl);
    // subprogram
    NodePtr inner = node;
    for (auto& c : node->children) {
        if (c->label == "<procedure-declaration>" || c->label == "<function-declaration>") {
            inner = c;
            sd->extra = (c->label == "<procedure-declaration>") ? "procedure" : "function";
            break;
        }
    }
    // name
    for (auto& c : inner->children) {
        if (startsWith(c->label, "ident(") && sd->value.empty()) {
            sd->value = extractInside(c->label);
            break;
        }
    }
    // body
    for (auto& c : inner->children) {
        if (c->label == "<block>") {
            for (auto& bc : c->children) {
                if (bc->label == "<compound-statement>") {
                    addAstChild(sd, buildCompoundStatement(bc));
                }
            }
        }
    }
    return sd;
}

// statements

AstNodePtr AstBuilder::buildCompoundStatement(const NodePtr& node) {
    auto comp = makeAst(AstKind::Compound);
    auto stmtList = findChild(node, "<statement-list>");
    if (!stmtList) return comp;
    for (auto& s : stmtList->children) {
        // skip semicolon
        if (s->label == "semicolon") continue;
        auto a = buildStatement(s);
        if (a) addAstChild(comp, a);
    }
    return comp;
}

AstNodePtr AstBuilder::buildStatement(const NodePtr& node) {
    if (!node) return nullptr;
    const string& l = node->label;
    if (l == "<assignment-statement>")     return buildAssignment(node);
    if (l == "<if-statement>")             return buildIfStatement(node);
    if (l == "<while-statement>")          return buildWhileStatement(node);
    if (l == "<for-statement>")            return buildForStatement(node);
    if (l == "<repeat-statement>")         return buildRepeatStatement(node);
    if (l == "<case-statement>")           return buildCaseStatement(node);
    if (l == "<compound-statement>")       return buildCompoundStatement(node);
    if (l == "<procedure/function-call>")  return buildProcFuncCall(node);
    if (l == "<empty-statement>")          return makeAst(AstKind::Empty);
    //fallback
    return nullptr;
}

AstNodePtr AstBuilder::buildAssignment(const NodePtr& node) {
    auto asg = makeAst(AstKind::Assign);
    // assign children
    AstNodePtr target, value;
    for (auto& c : node->children) {
        if (c->label == "becomes") continue;
        if (!target) {
            if (startsWith(c->label, "ident(")) {
                target = makeAst(AstKind::Var, extractInside(c->label));
            } else if (c->label == "<variable>") {
                target = buildVariable(c);
            }
        } else if (c->label == "<expression>") {
            value = buildExpression(c);
        }
    }
    if (target) addAstChild(asg, target);
    if (value)  addAstChild(asg, value);
    return asg;
}

AstNodePtr AstBuilder::buildVariable(const NodePtr& node) {
    // variable
    AstNodePtr current;
    for (auto& c : node->children) {
        if (startsWith(c->label, "ident(") && !current) {
            current = makeAst(AstKind::Var, extractInside(c->label));
        } else if (c->label == "<component-variable>") {
            // component
            bool isIndex = false;
            string fieldName;
            AstNodePtr indexExpr;
            for (auto& cc : c->children) {
                if (cc->label == "lbrack") isIndex = true;
                else if (cc->label == "<index-list>") {
                    // first child
                    if (!cc->children.empty()) {
                        auto& first = cc->children[0];
                        indexExpr = buildFactor(first);
                    }
                } else if (cc->label == "period") {
                    isIndex = false;
                } else if (startsWith(cc->label, "ident(") && !isIndex) {
                    fieldName = extractInside(cc->label);
                }
            }
            if (isIndex) {
                auto ix = makeAst(AstKind::IndexAccess);
                if (current) addAstChild(ix, current);
                if (indexExpr) addAstChild(ix, indexExpr);
                current = ix;
            } else {
                auto fa = makeAst(AstKind::FieldAccess, fieldName);
                if (current) addAstChild(fa, current);
                current = fa;
            }
        }
    }
    return current;
}

// expressions

AstNodePtr AstBuilder::buildExpression(const NodePtr& node) {
    // expression
    if (!node || node->children.empty()) return nullptr;
    AstNodePtr left = buildSimpleExpression(node->children[0]);
    // relop
    for (size_t i = 1; i + 1 < node->children.size(); i++) {
        auto& opNode = node->children[i];
        if (!isOpToken(opNode->label)) continue;
        auto right = buildSimpleExpression(node->children[i + 1]);
        auto bin = makeAst(AstKind::BinOp, opSymbol(opNode->label));
        if (left)  addAstChild(bin, left);
        if (right) addAstChild(bin, right);
        left = bin;
        i++;
    }
    return left;
}

AstNodePtr AstBuilder::buildSimpleExpression(const NodePtr& node) {
    // simple expr
    if (!node || node->children.empty()) return nullptr;
    size_t i = 0;
    bool unaryNeg = false, unaryPos = false;
    if (node->children[0]->label == "minus") { unaryNeg = true; i++; }
    else if (node->children[0]->label == "plus") { unaryPos = true; i++; }

    AstNodePtr left;
    if (i < node->children.size() && node->children[i]->label == "<term>") {
        left = buildTerm(node->children[i]);
        i++;
    }
    if (unaryNeg && left) {
        auto u = makeAst(AstKind::UnaryOp, "-");
        addAstChild(u, left);
        left = u;
    } else if (unaryPos && left) {
        // unary +
        auto u = makeAst(AstKind::UnaryOp, "+");
        addAstChild(u, left);
        left = u;
    }
    while (i + 1 < node->children.size()) {
        auto& opNode = node->children[i];
        if (!isOpToken(opNode->label)) { i++; continue; }
        auto right = buildTerm(node->children[i + 1]);
        auto bin = makeAst(AstKind::BinOp, opSymbol(opNode->label));
        if (left)  addAstChild(bin, left);
        if (right) addAstChild(bin, right);
        left = bin;
        i += 2;
    }
    return left;
}

AstNodePtr AstBuilder::buildTerm(const NodePtr& node) {
    // term
    if (!node || node->children.empty()) return nullptr;
    AstNodePtr left = buildFactor(node->children[0]);
    size_t i = 1;
    while (i + 1 < node->children.size()) {
        auto& opNode = node->children[i];
        if (!isOpToken(opNode->label)) { i++; continue; }
        auto right = buildFactor(node->children[i + 1]);
        auto bin = makeAst(AstKind::BinOp, opSymbol(opNode->label));
        if (left)  addAstChild(bin, left);
        if (right) addAstChild(bin, right);
        left = bin;
        i += 2;
    }
    return left;
}

AstNodePtr AstBuilder::buildFactor(const NodePtr& node) {
    // factor
    if (!node) return nullptr;
    if (node->label == "<constant>") {
        //<constant> = [plus|minus] (ident|intcon|realcon) | charcon | string
        bool neg = false;
        AstNodePtr inner;
        for (auto& c : node->children) {
            if (c->label == "minus") { neg = true; continue; }
            if (c->label == "plus") continue;
            inner = buildFactor(c);
            break;
        }
        if (neg && inner) {
            auto u = makeAst(AstKind::UnaryOp, "-");
            addAstChild(u, inner);
            return u;
        }
        return inner;
    }
    // leaf
    if (node->children.empty()) {
        const string& lbl = node->label;
        if (startsWith(lbl, "intcon("))   return makeAst(AstKind::Number, extractInside(lbl));
        if (startsWith(lbl, "realcon("))  return makeAst(AstKind::RealNumber, extractInside(lbl));
        if (startsWith(lbl, "charcon("))  return makeAst(AstKind::CharLit, extractInside(lbl));
        if (startsWith(lbl, "string("))   return makeAst(AstKind::StringLit, extractInside(lbl));
        if (startsWith(lbl, "ident(")) {
            string nm = extractInside(lbl);
            if (nm == "true" || nm == "True")  return makeAst(AstKind::BoolLit, "true");
            if (nm == "false" || nm == "False") return makeAst(AstKind::BoolLit, "false");
            return makeAst(AstKind::Var, nm);
        }
        return nullptr;
    }
    // wrapper
    AstNodePtr result;
    bool sawNot = false;
    for (size_t i = 0; i < node->children.size(); i++) {
        auto& c = node->children[i];
        const string& lbl = c->label;
        if (lbl == "lparent" || lbl == "rparent") continue;
        if (lbl == "notsy") { sawNot = true; continue; }
        if (lbl == "<expression>")            { result = buildExpression(c); break; }
        if (lbl == "<procedure/function-call>"){ result = buildProcFuncCall(c); break; }
        if (lbl == "<variable>")              { result = buildVariable(c); break; }
        if (lbl == "<factor>")                { result = buildFactor(c); break; }
        //leaf
        if (startsWith(lbl, "intcon("))  { result = makeAst(AstKind::Number, extractInside(lbl)); break; }
        if (startsWith(lbl, "realcon(")) { result = makeAst(AstKind::RealNumber, extractInside(lbl)); break; }
        if (startsWith(lbl, "charcon(")) { result = makeAst(AstKind::CharLit, extractInside(lbl)); break; }
        if (startsWith(lbl, "string("))  { result = makeAst(AstKind::StringLit, extractInside(lbl)); break; }
        if (startsWith(lbl, "ident(")) {
            string nm = extractInside(lbl);
            if (nm == "true" || nm == "True")   { result = makeAst(AstKind::BoolLit, "true"); break; }
            if (nm == "false" || nm == "False") { result = makeAst(AstKind::BoolLit, "false"); break; }
            result = makeAst(AstKind::Var, nm);
            break;
        }
    }
    if (sawNot && result) {
        auto u = makeAst(AstKind::UnaryOp, "not");
        addAstChild(u, result);
        result = u;
    }
    return result;
}

// control flow

AstNodePtr AstBuilder::buildIfStatement(const NodePtr& node) {
    // if
    auto ifn = makeAst(AstKind::If);
    AstNodePtr cond, thenS, elseS;
    bool sawElse = false;
    for (auto& c : node->children) {
        if (c->label == "<expression>" && !cond) cond = buildExpression(c);
        else if (c->label == "elsesy") sawElse = true;
        else if (c->label == "ifsy" || c->label == "thensy") continue;
        else {
            //statement node (bisa berbagai macam)
            auto s = buildStatement(c);
            if (!s) continue;
            if (!thenS) thenS = s;
            else if (sawElse && !elseS) elseS = s;
        }
    }
    if (cond)  addAstChild(ifn, cond);
    if (thenS) addAstChild(ifn, thenS);
    if (elseS) addAstChild(ifn, elseS);
    return ifn;
}

AstNodePtr AstBuilder::buildWhileStatement(const NodePtr& node) {
    // while
    auto w = makeAst(AstKind::While);
    AstNodePtr cond, body;
    for (auto& c : node->children) {
        if (c->label == "<expression>") cond = buildExpression(c);
        else if (c->label == "<compound-statement>") body = buildCompoundStatement(c);
    }
    if (cond) addAstChild(w, cond);
    if (body) addAstChild(w, body);
    return w;
}

AstNodePtr AstBuilder::buildForStatement(const NodePtr& node) {
    // for
    auto f = makeAst(AstKind::For);
    AstNodePtr startE, endE, body;
    string ctrl, dir = "to";
    bool sawBecomes = false, sawTo = false;
    for (auto& c : node->children) {
        const string& l = c->label;
        if (startsWith(l, "ident(") && ctrl.empty()) ctrl = extractInside(l);
        else if (l == "becomes") sawBecomes = true;
        else if (l == "tosy") { dir = "to"; sawTo = true; }
        else if (l == "downtosy") { dir = "downto"; sawTo = true; }
        else if (l == "<expression>") {
            if (sawBecomes && !sawTo && !startE) startE = buildExpression(c);
            else if (sawTo && !endE) endE = buildExpression(c);
        }
        else if (l == "<compound-statement>") body = buildCompoundStatement(c);
    }
    f->value = ctrl;
    f->extra = dir;
    if (startE) addAstChild(f, startE);
    if (endE)   addAstChild(f, endE);
    if (body)   addAstChild(f, body);
    return f;
}

AstNodePtr AstBuilder::buildRepeatStatement(const NodePtr& node) {
    // repeat
    auto r = makeAst(AstKind::Repeat);
    AstNodePtr body = makeAst(AstKind::Compound);
    AstNodePtr cond;
    auto sl = findChild(node, "<statement-list>");
    if (sl) {
        for (auto& s : sl->children) {
            if (s->label == "semicolon") continue;
            auto a = buildStatement(s);
            if (a) addAstChild(body, a);
        }
    }
    auto ex = findChild(node, "<expression>");
    if (ex) cond = buildExpression(ex);
    addAstChild(r, body);
    if (cond) addAstChild(r, cond);
    return r;
}

AstNodePtr AstBuilder::buildCaseStatement(const NodePtr& node) {
    // case
    auto cs = makeAst(AstKind::Case);
    auto ex = findChild(node, "<expression>");
    if (ex) {
        auto e = buildExpression(ex);
        if (e) addAstChild(cs, e);
    }
    // case block
    for (auto& c : node->children) {
        if (c->label != "<case-block>") continue;
        auto branch = makeAst(AstKind::Block);
        for (auto& cb : c->children) {
            if (cb->label == "<constant>") {
                auto v = buildFactor(cb);
                if (v) addAstChild(branch, v);
            } else if (cb->label == "colon" || cb->label == "comma" || cb->label == "semicolon") {
                continue;
            } else if (cb->label == "<case-block>") {
                continue; //nested handled di iterasi luar
            } else {
                //statement
                auto s = buildStatement(cb);
                if (s) addAstChild(branch, s);
            }
        }
        addAstChild(cs, branch);
    }
    return cs;
}

AstNodePtr AstBuilder::buildProcFuncCall(const NodePtr& node) {
    // call
    auto call = makeAst(AstKind::ProcCall);
    for (auto& c : node->children) {
        const string& l = c->label;
        if (startsWith(l, "ident(") && call->value.empty()) {
            call->value = extractInside(l);
        } else if (l == "<parameter-list>") {
            for (auto& p : c->children) {
                if (p->label == "<expression>") {
                    auto a = buildExpression(p);
                    if (a) addAstChild(call, a);
                }
            }
        } else if (l == "<expression>") {
            auto a = buildExpression(c);
            if (a) addAstChild(call, a);
        }
    }
    return call;
}
