// aloe_cmd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "aloe/logger.h"
#include "aloe/parser.h"
#include "aloe/utils.h"

using namespace aloe;

void aloe::log1(FILE* stream, const char* text)
{
    fprintf(stream, text);
}

void aloe::log1nl(FILE* stream)
{
    fprintf(stream, "\n");
}

#define TEST_PARSE(CMD,E) { printf("TEST \"%-50s\"",CMD);   p->parse_from_string(CMD) == E ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

int main()
{
    auto p = create_parser();

    TEST_PARSE(R"( object B {}; object A > B {})", true);
    TEST_PARSE(R"( object B {}; object A > ^B {})", true);
    TEST_PARSE(R"( object A > C {})",false);
    TEST_PARSE(R"( object A > ^C {})", false);
    TEST_PARSE(R"( object A > int {})", false);

    TEST_PARSE(R"( fun foo : int (var x:int, var y:int) {})", true);
    TEST_PARSE(R"( fun foo : int (var x:A, var y:int) {})",   false);
    
   
    release_parser(p);
    
}

