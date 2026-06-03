#include "runtime_context.hpp"

void RuntimeContext::reset() {
    stack.clear();
    bp = 0;
    ra = -1;
    dl = -1;
    ip = 0;
    program.clear();
    output.str("");
    output.clear();
    halted = false;
}
