#pragma once
#include <string>
#include <vector>

struct Token 
{
    std::string type;
    std::string value;
};

std::vector<Token> tokenize(const std::string& src);