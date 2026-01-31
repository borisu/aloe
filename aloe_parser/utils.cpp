#include "pch.h"
#include "utils.h"

using namespace std;
using namespace aloe;


// https://stackoverflow.com/questions/5612182/convert-string-with-explicit-escape-sequence-into-relative-character

string aloe::unescape(const string& s)
{
    string res;
    string::const_iterator it = s.begin();
    while (it != s.end())
    {
        char c = *it++;
        if (c == '\\' && it != s.end())
        {
            switch (*it++) {
            case '\\': c = '\\'; break;
            case '\'': c = '\''; break;
            case '\"': c = '\"'; break;
            case 'a': c = '\a'; break;
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case 'v': c = '\v'; break;
            case '0': c = '\0'; break;
                // all other escapes
            default:
                // invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
                continue;
            }
        }
        res += c;
    }

    return res;
}