#include "stack_machine.hpp"
#include "runtime_error_hook.hpp"
#include <stdexcept>
#include <cstdlib>
#include <limits>

using namespace std;

// batas kedalaman stack — cegah stack overflow (mis. rekursi tanpa base case)
static const size_t MAX_STACK_DEPTH = 100000;
// batas jumlah frame — sesuai spek (maksimal ~1000 frame)
static const int MAX_FRAME_DEPTH = 1000;

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
    if (context.stack.size() >= MAX_STACK_DEPTH) {
        RuntimeErrorHook::stackOverflow(1, context.stack.size(), MAX_STACK_DEPTH);
    }
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
    int size = parseIntArg(instr, 1);

    // cek batas frame — cegah stack overflow akibat rekursi tanpa base case
    if (context.frameDepth >= MAX_FRAME_DEPTH) {
        RuntimeErrorHook::stackOverflow(static_cast<size_t>(size),
                                        context.stack.size(),
                                        static_cast<size_t>(MAX_FRAME_DEPTH));
    }
    context.frameDepth++;

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
    int addr = parseIntArg(instr, 1);
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
    int addr = parseIntArg(instr, 1);
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

    // Save return address (1‑based line number of instruksi berikutnya)
    context.ra = static_cast<int>(context.ip) + 1 + 1;

    // Jump to target (validasi + konversi 1‑based → 0‑based)
    context.ip = resolveJump(instr.target);
}

// ---------------------------------------------------------------------------
// resolveJump – validasi target lompat 1‑based, kembalikan index 0‑based.
// target -1 berarti label tak ditemukan saat resolusi (invalid jump target).
// ---------------------------------------------------------------------------
size_t StackMachine::resolveJump(int target) const {
    int progSize = static_cast<int>(context.program.size());
    // target < 1 (termasuk -1 label tak resolve) atau di luar batas → invalid
    if (target < 1 || target > progSize) {
        RuntimeErrorHook::invalidJumpTarget(target, progSize);
    }
    return static_cast<size_t>(target - 1);  // 1‑based → 0‑based
}

// ---------------------------------------------------------------------------
// JMP  – unconditional jump
// ---------------------------------------------------------------------------
void StackMachine::execJMP(const RuntimeInstruction& instr) {
    context.ip = resolveJump(instr.target);
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
        context.ip = resolveJump(instr.target);
    } else {
        context.ip++;
    }
}

// ---------------------------------------------------------------------------
// OPR  – operation:  OPR <lev> <opcode>
// ---------------------------------------------------------------------------
void StackMachine::execOPR(const RuntimeInstruction& instr) {
    int code = parseIntArg(instr, 1);

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
        push(RuntimeValue::integer(negChecked(a.intVal, "NEG")));
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
            push(RuntimeValue::integer(addChecked(asInteger(a), asInteger(b), "ADD")));
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
        push(RuntimeValue::integer(subChecked(asInteger(a), asInteger(b), "SUB")));
}

void StackMachine::execOPR_mul() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("MUL", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    if (a.kind == RuntimeValue::Kind::REAL || b.kind == RuntimeValue::Kind::REAL)
        push(RuntimeValue::real(asReal(a) * asReal(b)));
    else
        push(RuntimeValue::integer(mulChecked(asInteger(a), asInteger(b), "MUL")));
}

void StackMachine::execOPR_div() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("DIV", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    double divisor = asReal(b);
    if (divisor == 0.0) RuntimeErrorHook::divisionByZero();
    // Arion div returns integer if both operands are integer
    if (a.kind == RuntimeValue::Kind::INTEGER && b.kind == RuntimeValue::Kind::INTEGER)
        push(RuntimeValue::integer(divChecked(asInteger(a), asInteger(b), "DIV")));
    else
        push(RuntimeValue::real(asReal(a) / divisor));
}

