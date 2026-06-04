#pragma once

#include <string>
#include <stdexcept>
#include <cstddef>

// Runtime safety & error handling untuk Arion interpreter.
// Tiap pelanggaran keamanan melempar RuntimeError (bukan crash mentah);
// interpreter/main yang menangkap lalu mencetak pesan rapi dan berhenti
// secara graceful.

// jenis error runtime — dipakai untuk klasifikasi & pesan
enum class RuntimeErrorKind {
    DIVISION_BY_ZERO,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    INVALID_JUMP,
    INVALID_MEMORY,
    INVALID_OPERAND,
    STACK_CORRUPTION,
    STACK_SMASHING,
    TYPE_MISMATCH,
    NUMERIC_OVERFLOW,
    NUMERIC_UNDERFLOW,
    INDEX_OUT_OF_BOUNDS,
    GENERIC
};

class RuntimeError : public std::runtime_error {
public:
    RuntimeError(RuntimeErrorKind k, const std::string& msg)
        : std::runtime_error(msg), kind(k) {}

    RuntimeErrorKind kind;
};

namespace RuntimeErrorHook {

// nama jenis error untuk dicetak (mis. "OverflowError")
std::string kindName(RuntimeErrorKind kind);

// tiap fungsi memformat pesan lalu melempar RuntimeError
[[noreturn]] void divisionByZero();
[[noreturn]] void stackOverflow(size_t requested, size_t current, size_t max);
[[noreturn]] void stackUnderflow(const std::string& operation, size_t available, size_t needed);
[[noreturn]] void invalidJumpTarget(int target, int programSize);
[[noreturn]] void invalidMemoryAccess(int address, int bp, int stackSize);
[[noreturn]] void invalidOperand(const std::string& op, const std::string& detail);
[[noreturn]] void stackCorruption(const std::string& detail, int expected, int actual);
[[noreturn]] void stackSmashing(int expectedRa, int actualRa);
[[noreturn]] void typeMismatch(const std::string& op, const std::string& expected, const std::string& got);
[[noreturn]] void numericOverflow(const std::string& op);
[[noreturn]] void numericUnderflow(const std::string& op);
[[noreturn]] void indexOutOfBounds(int index, int length);
[[noreturn]] void generic(const std::string& message);

}  // namespace RuntimeErrorHook
