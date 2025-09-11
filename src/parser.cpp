#include <iostream>
#include <string>
#include "../headers/parser.h"

std::map<std::string, Variant> variables;

Variant parseExpr(std::vector<Token>& tokens, int& i);

Variant parserCompretions(std::vector<Token>& tokens, int& i)
{
    Variant left = parseExpr(tokens, i);

    while (i < tokens.size() && (tokens[i].value == "==" 
        || tokens[i].value == "!=" || tokens[i].value == "<" 
        || tokens[i].value == ">"  || tokens[i].value == "<=" 
        || tokens[i].value == ">="))
    {
        std::string op = tokens[i].value;
        i++;

        Variant right = parseExpr(tokens, i);

        if (op == "==") return left == right;
        else if (op == "!=")return  left != right;

        if ((!std::holds_alternative<int>(left) && !std::holds_alternative<double>(left)) 
        || (!std::holds_alternative<int>(right) && !std::holds_alternative<double>(right)))
        {
            throw std::runtime_error("Операторы ==, !=, >, >=, <, <= - доступны только для чисел!");
        }

        double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
        double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);

        if (op == "<") return l < r;
        if (op == ">") return l > r;
        if (op == "<=") return l <= r;
        if (op == ">=") return l >= r;
    }
    
    return left;
}

Variant parseLogic(std::vector<Token>& tokens, int& i)
{
    Variant left = parserCompretions(tokens, i);

    while (i < tokens.size() && (tokens[i].value == "&&" || tokens[i].value == "||"))
    {
        std::string op = tokens[i].value;
        i++;
        Variant right = parserCompretions(tokens, i);

        if (!std::holds_alternative<bool>(left) || !std::holds_alternative<bool>(right))
        {
            throw std::runtime_error("Операторы &&, || - доступны только для булевых значений!");
        }

        bool l = std::get<bool>(left);
        bool r = std::get<bool>(right);

        if (op == "&&") left = l && r;
        else if (op == "||") left = l || r;
    }

    return left;
}

Variant parseFactor(std::vector<Token>& tokens, int& i)
{
    Token t = tokens[i];

    if (t.type == "DIGIT" || t.type == "FLOAT")
    {
        double value = (t.value.find('.') != std::string::npos) 
                        ? std::stod(t.value) 
                        : std::stoi(t.value);
        i++;
        return value;
    } 
    else if (t.type == "BOOL")
    {
        bool value = (t.value == "true");
        i++;
        return value;
    }
    else if (t.type == "STRING")
    {
        std::string str = t.value;
        i++;
        return t.value;
    }
    else if (t.type == "IDENT") 
    {
        std::string varName = t.value;
        i++;

        if (variables.find(varName) == variables.end()) 
            throw std::runtime_error("Variable not found: " + varName);

        return variables[varName];
    } 
    else if (t.value == "(")
    {
        i++;
        Variant value = parseExpr(tokens, i);
        if (tokens[i].value == ")") i++;
        return value;
    }

    throw std::runtime_error("Unexpected token: " + t.value);
}

double getNumber(const Variant& v) 
{
    return std::visit([](auto&& arg) -> double {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>)
            return static_cast<double>(arg);
        else if constexpr (std::is_same_v<T, double>)
            return arg;
        else
            throw std::runtime_error("Нельзя использовать нечисловую переменную в выражении");
    }, v);
}


Variant parseTerm(std::vector<Token>& tokens, int& i)
{
    Variant left = parseFactor(tokens, i);

    while (i < tokens.size() && (tokens[i].value == "*" || tokens[i].value == "/"))
    {
        std::string op = tokens[i].value;
        i++;
        Variant right = parseFactor(tokens, i);

        if (!std::holds_alternative<int>(left) && !std::holds_alternative<double>(left) ||
            !std::holds_alternative<int>(right) && !std::holds_alternative<double>(right))
        {
            throw std::runtime_error("Нельзя использовать нечисловую переменную в арифметике");
        }

        double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
        double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);

        if (op == "*") left = l * r;
        else if (op == "/") left = l / r;
    }

    return left;
}


Variant parseExpr(std::vector<Token>& tokens, int& i)
{
    Variant left = parseTerm(tokens, i);

    while (i < tokens.size() && (tokens[i].value == "+" || tokens[i].value == "-"))
    {
        std::string op = tokens[i].value;
        i++;
        Variant right = parseTerm(tokens, i);

        if ((!std::holds_alternative<int>(left) && !std::holds_alternative<double>(left)) ||
            (!std::holds_alternative<int>(right) && !std::holds_alternative<double>(right)))
        {
            throw std::runtime_error("Нельзя использовать нечисловую переменную в арифметике");
        }

        double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
        double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);

        if (op == "+") left = l + r;
        else if (op == "-") left = l - r;
    }

    return left;
}

