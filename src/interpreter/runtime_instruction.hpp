#pragma once

#include <string>
#include <vector>

struct RuntimeInstruction {
    std::string op;
    std::vector<std::string> args;
    std::string label;
    int target = -1;  // resolved jump target (1‑based instruction number)
};
