#include "runtime_error_hook.hpp"
#include <iostream>
#include <stdexcept>

using namespace std;

namespace RuntimeErrorHook {

[[noreturn]] static void die(const string& msg) {
    cerr << "[RUNTIME ERROR] " << msg << '\n';
    // PLACEHOLDER(Nus): throw a proper exception class, log, or halt gracefully.
    throw runtime_error(msg);
}

void divisionByZero() {
    die("Division by zero");
}

void stackOverflow(size_t requested, size_t current, size_t max) {
    die("Stack overflow: requested " + to_string(requested)
        + " cells, current size " + to_string(current)
        + ", max " + to_string(max));
}

void stackUnderflow(const string& operation, size_t available, size_t needed) {
    die("Stack underflow in '" + operation + "': needed "
        + to_string(needed) + " values, available " + to_string(available));
}

void invalidJumpTarget(int target, int programSize) {
    die("Invalid jump target " + to_string(target)
        + " (program has " + to_string(programSize) + " instructions)");
}

void invalidMemoryAccess(int address, int bp, int stackSize) {
    die("Invalid memory access at address " + to_string(address)
        + " (bp=" + to_string(bp) + ", stack size=" + to_string(stackSize) + ")");
}

void invalidOperand(const string& op, const string& detail) {
    die("Invalid operand in '" + op + "': " + detail);
}

void generic(const string& message) {
    die(message);
}

}  // namespace RuntimeErrorHook
