#include "interpreter.hpp"
#include "stack_machine.hpp"

Interpreter::Interpreter() : vm(std::make_unique<StackMachine>()) {}
Interpreter::~Interpreter() = default;

void Interpreter::loadProgram(const std::vector<RuntimeInstruction>& instrs) {
    vm->loadProgram(instrs);
}

void Interpreter::run() {
    vm->run();
}

std::string Interpreter::getOutput() const {
    return vm->getOutput();
}

bool Interpreter::isHalted() const {
    return vm->isHalted();
}