Variant evaluate(std::vector<Token>& tokens)
{
    Variant result = 0;
    std::string op = "+";
    int i = 0;

    while (i < tokens.size())
    {
        auto &t = tokens[i];

        if (t.type == "KEYWORD" && t.value == "let")
        {
            std::string varName = tokens[++i].value;

            if (tokens[++i].value != "=")
            {
                std::cout << "No '=' matching in varible!";
                return 0;
            }

            i++;
            Variant value = parseExpr(tokens, i);
            variables[varName] = value;
            result = value;
        }
        else if (t.type == "KEYWORD" && t.value == "print")
        {
            Token next = tokens[++i];

            if (next.type == "DIGIT")
                std::cout << std::stoi(next.value) << std::endl;
            else if (next.type == "FLOAT")
                std::cout << std::stod(next.value) << std::endl;
            else if (next.type == "BOOL")
                std::cout << (next.value == "true" ? "true" : "false") << std::endl;
            else if (next.type == "STRING")
                std::cout << next.value << std::endl;
            else if (next.type == "IDENT")
            {
                if (variables.find(next.value) == variables.end())
                {
                    throw std::runtime_error("Переменная не найдена: " + next.value);
                }
                std::visit([](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_same_v<T, bool>)
                        std::cout << (arg ? "true" : "false") << std::endl;
                    else if constexpr (std::is_same_v<T, std::string>)
                        std::cout << arg << std::endl;
                    else
                        std::cout << arg << std::endl;
                }, variables[next.value]);
            }
        }
        else if (t.type == "KEYWORD" && t.value == "if")
        {
            i++;

            Variant condition = parseLogic(tokens, i);

            if (!std::holds_alternative<bool>(condition))
                throw std::runtime_error("Условие if должно быть булевым");

            if (tokens[i].value != "{")
                throw std::runtime_error("Ожидался '{' после if");

            i++;
            std::vector<Token> blockTokens;
            while (i < tokens.size() && tokens[i].value != "}")
                blockTokens.push_back(tokens[i++]);
            i++;

            if (std::get<bool>(condition))
                evaluate(blockTokens);
            else
            {
                if (i < tokens.size() && t.type == "KEYWORD" && t.value == "else")
                {
                    i++;

                    if (tokens[i].value != "{")
                        throw std::runtime_error("Ожидался '{' после else");
                    i++;
                    
                    std::vector<Token> elseTokens;
                    while (i < tokens.size() && tokens[i].value != "}")
                        elseTokens.push_back(tokens[i++]);

                    i++;
                    evaluate(elseTokens);
                }
            }
        }
        else if (t.type == "KEYWORD" && t.value == "while")
        {
            i++;
            
            int startPos = i;
            Variant condition = parseLogic(tokens, i);

            if (!std::holds_alternative<bool>(condition))
                throw std::runtime_error("Условие while должно быть булевым");

            if (tokens[i].value != "{")
                throw std::runtime_error("После while цикла должено стоять {");

            i++;

            std::vector<Token> conditionBlock;
            while (i < tokens.size() && tokens[i].value != "}")
                conditionBlock.push_back(tokens[i++]);
            i++;

            while (true)
            {
                int index = startPos;
                Variant condValue = parseLogic(tokens, index);
                
                if (!std::holds_alternative<bool>(condValue))
                    throw std::runtime_error("Условие while должно быть булевым");

                if (!std::get<bool>(condValue))
                    break;

                evaluate(conditionBlock);
            }
        }
        else if (t.type == "IDENT" && i + 1 < tokens.size() && tokens[i + 1].value == "++")
        {
            std::string varName = t.value;

            if (!std::holds_alternative<int>(variables[varName]) && !std::holds_alternative<double>(variables[varName]))
                throw std::runtime_error("Cannot use ++ on non digit variables.");

            double var = getNumber(variables[varName]);
            variables[varName] = var + 1;
            i += 2;
        }
        else if (t.type == "IDENT" && i + 1 < tokens.size() && tokens[i + 1].value == "--")
        {
            std::string varName = t.value;

            if (!std::holds_alternative<int>(variables[varName]) && !std::holds_alternative<double>(variables[varName]))
                throw std::runtime_error("Cannot use -- on non digit variables.");

            double var = getNumber(variables[varName]);
            variables[varName] = var - 1;
            i += 2;
        }
        else
        {
            i++;
        }
    }

    return result;
}