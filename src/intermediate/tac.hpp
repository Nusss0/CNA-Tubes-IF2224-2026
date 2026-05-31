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

    // generate TAC
    void generate(const AstNodePtr& root);

    // print resolved TAC to output
    void print(std::ostream& out) const;

private:
    std::vector<TacInstr> code;
    int tempCounter = 0;
    int labelCounter = 0;
    std::unordered_map<std::string,int> labelMap; // label -> instr index

    // helpers
    std::string newTemp();
    std::string newLabel();
    void emit(const std::string& op, const std::vector<std::string>& args = {}, const std::string& label = "");
    void emitLabel(const std::string& labelName);

    // node visitors
    void genNode(const AstNodePtr& node);
    std::string genExpr(const AstNodePtr& node);
    void genAssign(const AstNodePtr& node);
    void genIf(const AstNodePtr& node);
    void genWhile(const AstNodePtr& node);
    void genCompound(const AstNodePtr& node);

    // resolve labels to instruction numbers
    void resolveLabels();
};
