#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include "../headers/token.h"
#include "../headers/parser.h"


int main(int argc, char* argv[])
{
    if (argv[1] == nullptr)
        throw std::runtime_error("No input files!");

    std::ifstream file(argv[1]);

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