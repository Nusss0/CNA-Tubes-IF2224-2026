#pragma once

#include "../std.hpp"
// Forward declaration to avoid circular include with token_processing
struct Token;

//Read everything from file and return it into one single line string.
string DFAFileReader(const string fileName);


void TokenPrinter(const vector<Token> &tokens);
void PrintTokenToFile(const string fileName, const vector<Token> &tokens);
bool IsFileExist(const string path);