#include "stack_machine.hpp"
#include "runtime_error_hook.hpp"
#include <stdexcept>
#include <cstdlib>

using namespace std;

// ---------------------------------------------------------------------------
// construction
// ---------------------------------------------------------------------------

StackMachine::StackMachine() {}

// ---------------------------------------------------------------------------
// program loading
// ---------------------------------------------------------------------------

void StackMachine::loadProgram(const vector<RuntimeInstruction>& instrs) {
    context.reset();
    context.program = instrs;
}

// ---------------------------------------------------------------------------
// public execution
// ---------------------------------------------------------------------------

void StackMachine::run() {
    while (!context.halted && context.ip < context.program.size()) {
        step();
    }
}

void StackMachine::step() {
    if (context.halted) return;
    if (context.ip >= context.program.size()) {
        context.halted = true;
        return;
    }
    execute(context.program[context.ip]);
}

string StackMachine::getOutput() const {
    return context.output.str();
}

bool StackMachine::isHalted() const {
    return context.halted;
}

// ---------------------------------------------------------------------------
// stack helpers
// ---------------------------------------------------------------------------

void StackMachine::push(const RuntimeValue& v) {
    context.stack.push_back(v);
}

RuntimeValue StackMachine::pop() {
    if (context.stack.empty()) {
        RuntimeErrorHook::stackUnderflow("pop", 0, 1);
        return RuntimeValue::none();  // unreachable
    }
    auto v = context.stack.back();
    context.stack.pop_back();
    return v;
}

RuntimeValue& StackMachine::top() {
    if (context.stack.empty()) {
        RuntimeErrorHook::stackUnderflow("peek", 0, 1);
    }
    return context.stack.back();
}

bool StackMachine::hasValues(size_t n) const {
    return context.stack.size() >= n;
}

// ---------------------------------------------------------------------------
// execute dispatch
// ---------------------------------------------------------------------------

void StackMachine::execute(const RuntimeInstruction& instr) {
    const string& op = instr.op;

    if (op == "INT")       { execINT(instr);  return; }
    if (op == "LIT")       { execLIT(instr);  return; }
    if (op == "LOD")       { execLOD(instr);  return; }
    if (op == "STO")       { execSTO(instr);  return; }
    if (op == "CAL")       { execCAL(instr);  return; }
    if (op == "JMP")       { execJMP(instr);  return; }
    if (op == "JPC")       { execJPC(instr);  return; }
    if (op == "OPR")       { execOPR(instr);  return; }
    if (op == "RET")       { execRET(instr);  return; }
    if (op == "LABEL")     { context.ip++;    return; }
    if (op == "ASSIGN" || op == "BINOP" || op == "UNOP" || op == "CALL") {
        // PLACEHOLDER(Michael/Daniel): legacy instruction formats —
        // interpreter skips them for now; they will be replaced by proper
        // LIT/LOD/STO/OPR/JMP/JPC after the intermediate generator is finalised.
        context.ip++;
        return;
    }
    RuntimeErrorHook::invalidOperand(op, "unknown instruction");
}

// ---------------------------------------------------------------------------
// INT  – initiate memory:  INT <lev> <size>
// ---------------------------------------------------------------------------
void StackMachine::execINT(const RuntimeInstruction& instr) {
    if (instr.args.size() < 2) {
        RuntimeErrorHook::invalidOperand("INT", "missing size argument");
    }
    int size = stoi(instr.args[1]);

    // Save previous frame info (for main: dl = -1, ra = -1)
    context.dl = context.bp;
    context.ra = -1;  // main has no return address

    size_t bpIndex = context.stack.size();

    // Push static link  (address 0)
    push(RuntimeValue::integer(context.bp));
    // Push dynamic link (address 1)
    push(RuntimeValue::integer(context.dl));
    // Push return addr  (address 2)
    push(RuntimeValue::integer(context.ra));

    // Allocate remaining variable cells (address 3 … size-1)
    int varCells = size - 3;
    for (int i = 0; i < varCells; ++i) {
        push(RuntimeValue::none());
    }

    context.bp = static_cast<int>(bpIndex);
    context.ip++;
}

