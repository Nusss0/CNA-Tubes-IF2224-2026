#include "runtime_error_hook.hpp"

using namespace std;

namespace RuntimeErrorHook {

string kindName(RuntimeErrorKind kind) {
    switch (kind) {
        case RuntimeErrorKind::DIVISION_BY_ZERO:   return "DivisionByZeroError";
        case RuntimeErrorKind::STACK_OVERFLOW:     return "StackOverflowError";
        case RuntimeErrorKind::STACK_UNDERFLOW:    return "StackUnderflowError";
        case RuntimeErrorKind::INVALID_JUMP:       return "InvalidJumpError";
        case RuntimeErrorKind::INVALID_MEMORY:     return "InvalidMemoryAccessError";
        case RuntimeErrorKind::INVALID_OPERAND:    return "InvalidOperandError";
        case RuntimeErrorKind::NUMERIC_OVERFLOW:   return "OverflowError";
        case RuntimeErrorKind::NUMERIC_UNDERFLOW:  return "UnderflowError";
        case RuntimeErrorKind::INDEX_OUT_OF_BOUNDS:return "IndexOutOfBoundsError";
        case RuntimeErrorKind::GENERIC:            return "RuntimeError";
    }
    return "RuntimeError";
}

// bikin pesan format "[ERROR] <kind>: <detail> !" lalu lempar
[[noreturn]] static void die(RuntimeErrorKind kind, const string& detail) {
    string msg = "[ERROR] " + kindName(kind) + ": " + detail + " !";
    throw RuntimeError(kind, msg);
}

void divisionByZero() {
    die(RuntimeErrorKind::DIVISION_BY_ZERO, "division by zero");
}

void stackOverflow(size_t requested, size_t current, size_t max) {
    die(RuntimeErrorKind::STACK_OVERFLOW,
        "cannot allocate " + to_string(requested) + " cell(s); stack at "
        + to_string(current) + " of max " + to_string(max));
}

void stackUnderflow(const string& operation, size_t available, size_t needed) {
    die(RuntimeErrorKind::STACK_UNDERFLOW,
        "'" + operation + "' needs " + to_string(needed)
        + " value(s) but only " + to_string(available) + " available");
}

void invalidJumpTarget(int target, int programSize) {
    die(RuntimeErrorKind::INVALID_JUMP,
        "jump target " + to_string(target)
        + " is out of range (program has " + to_string(programSize) + " instruction(s))");
}

void invalidMemoryAccess(int address, int bp, int stackSize) {
    die(RuntimeErrorKind::INVALID_MEMORY,
        "address " + to_string(address) + " (bp=" + to_string(bp)
        + ", stack size=" + to_string(stackSize) + ") is out of range");
}

void invalidOperand(const string& op, const string& detail) {
    die(RuntimeErrorKind::INVALID_OPERAND, "'" + op + "': " + detail);
}

void numericOverflow(const string& op) {
    die(RuntimeErrorKind::NUMERIC_OVERFLOW,
        "'" + op + "' result exceeds the maximum integer value");
}

void numericUnderflow(const string& op) {
    die(RuntimeErrorKind::NUMERIC_UNDERFLOW,
        "'" + op + "' result falls below the minimum integer value");
}

void indexOutOfBounds(int index, int length) {
    die(RuntimeErrorKind::INDEX_OUT_OF_BOUNDS,
        "index " + to_string(index) + " is out of bounds for length "
        + to_string(length));
}

void generic(const string& message) {
    die(RuntimeErrorKind::GENERIC, message);
}

}  // namespace RuntimeErrorHook
