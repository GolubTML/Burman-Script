#pragma once
#include "token.h"
#include <map>
#include <variant>

struct Functions 
{
    std::vector<std::string> args;
    std::vector<Token> body;
};


using Variant = std::variant<int, double, std::string, bool>;

struct ReturnException {
    Variant value;
    ReturnException(const Variant& v) : value(v) {}
};

extern std::map<std::string, Variant> variables;
extern std::map<std::string, Functions> funcTable;

Variant parseExpr(std::vector<Token>& tokens, int& i);

Variant evaluate(std::vector<Token>& tokens);