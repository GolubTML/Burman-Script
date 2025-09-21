#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include "../headers/parser.h"

std::map<std::string, Variant> variables;
std::map<std::string, Functions> funcTable;

std::vector<Token> parseBlock(std::vector<Token>& tokens, int& i)
{
    if (tokens[i].value != "{")
        throw std::runtime_error("После while цикла должено стоять {");

    i++;

    std::vector<Token> conditionBlock;
    int braceCount = 1;
    while (i < tokens.size() && braceCount > 0)
    {
        if (tokens[i].value == "{") braceCount++;
        else if (tokens[i].value == "}") braceCount--;
    
        if (braceCount > 0) conditionBlock.push_back(tokens[i]);
        i++;
    }

    return conditionBlock;
}

Variant convert(const std::string& line)
{
    bool isInt = !line.empty() && std::all_of(line.begin(), line.end(), [](char c){ return std::isdigit(c) || c == '-'; });
     
    if (isInt) 
    {
        try { return std::stoi(line); }
        catch (...) { return line; }
    }

    bool isFloat = (line.find('.') != std::string::npos);
    if (isFloat)
    {
        try { return std::stod(line); }
        catch (...) { return line; }
    }

    return line;
}

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
        bool value = (t.value == "true" || t.value == "serega") ;
        i++;
        return value;
    }
    else if (t.type == "STRING")
    {
        std::string str = t.value;
        if (!str.empty() && str.front() == '"' && str.back() == '"')
            str = str.substr(1, str.size() - 2);

        i++;
        return str;
    }
    else if (t.type == "IDENT") 
    {
        std::string varName = t.value;
        i++;

        if (i < tokens.size() && tokens[i].value == "(")
        {
            std::string funcName = varName;
            i++;
            
            std::vector<Variant> callArgs;
            while (i < tokens.size() && tokens[i].value != ")")
            {
                if (tokens[i].value == ",") 
                { 
                    i++; 
                    continue; 
                }
            
                callArgs.push_back(parseExpr(tokens, i));
            }

            if (i >= tokens.size() || tokens[i].value != ")")
                throw std::runtime_error("Expected ')' at the end of the fucntions");
            i++;

            if (funcTable.find(funcName) == funcTable.end())
                throw std::runtime_error("Function not found: " + funcName);

            Functions func = funcTable[funcName];
            if (func.args.size() != callArgs.size())
                throw std::runtime_error("Not found function with this amount of arguments\nFunction name: " + funcName);

            std::map<std::string, Variant> backup = variables;
            for (size_t j = 0; j < func.args.size(); j++)
                variables[func.args[j]] = callArgs[j];

            Variant res;
            try 
            {
                res = evaluate(func.body);
            }
            catch (ReturnException& re)
            {
                res = re.value;
            }

            std::cerr << ">>> Calling function " << funcName << " with body size: " << func.body.size() << "\n";
            for (auto &tok : func.body) {
                std::cerr << tok.value << " ";
            }
            std::cerr << "\n";


            variables = backup;
            return res;

        }
        if (variables.find(varName) == variables.end()) 
            throw std::runtime_error("Variable not found: " + varName);

        return variables[varName];
    } 
    else if (t.type == "KEYWORD" && (t.value == "input" || t.value == "fuck"))
    {
        i++;
        std::string line;
        std::getline(std::cin, line);
        return convert(line);
    }
    else if (t.type == "KEYWORD" && (t.value == "int" || t.value == "pidor"))
    {
        i++;
        if (tokens[i].value != "(") throw std::runtime_error("Ожидался '(' после int");
        i++;
        Variant arg = parseExpr(tokens, i);
        if (tokens[i].value != ")") throw std::runtime_error("Ожидался ')' после int");
        i++;

        if (std::holds_alternative<std::string>(arg))
            return std::stoi(std::get<std::string>(arg));
        if (std::holds_alternative<double>(arg))
            return static_cast<int>(std::get<double>(arg));
        if (std::holds_alternative<int>(arg))
            return arg;
        if (std::holds_alternative<bool>(arg))
            return std::get<bool>(arg) ? 1 : 0;

        throw std::runtime_error("Нельзя преобразовать в int");
    }
    else if (t.type == "KEYWORD" && (t.value == "float" || t.value == "nefor"))
    {
        i++;
        if (tokens[i].value != "(") throw std::runtime_error("Ожидался '(' после float");
        i++;
        Variant arg = parseExpr(tokens, i);
        if (tokens[i].value != ")") throw std::runtime_error("Ожидался ')' после float");
        i++;

        if (std::holds_alternative<std::string>(arg))
            return std::stod(std::get<std::string>(arg));
        if (std::holds_alternative<int>(arg))
            return static_cast<double>(std::get<int>(arg));
        if (std::holds_alternative<double>(arg))
            return arg;
        if (std::holds_alternative<bool>(arg))
            return std::get<bool>(arg) ? 1 : 0;

        throw std::runtime_error("Нельзя преобразовать в float");
    }
    else if (t.type == "KEYWORD" && (t.value == "str" || t.value == "muzik"))
    {
        i++;
        if (tokens[i].value != "(") throw std::runtime_error("Ожидался '(' после str");
        i++;
        Variant arg = parseExpr(tokens, i);
        if (tokens[i].value != ")") throw std::runtime_error("Ожидался ')' после str");
        i++;

        if (std::holds_alternative<int>(arg))
            return std::to_string(std::get<int>(arg));
        if (std::holds_alternative<double>(arg))
            return std::to_string(std::get<double>(arg));
        if (std::holds_alternative<std::string>(arg))
            return arg;
        if (std::holds_alternative<bool>(arg))
        return std::get<bool>(arg) ? "true" : "false";

        throw std::runtime_error("Нельзя преобразовать в str");
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

        if (t.type == "KEYWORD" && (t.value == "let" || t.value == "burman"))
        {
            std::string varName = tokens[++i].value;

            if (tokens[++i].value != "=" && tokens[++i].value != "ass")
            {
                std::cout << "No '=' matching in varible!";
                return 0;
            }

            i++;
            Variant value = parseExpr(tokens, i);
            variables[varName] = value;
            result = value;
        }
        else if (t.type == "KEYWORD" && (t.value == "func" || t.value == "arslan"))
        {
            i++;
            std::vector<std::string> args;
            std::string funcName = tokens[i].value;
            i++;

            if (tokens[i].value != "(")
                throw std::runtime_error("No '(' in function declaration");
            i++;

            while (i < tokens.size() && tokens[i].value != ")")
            {
                if (tokens[i].type == "IDENT")
                {
                    args.push_back(tokens[i].value);
                    i++;
                }
                else
                {
                    throw std::runtime_error("Unexpected token in function args: " + tokens[i].value);
                }
            
                if (tokens[i].value == ",")
                    i++;
            }

            if (tokens[i].value != ")")
                throw std::runtime_error("No ')' in function declaration");
            i++;


            std::vector<Token> funcBody = parseBlock(tokens, i);

            funcTable[funcName] = {args, funcBody};
        }
        else if (t.type == "KEYWORD" && (t.value == "return" || t.value == "tuhum"))
        {
            i++;
            Variant retValue = parseExpr(tokens, i);
            throw ReturnException(retValue); 
        }
        else if (t.type == "KEYWORD" && (t.value == "print" || t.value == "dickpik"))
        {
            i++;
            Variant value = parseExpr(tokens, i);

            std::visit([](auto&& arg) 
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>)
                    std::cout << (arg ? "true" : "false") << std::endl;
                else
                    std::cout << arg << std::endl;
            }, value);
        }
        else if (t.type == "KEYWORD" && (t.value == "printn" || t.value == "cockpik"))
        {
            i++;
            Variant value = parseExpr(tokens, i);

            std::visit([](auto&& arg) 
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>)
                    std::cout << (arg ? "true" : "false");
                else
                    std::cout << arg;
            }, value);
        }
        else if (t.type == "KEYWORD" && (t.value == "input" || t.value == "fuck"))
        {
            i++;
            std::string line;
            std::getline(std::cin, line);
            result = convert(line);
        }
        else if (t.type == "KEYWORD" && (t.value == "if" || t.value == "bwc"))
        {
            i++;

            Variant condition = parseLogic(tokens, i);

            if (!std::holds_alternative<bool>(condition))
                throw std::runtime_error("Условие if должно быть булевым");

            std::vector<Token> blockTokens = parseBlock(tokens, i);
        

            if (std::get<bool>(condition))
                evaluate(blockTokens);
            else
            {
                if (i < tokens.size() && tokens[i].type == "KEYWORD" && (tokens[i].value == "else" || tokens[i].value == "bbc"))
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
        else if (t.type == "KEYWORD" && (t.value == "while" || t.value == "huy"))
        {
            i++;
            
            int startPos = i;
            Variant condition = parseLogic(tokens, i);

            if (!std::holds_alternative<bool>(condition))
                throw std::runtime_error("Условие while должно быть булевым");

            std::vector<Token> conditionBlock = parseBlock(tokens, i);

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
        else if (t.type == "KEYWORD" && (t.value == "int" || t.value == "pidor"))
        {
            i++;
            if (tokens[i].value != "(") throw std::runtime_error("Ожидался '(' после int");
            i++;
            Variant arg = parseExpr(tokens, i);
            if (tokens[i].value != ")") throw std::runtime_error("Ожидался ')' после int");
            i++;

            if (std::holds_alternative<std::string>(arg))
                return std::stoi(std::get<std::string>(arg));
            if (std::holds_alternative<double>(arg))
                return static_cast<int>(std::get<double>(arg));
            if (std::holds_alternative<int>(arg))
                return arg;
            if (std::holds_alternative<bool>(arg))
                return std::get<bool>(arg) ? 1 : 0;

            throw std::runtime_error("Нельзя преобразовать в int");
        }
        else if (t.type == "KEYWORD" && (t.value == "float" || t.value == "nefor"))
        {
            i++;
            if (tokens[i].value != "(") throw std::runtime_error("Ожидался '(' после float");
            i++;
            Variant arg = parseExpr(tokens, i);
            if (tokens[i].value != ")") throw std::runtime_error("Ожидался ')' после float");
            i++;

            if (std::holds_alternative<std::string>(arg))
                return std::stod(std::get<std::string>(arg));
            if (std::holds_alternative<int>(arg))
                return static_cast<double>(std::get<int>(arg));
            if (std::holds_alternative<double>(arg))
                return arg;
            if (std::holds_alternative<bool>(arg))
                return std::get<bool>(arg) ? 1 : 0;

            throw std::runtime_error("Нельзя преобразовать в float");
        }
        else if (t.type == "KEYWORD" && (t.value == "str" || t.value == "muzick"))
        {
            i++;
            if (tokens[i].value != "(") throw std::runtime_error("Ожидался '(' после str");
            i++;
            Variant arg = parseExpr(tokens, i);
            if (tokens[i].value != ")") throw std::runtime_error("Ожидался ')' после str");
            i++;

            if (std::holds_alternative<int>(arg))
                return std::to_string(std::get<int>(arg));
            if (std::holds_alternative<double>(arg))
                return std::to_string(std::get<double>(arg));
            if (std::holds_alternative<std::string>(arg))
                return arg;

            throw std::runtime_error("Нельзя преобразовать в str");
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