// ---------------------------------------------------------------------------
// LIT  – load literal:  LIT <lev> <value>
// ---------------------------------------------------------------------------
void StackMachine::execLIT(const RuntimeInstruction& instr) {
    if (instr.args.size() < 2) {
        RuntimeErrorHook::invalidOperand("LIT", "missing value argument");
    }
    const string& raw = instr.args[1];

    RuntimeValue val;

    // heuristic: try integer first, then real, then bool, then string
    if (raw == "true") {
        val = RuntimeValue::boolean(true);
    } else if (raw == "false") {
        val = RuntimeValue::boolean(false);
    } else {
        char* end = nullptr;
        long long llv = strtoll(raw.c_str(), &end, 10);
        if (end && *end == '\0') {
            val = RuntimeValue::integer(llv);
        } else {
            double dv = strtod(raw.c_str(), &end);
            if (end && *end == '\0') {
                val = RuntimeValue::real(dv);
            } else {
                // keep as string / char token (identifier name, etc.)
                val = RuntimeValue::str(raw);
            }
        }
    }

    push(val);
    context.ip++;
}

// ---------------------------------------------------------------------------
// LOD  – load from address:  LOD <lev> <addr>
// ---------------------------------------------------------------------------
void StackMachine::execLOD(const RuntimeInstruction& instr) {
    if (instr.args.size() < 2) {
        RuntimeErrorHook::invalidOperand("LOD", "missing address argument");
    }
    int addr = stoi(instr.args[1]);
    int absIdx = context.bp + addr;

    if (absIdx < 0 || static_cast<size_t>(absIdx) >= context.stack.size()) {
        RuntimeErrorHook::invalidMemoryAccess(addr, context.bp,
                                               static_cast<int>(context.stack.size()));
        return;  // unreachable
    }

    push(context.stack[static_cast<size_t>(absIdx)]);
    context.ip++;
}

// ---------------------------------------------------------------------------
// STO  – store to address:  STO <lev> <addr>
// ---------------------------------------------------------------------------
void StackMachine::execSTO(const RuntimeInstruction& instr) {
    if (instr.args.size() < 2) {
        RuntimeErrorHook::invalidOperand("STO", "missing address argument");
    }
    int addr = stoi(instr.args[1]);
    int absIdx = context.bp + addr;

    if (absIdx < 0 || static_cast<size_t>(absIdx) >= context.stack.size()) {
        RuntimeErrorHook::invalidMemoryAccess(addr, context.bp,
                                               static_cast<int>(context.stack.size()));
        return;
    }

    RuntimeValue val = pop();
    context.stack[static_cast<size_t>(absIdx)] = val;
    context.ip++;
}

// ---------------------------------------------------------------------------
// CAL  – call function/procedure:  CAL <lev> <name>
// ---------------------------------------------------------------------------
void StackMachine::execCAL(const RuntimeInstruction& instr) {
    // PLACEHOLDER(Michael/Daniel): full call implementation needs:
    //   - create new stack frame
    //   - push arguments (done by the caller beforehand)
    //   - set ra = ip + 1 (return address, 1‑based)
    //   - jump to target

    if (instr.target < 0) {
        RuntimeErrorHook::invalidJumpTarget(instr.target,
                                             static_cast<int>(context.program.size()));
    }

    // Save return address
    context.ra = static_cast<int>(context.ip) + 1 + 1;  // 1‑based line number

    // Jump to target (convert 1‑based target → 0‑based index)
    int tgt = instr.target - 1;
    if (tgt < 0 || static_cast<size_t>(tgt) >= context.program.size()) {
        RuntimeErrorHook::invalidJumpTarget(instr.target,
                                             static_cast<int>(context.program.size()));
    }
    context.ip = static_cast<size_t>(tgt);
}

// ---------------------------------------------------------------------------
// JMP  – unconditional jump
// ---------------------------------------------------------------------------
void StackMachine::execJMP(const RuntimeInstruction& instr) {
    int tgt = instr.target;
    if (tgt < 0) {
        // PLACEHOLDER(Daniel): label not yet resolved
        RuntimeErrorHook::invalidJumpTarget(tgt,
                                             static_cast<int>(context.program.size()));
    }
    int idx = tgt - 1;  // 1‑based → 0‑based
    if (idx < 0 || static_cast<size_t>(idx) >= context.program.size()) {
        RuntimeErrorHook::invalidJumpTarget(tgt,
                                             static_cast<int>(context.program.size()));
    }
    context.ip = static_cast<size_t>(idx);
}

