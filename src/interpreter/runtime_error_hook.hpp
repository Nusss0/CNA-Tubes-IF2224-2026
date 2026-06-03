#pragma once

#include <string>

// PLACEHOLDER(Nus): replace with final runtime safety / error handling.

namespace RuntimeErrorHook {

void divisionByZero();
void stackOverflow(size_t requested, size_t current, size_t max);
void stackUnderflow(const std::string& operation, size_t available, size_t needed);
void invalidJumpTarget(int target, int programSize);
void invalidMemoryAccess(int address, int bp, int stackSize);
void invalidOperand(const std::string& op, const std::string& detail);
void generic(const std::string& message);

}  // namespace RuntimeErrorHook
