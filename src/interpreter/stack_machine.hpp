#pragma once

#include "runtime_context.hpp"
#include "runtime_value.hpp"
#include "runtime_instruction.hpp"
#include <string>
#include <vector>
#include <cstddef>

class StackMachine {
public:
    StackMachine();

    void loadProgram(const std::vector<RuntimeInstruction>& instrs);

    void run();

    void step();

    std::string getOutput() const;
    bool isHalted() const;

    const RuntimeContext& ctx() const { return context; }

private:
    RuntimeContext context;

    // stack primitive helpers
    void push(const RuntimeValue& v);
    RuntimeValue pop();
    RuntimeValue& top();
    bool hasValues(size_t n) const;

    // execute one instruction (called by step)
    void execute(const RuntimeInstruction& instr);

    // instruction handlers
    void execINT(const RuntimeInstruction& instr);
    void execLIT(const RuntimeInstruction& instr);
    void execLOD(const RuntimeInstruction& instr);
    void execSTO(const RuntimeInstruction& instr);
    void execCAL(const RuntimeInstruction& instr);
    void execJMP(const RuntimeInstruction& instr);
    void execJPC(const RuntimeInstruction& instr);
    void execOPR(const RuntimeInstruction& instr);
    void execRET(const RuntimeInstruction& instr);

    // OPR sub‑handlers
    void execOPR_neg();
    void execOPR_add();
    void execOPR_sub();
    void execOPR_mul();
    void execOPR_div();
    void execOPR_mod();
    void execOPR_eql();
    void execOPR_neq();
    void execOPR_lss();
    void execOPR_geq();
    void execOPR_gtr();
    void execOPR_leq();
    void execOPR_wrt();
    void execOPR_wrtln();

    // helpers for arithmetic operand resolution
    static long long   asInteger(const RuntimeValue& v);
    static double      asReal(const RuntimeValue& v);
    static bool        isNumber(const RuntimeValue& v);
};
