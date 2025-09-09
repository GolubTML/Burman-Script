#pragma once
#include "token.h"
#include <map>
#include <variant>

using Variant = std::variant<int, double, std::string, bool>;

extern std::map<std::string, Variant> variables;

Variant parseExpr(std::vector<Token>& tokens, int& i);

Variant evaluate(std::vector<Token>& tokens);