// ---------------------------------------------------------------------------
// JPC  – conditional jump (jump if false)
// ---------------------------------------------------------------------------
void StackMachine::execJPC(const RuntimeInstruction& instr) {
    if (!hasValues(1)) {
        RuntimeErrorHook::stackUnderflow("JPC", context.stack.size(), 1);
    }
    RuntimeValue cond = pop();

    if (!cond.isTruthy()) {
        // condition is false → jump  (IF_FALSE convention)
        int tgt = instr.target;
        if (tgt < 0) {
            // PLACEHOLDER(Daniel): label not yet resolved
            RuntimeErrorHook::invalidJumpTarget(tgt,
                                                 static_cast<int>(context.program.size()));
        }
        int idx = tgt - 1;
        if (idx < 0 || static_cast<size_t>(idx) >= context.program.size()) {
            RuntimeErrorHook::invalidJumpTarget(tgt,
                                                 static_cast<int>(context.program.size()));
        }
        context.ip = static_cast<size_t>(idx);
    } else {
        context.ip++;
    }
}

// ---------------------------------------------------------------------------
// OPR  – operation:  OPR <lev> <opcode>
// ---------------------------------------------------------------------------
void StackMachine::execOPR(const RuntimeInstruction& instr) {
    if (instr.args.size() < 2) {
        RuntimeErrorHook::invalidOperand("OPR", "missing opcode argument");
    }
    int code = stoi(instr.args[1]);

    switch (code) {
        case 1:  execOPR_neg();   break;
        case 2:  execOPR_add();   break;
        case 3:  execOPR_sub();   break;
        case 4:  execOPR_mul();   break;
        case 5:  execOPR_div();   break;
        case 6:  execOPR_mod();   break;
        case 7:  execOPR_eql();   break;
        case 8:  execOPR_neq();   break;
        case 9:  execOPR_lss();   break;
        case 10: execOPR_geq();   break;
        case 11: execOPR_gtr();   break;
        case 12: execOPR_leq();   break;
        case 13: execOPR_wrt();   break;
        case 14: execOPR_wrtln(); break;
        default:
            RuntimeErrorHook::invalidOperand("OPR", "unknown opcode " + to_string(code));
    }
    context.ip++;
}

// ---------------------------------------------------------------------------
// OPR  sub‑handlers
// ---------------------------------------------------------------------------

void StackMachine::execOPR_neg() {
    if (!hasValues(1)) RuntimeErrorHook::stackUnderflow("NEG", context.stack.size(), 1);
    RuntimeValue a = pop();
    if (a.kind == RuntimeValue::Kind::INTEGER) {
        push(RuntimeValue::integer(-a.intVal));
    } else if (a.kind == RuntimeValue::Kind::REAL) {
        push(RuntimeValue::real(-a.realVal));
    } else {
        RuntimeErrorHook::invalidOperand("NEG", "non-numeric operand");
    }
}

void StackMachine::execOPR_add() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("ADD", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();

    bool aInt = (a.kind == RuntimeValue::Kind::INTEGER);
    bool bInt = (b.kind == RuntimeValue::Kind::INTEGER);
    bool aReal = (a.kind == RuntimeValue::Kind::REAL);
    bool bReal = (b.kind == RuntimeValue::Kind::REAL);

    if ((aInt || aReal) && (bInt || bReal)) {
        if (aReal || bReal) {
            push(RuntimeValue::real(asReal(a) + asReal(b)));
        } else {
            push(RuntimeValue::integer(asInteger(a) + asInteger(b)));
        }
    } else {
        // string concatenation or error
        if (a.kind == RuntimeValue::Kind::STRING && b.kind == RuntimeValue::Kind::STRING) {
            push(RuntimeValue::str(a.strVal + b.strVal));
        } else {
            RuntimeErrorHook::invalidOperand("ADD", "type mismatch");
        }
    }
}

void StackMachine::execOPR_sub() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("SUB", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    if (a.kind == RuntimeValue::Kind::REAL || b.kind == RuntimeValue::Kind::REAL)
        push(RuntimeValue::real(asReal(a) - asReal(b)));
    else
        push(RuntimeValue::integer(asInteger(a) - asInteger(b)));
}

void StackMachine::execOPR_mul() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("MUL", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    if (a.kind == RuntimeValue::Kind::REAL || b.kind == RuntimeValue::Kind::REAL)
        push(RuntimeValue::real(asReal(a) * asReal(b)));
    else
        push(RuntimeValue::integer(asInteger(a) * asInteger(b)));
}

