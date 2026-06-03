#include "src/interpreter/interpreter.hpp"
#include "src/interpreter/runtime_instruction.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

// Parse a TAC text file into a vector of RuntimeInstruction.
// Format per line (space‑separated):
//   <line_number> <OP> <arg1> [<arg2> ...]
// Lines starting with '#' or empty are ignored.
// For JMP/JPC instructions the last argument is treated as a numeric
// jump target and stored in RuntimeInstruction::target.

static vector<RuntimeInstruction> parseTacFile(const string& path) {
    ifstream fin(path);
    if (!fin) {
        cerr << "[ERROR] Cannot open TAC file: " << path << '\n';
        exit(1);
    }

    vector<RuntimeInstruction> program;
    string line;

    while (getline(fin, line)) {
        // trim
        size_t start = line.find_first_not_of(" \t\r");
        if (start == string::npos) continue;
        if (line[start] == '#') continue;
        size_t end = line.find_last_not_of(" \t\r");
        line = line.substr(start, end - start + 1);

        istringstream iss(line);
        vector<string> tokens;
        string tok;
        while (iss >> tok) tokens.push_back(tok);
        if (tokens.empty()) continue;

        // first token = line number (discard for ordering; we use insertion order)
        size_t idx = 1;
        if (tokens[0].find_first_not_of("0123456789") == string::npos) {
            idx = 1;  // skip the line number
        } else {
            idx = 0;  // no line number
        }

        if (idx >= tokens.size()) continue;

        RuntimeInstruction ri;
        ri.op = tokens[idx++];

        while (idx < tokens.size()) {
            ri.args.push_back(tokens[idx++]);
        }

        // For JMP / JPC: the last arg is the numeric target
        if (ri.op == "JMP" || ri.op == "JPC") {
            if (!ri.args.empty()) {
                ri.target = stoi(ri.args.back());
            }
        }

        program.push_back(ri);
    }

    return program;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: test_interpreter <tac_file>\n";
        cerr << "  Runs the TAC program and prints its output.\n";
        return 1;
    }

    string path = argv[1];

    vector<RuntimeInstruction> program = parseTacFile(path);

    Interpreter interp;
    interp.loadProgram(program);

    try {
        interp.run();
        cout << interp.getOutput();
        if (!interp.getOutput().empty() && interp.getOutput().back() != '\n') {
            cout << '\n';
        }
    } catch (const exception& e) {
        cerr << "[INTERPRETER ERROR] " << e.what() << '\n';
        return 1;
    }

    return 0;
}
