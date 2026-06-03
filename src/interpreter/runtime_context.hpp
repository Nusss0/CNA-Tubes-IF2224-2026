#pragma once

#include "runtime_value.hpp"
#include "runtime_instruction.hpp"
#include <vector>
#include <sstream>
#include <string>
#include <cstddef>

struct RuntimeContext {
    std::vector<RuntimeValue> stack;
    int bp = 0;   // base pointer  (stack index of current frame start)
    int ra = -1;  // return address (1‑based instruction number, -1 = none)
    int dl = -1;  // dynamic link  (previous bp value)

    // instruction pointer (index into the program vector, 0‑based)
    size_t ip = 0;

    // program loaded
    std::vector<RuntimeInstruction> program;

    // output buffer
    std::ostringstream output;

    bool halted = false;

    void reset();
};