void StackMachine::execOPR_div() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("DIV", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    double divisor = asReal(b);
    if (divisor == 0.0) RuntimeErrorHook::divisionByZero();
    // Arion div returns integer if both operands are integer
    if (a.kind == RuntimeValue::Kind::INTEGER && b.kind == RuntimeValue::Kind::INTEGER)
        push(RuntimeValue::integer(asInteger(a) / asInteger(b)));
    else
        push(RuntimeValue::real(asReal(a) / divisor));
}

void StackMachine::execOPR_mod() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("MOD", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    long long divisor = asInteger(b);
    if (divisor == 0) RuntimeErrorHook::divisionByZero();
    push(RuntimeValue::integer(asInteger(a) % divisor));
}

void StackMachine::execOPR_eql() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("EQL", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    push(RuntimeValue::boolean(a.equals(b)));
}

void StackMachine::execOPR_neq() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("NEQ", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    push(RuntimeValue::boolean(!a.equals(b)));
}

void StackMachine::execOPR_lss() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("LSS", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    push(RuntimeValue::boolean(a.compare(b) < 0));
}

void StackMachine::execOPR_geq() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("GEQ", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    push(RuntimeValue::boolean(a.compare(b) >= 0));
}

void StackMachine::execOPR_gtr() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("GTR", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    push(RuntimeValue::boolean(a.compare(b) > 0));
}

void StackMachine::execOPR_leq() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("LEQ", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    push(RuntimeValue::boolean(a.compare(b) <= 0));
}

void StackMachine::execOPR_wrt() {
    if (!hasValues(1)) RuntimeErrorHook::stackUnderflow("WRT", context.stack.size(), 1);
    RuntimeValue v = pop();
    context.output << v.toString();
}

void StackMachine::execOPR_wrtln() {
    if (!hasValues(1)) RuntimeErrorHook::stackUnderflow("WRTLN", context.stack.size(), 1);
    RuntimeValue v = pop();
    context.output << v.toString() << '\n';
}

// ---------------------------------------------------------------------------
// RET  – return from function
// ---------------------------------------------------------------------------
void StackMachine::execRET(const RuntimeInstruction& instr) {
    (void)instr;

    // For main program: return address is -1 → halt
    if (context.ra < 0) {
        context.halted = true;
        context.ip++;
        return;
    }

    // Pop current frame up to (and including) bp
    if (context.bp >= 0 && static_cast<size_t>(context.bp) < context.stack.size()) {
        context.stack.resize(static_cast<size_t>(context.bp));
    }

    // Restore previous frame
    context.bp = context.dl;
    context.dl = -1;  // PLACEHOLDER(Michael/Daniel): proper nested scopes

    // Jump to return address
    int tgt = context.ra - 1;  // 1‑based → 0‑based
    context.ra = -1;

    if (tgt < 0 || static_cast<size_t>(tgt) > context.program.size()) {
        context.halted = true;
    } else {
        context.ip = static_cast<size_t>(tgt);
    }
}

// ---------------------------------------------------------------------------
// utility helpers
// ---------------------------------------------------------------------------

long long StackMachine::asInteger(const RuntimeValue& v) {
    switch (v.kind) {
        case RuntimeValue::Kind::INTEGER: return v.intVal;
        case RuntimeValue::Kind::REAL:    return static_cast<long long>(v.realVal);
        case RuntimeValue::Kind::BOOLEAN: return v.boolVal ? 1 : 0;
        case RuntimeValue::Kind::CHAR:    return v.intVal;
        default:                          return 0;
    }
}

double StackMachine::asReal(const RuntimeValue& v) {
    switch (v.kind) {
        case RuntimeValue::Kind::INTEGER: return static_cast<double>(v.intVal);
        case RuntimeValue::Kind::REAL:    return v.realVal;
        case RuntimeValue::Kind::BOOLEAN: return v.boolVal ? 1.0 : 0.0;
        case RuntimeValue::Kind::CHAR:    return static_cast<double>(v.intVal);
        default:                          return 0.0;
    }
}

bool StackMachine::isNumber(const RuntimeValue& v) {
    return v.kind == RuntimeValue::Kind::INTEGER
        || v.kind == RuntimeValue::Kind::REAL;
}
