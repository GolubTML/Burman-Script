#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
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

    auto start = std::chrono::high_resolution_clock::now();
    auto result = evaluate(tokens);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Execution time: " 
          << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
          << " ms" << std::endl;
    return 0;
}