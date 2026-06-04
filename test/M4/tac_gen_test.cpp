// Non-interactive driver for the Intermediate Code (TAC) generator.
//
//   Usage: tac_gen_test <source_file.txt>
//
// Runs the real front-end pipeline (lexer -> parser -> AST builder ->
// semantic analyzer -> AST decorator), generates stack-machine TAC with
// TacGenerator, prints it, then executes it on the interpreter and prints
// the program output.
//
// Build (from repo root), all on one line:
//   g++ -std=c++17 -Isrc src/helpers/*.cpp src/lexical/*.cpp src/syntax/*.cpp
//       src/semantic/*.cpp src/intermediate/*.cpp src/interpreter/*.cpp
//       test/M4/tac_gen_test.cpp -o bin/tac_gen_test

#include "main.hpp"
#include "interpreter/interpreter.hpp"
#include "interpreter/tac_adapter.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: tac_gen_test <source_file.txt>\n";
        return 1;
    }
    string path = argv[1];

    // --- lex ---
    string raw = DFAFileReader(path);
    vector<Token> tokens = Tokenizing(raw);
    {
        vector<Token> filtered;
        for (const auto& t : tokens) if (t.type != "comment") filtered.push_back(t);
        tokens.swap(filtered);
    }

    // --- parse ---
    Parser parser(tokens);
    NodePtr root = parser.parseProgram();
    if (!root || parser.hasError()) {
        cerr << "[ERROR] parsing failed with "
             << parser.getErrors().size() << " error(s)\n";
        return 1;
    }

    // --- build AST ---
    AstBuilder builder;
    AstNodePtr ast = builder.build(root);
    if (!ast) {
        cerr << "[ERROR] AST build failed\n";
        return 1;
    }

    // --- semantic + decorate ---
    SemanticAnalyzer analyzer;
    analyzer.analyze(root);
    AstDecorator decorator(analyzer.getSymbols());
    decorator.decorate(ast);
    if (analyzer.hasErrors()) {
        cerr << "[ERROR] semantic analysis failed with "
             << analyzer.getErrors().size() << " error(s)\n";
        for (const auto& e : analyzer.getErrors()) cerr << "  " << e << "\n";
        return 1;
    }

    // --- generate TAC ---
    TacGenerator gen;
    gen.generate(ast);

    cout << "=== Intermediate Code ===\n";
    gen.print(cout);

    // --- run on interpreter ---
    cout << "\n=== Program Output ===\n";
    vector<RuntimeInstruction> program = TacAdapter::convertAll(gen.instructions());
    Interpreter interp;
    interp.loadProgram(program);
    try {
        interp.run();
        cout << interp.getOutput();
    } catch (const exception& e) {
        cerr << "[INTERPRETER ERROR] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
