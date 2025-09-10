#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "../headers/token.h"
#include "../headers/parser.h"


int main()
{
    std::ifstream file("code.bs");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    auto tokens = tokenize(code);

    std::ofstream out("tokens.log");

    for (auto &t : tokens)
    {
        out << t.type << " -> " << t.value << std::endl;
    }

    out.close();

    auto result = evaluate(tokens);
    return 0;
}