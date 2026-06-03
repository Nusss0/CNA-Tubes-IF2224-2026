#pragma once

#include <vector>
#include "runtime_instruction.hpp"
#include "../intermediate/tac.hpp"

// Adapter between Michael/Daniel's TacGenerator output and Nuno's interpreter.
// – reads the existing TacInstr struct (defined in intermediate/tac.hpp)
// – converts to RuntimeInstruction (used by StackMachine)
// – interpreter code stays decoupled from TacGenerator internals.

namespace TacAdapter {

RuntimeInstruction convert(const TacInstr& src);

// Convert a whole program vector
std::vector<RuntimeInstruction> convertAll(const std::vector<TacInstr>& src);

}  // namespace TacAdapter
