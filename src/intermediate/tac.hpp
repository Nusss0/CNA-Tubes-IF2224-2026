#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "../semantic/ast_node.hpp"

struct TacInstr {
    std::string op;
    std::vector<std::string> args;
    std::string label; // empty if none
    int target = -1; // resolved target (filled after resolution)
};

class TacGenerator {
public:
    TacGenerator();

    // generate TAC from a (decorated) AST root
    void generate(const AstNodePtr& root);

    // print resolved TAC to output (canonical "<line> <OP> <lev> <operand>" format)
    void print(std::ostream& out) const;

    // expose the raw instruction list (for TacAdapter -> interpreter)
    const std::vector<TacInstr>& instructions() const { return code; }

private:
    std::vector<TacInstr> code;
    int labelCounter = 0;
    int nextAddr = 3;                                  // 0=static link, 1=dynamic link, 2=return addr
    std::unordered_map<std::string,int> addrMap;       // lowercased var name -> memory address
    std::unordered_map<std::string,int> labelMap;      // label -> 1-based instr line

    // helpers
    std::string newLabel();
    void emit(const std::string& op, const std::vector<std::string>& args = {}, const std::string& label = "");
    void emitLabel(const std::string& labelName);

    // address mapping
    void collectDecls(const AstNodePtr& program);
    int addrOf(const AstNodePtr& varNode) const;       // -1 if unknown
    static std::string lower(const std::string& s);
    static int oprCode(const std::string& op);         // operator string -> OPR opcode (-1 if none)

    // node visitors
    void genNode(const AstNodePtr& node);
    void genExpr(const AstNodePtr& node);              // emits; result left on top of stack
    void genAssign(const AstNodePtr& node);
    void genWrite(const AstNodePtr& node, int oprNo);  // write=13 / writeln=14
    void genIf(const AstNodePtr& node);
    void genWhile(const AstNodePtr& node);
    void genCompound(const AstNodePtr& node);

    // resolve labels to instruction numbers
    void resolveLabels();
};
