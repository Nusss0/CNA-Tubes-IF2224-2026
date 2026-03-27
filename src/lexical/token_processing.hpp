#pragma once

#include "../std.hpp"

struct Token{
    string type;
    string value;
};

vector<Token> Tokenizing(const string &raw);