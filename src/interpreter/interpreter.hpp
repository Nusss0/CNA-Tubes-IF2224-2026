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

    // jalankan program; menangkap RuntimeError supaya tidak crash.
    // kembalikan true jika selesai tanpa error runtime.
    bool run();

    std::string getOutput() const;
    bool isHalted() const;

    // info error runtime (jika run() mengembalikan false)
    bool hasError() const { return errorRaised; }
    const std::string& getError() const { return errorMessage; }

private:
    std::unique_ptr<StackMachine> vm;
    bool errorRaised = false;
    std::string errorMessage;
};
