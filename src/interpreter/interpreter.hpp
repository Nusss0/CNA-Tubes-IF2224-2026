#pragma once

#include <string>
#include <vector>
#include <memory>
#include "runtime_instruction.hpp"

class StackMachine;

class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;

    void loadProgram(const std::vector<RuntimeInstruction>& instrs);

    void run();

    std::string getOutput() const;
    bool isHalted() const;

private:
    std::unique_ptr<StackMachine> vm;
};
