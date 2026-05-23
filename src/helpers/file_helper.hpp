#pragma once

#include "../std.hpp"
// fwd decl
struct Token;

// read file
string DFAFileReader(const string fileName);

// load tokens
vector<Token> TokenFileReader(const string& path);

// auto-detect format
vector<Token> LoadTokens(const string& path);

void TokenPrinter(const vector<Token> &tokens);
void PrintTokenToFile(const string fileName, const vector<Token> &tokens);
bool IsFileExist(const string path);