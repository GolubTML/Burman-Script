#include "../headers/token.h"

std::vector<Token> tokenize(const std::string& src)
{
    std::vector<Token> tokens;
    for (int i = 0; i < src.size(); ++i)
    {
        char c = src[i];

        if (isspace(c)) continue;

        if (isdigit(c))
        {
            std::string num;
            bool isFloat = false;

            while (i < src.size() && (isdigit(src[i]) || src[i] == '.'))
            {
                if (src[i] == '.') isFloat = true;
                num += src[i];
                i++;
            }
            --i;
            if (isFloat) tokens.push_back({"FLOAT", num});
            else tokens.push_back({"DIGIT", num});
        }
        else if (isalpha(c))
        {
            std::string ident;

            while (i < src.size() && isalnum(src[i]))
            {
                ident += src[i];
                i++;
            }
            --i;

            if (ident == "let" || ident == "print" || ident == "if" || ident == "else" || ident == "while" || ident == "func" || ident == "return")
                tokens.push_back({"KEYWORD", ident});
            else if (ident == "true" || ident == "false")
                tokens.push_back({"BOOL", ident});
            else
                tokens.push_back({"IDENT", ident});
        }
        else if (c == '"')
        {
            std::string str;
            i++;
            while (i < src.size() && src[i] != '"')
                str += src[i++];

            tokens.push_back({"STRING", str});
        }
        else if (c == '+' && i + 1 < src.size() && src[i + 1] == '+')
        {
            tokens.push_back({"OPERATOR", "++"});
            i++;
        }
        else if (c == '-' && i + 1 < src.size() && src[i + 1] == '-')
        {
            tokens.push_back({"OPERATOR", "--"});
            i++;
        }
        else if (c == '+' || c == '-' || c == '/' || c == '*')
        {
            tokens.push_back({"OPERATOR", std::string(1, c)});
        }
        else if (c == '!' || c == '=' || c == '>' || c == '<')
        {
            std::string op(1, c);

            if (i + 1 < src.size() && src[i + 1] == '=')
            {
                op += '=';
                i++;
            }

            tokens.push_back({"OPERATOR", op});
        }
        else if ((c == '&' && i + 1 < src.size() && src[i + 1] == '&') || (c == '|' && i + 1 < src.size() && src[i + 1] == '|'))
        {
            std::string op;
            op += c;
            op += src[i + 1];
            tokens.push_back({"OPERATOR", op});
            i++;
        }

        else
        {
            tokens.push_back({"SYMBOL", std::string(1, c)});
        }
    }


    return tokens;
}