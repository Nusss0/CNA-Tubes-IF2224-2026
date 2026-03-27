#pragma once

#include "../std.hpp"
#include "../lexical/token_processing.hpp"
//Read everything from file and return it into one single line string.
string DFAFileReader(const string fileName);


void TokenPrinter(vector<Token> &tokens);