void StackMachine::execOPR_mod() {
    if (!hasValues(2)) RuntimeErrorHook::stackUnderflow("MOD", context.stack.size(), 2);
    RuntimeValue b = pop();
    RuntimeValue a = pop();
    long long divisor = asInteger(b);
    if (divisor == 0) RuntimeErrorHook::divisionByZero();
    push(RuntimeValue::integer(modChecked(asInteger(a), divisor, "MOD")));
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

    if (context.frameDepth > 0) {
        context.frameDepth--;
    }

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

// ---------------------------------------------------------------------------
// parseIntArg – ambil argumen ke-idx sebagai integer dgn validasi penuh.
// argumen hilang / bukan angka / di luar jangkauan int → InvalidOperand
// (RuntimeError), bukan std::exception mentah dari stoi yang bikin crash.
// ---------------------------------------------------------------------------
int StackMachine::parseIntArg(const RuntimeInstruction& instr, size_t idx) {
    if (idx >= instr.args.size()) {
        RuntimeErrorHook::invalidOperand(instr.op,
            "missing integer argument at position " + to_string(idx));
    }
    const string& raw = instr.args[idx];
    size_t consumed = 0;
    long long value = 0;
    try {
        value = stoll(raw, &consumed);
    } catch (const exception&) {
        RuntimeErrorHook::invalidOperand(instr.op, "argument '" + raw + "' is not an integer");
    }
    // tolak sisa karakter non-angka (mis. "12x")
    if (consumed != raw.size()) {
        RuntimeErrorHook::invalidOperand(instr.op, "argument '" + raw + "' is not an integer");
    }
    if (value < numeric_limits<int>::min() || value > numeric_limits<int>::max()) {
        RuntimeErrorHook::invalidOperand(instr.op, "argument '" + raw + "' is out of range");
    }
    return static_cast<int>(value);
}

// ---------------------------------------------------------------------------
// integer aritmatika dengan deteksi overflow/underflow
// pakai builtin g++ supaya wrap-around ketahuan, lalu lempar Over/UnderflowError
// ---------------------------------------------------------------------------

long long StackMachine::addChecked(long long a, long long b, const string& op) {
    long long r;
    if (__builtin_add_overflow(a, b, &r)) {
        if (a > 0 && b > 0) RuntimeErrorHook::numericOverflow(op);
        RuntimeErrorHook::numericUnderflow(op);
    }
    return r;
}

long long StackMachine::subChecked(long long a, long long b, const string& op) {
    long long r;
    if (__builtin_sub_overflow(a, b, &r)) {
        if (a >= 0 && b < 0) RuntimeErrorHook::numericOverflow(op);
        RuntimeErrorHook::numericUnderflow(op);
    }
    return r;
}

long long StackMachine::mulChecked(long long a, long long b, const string& op) {
    long long r;
    if (__builtin_mul_overflow(a, b, &r)) {
        bool sameSign = (a > 0) == (b > 0);
        if (sameSign) RuntimeErrorHook::numericOverflow(op);
        RuntimeErrorHook::numericUnderflow(op);
    }
    return r;
}

long long StackMachine::divChecked(long long a, long long b, const string& op) {
    // pembagian dgn 0 ditangani pemanggil; sisa kasus UB hanya MIN / -1
    if (a == numeric_limits<long long>::min() && b == -1) {
        RuntimeErrorHook::numericOverflow(op);
    }
    return a / b;
}

long long StackMachine::modChecked(long long a, long long b, const string& op) {
    // MIN % -1 juga UB walau hasil matematisnya 0
    if (a == numeric_limits<long long>::min() && b == -1) {
        RuntimeErrorHook::numericOverflow(op);
    }
    return a % b;
}

long long StackMachine::negChecked(long long a, const string& op) {
    long long r;
    if (__builtin_sub_overflow(0LL, a, &r)) {
        RuntimeErrorHook::numericOverflow(op);
    }
    return r;
}
