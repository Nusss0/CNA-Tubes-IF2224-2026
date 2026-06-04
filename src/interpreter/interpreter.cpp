#include "interpreter.hpp"
#include "stack_machine.hpp"
#include "runtime_error_hook.hpp"

Interpreter::Interpreter() : vm(std::make_unique<StackMachine>()) {}
Interpreter::~Interpreter() = default;

void Interpreter::loadProgram(const std::vector<RuntimeInstruction>& instrs) {
    vm->loadProgram(instrs);
}

bool Interpreter::run() {
    errorRaised = false;
    errorMessage.clear();
    // tangkap RuntimeError supaya program berhenti graceful, bukan crash
    try {
        vm->run();
        return true;
    } catch (const RuntimeError& e) {
        errorRaised = true;
        errorMessage = e.what();
        return false;
    }
}

std::string Interpreter::getOutput() const {
    return vm->getOutput();
}

bool Interpreter::isHalted() const {
    return vm->isHalted();
}
