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
    fprintf(stream, ";");
}

#define TEST_PARSE(CMD,E) { printf("TEST \"%-50s\"",CMD);   p->parse_from_string(CMD) == E ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

#define TEST_EXEC(CMD,E) { printf("EXEC \"%-50s\"", CMD);  exec(CMD) == true ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

#define TEST_FILE(CMD,E) { printf("RUN \"%-50s\"", CMD);   run_file(CMD) == true ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

bool exec(const char *al)
{
    auto p = create_parser();

    if (!p->parse_from_string(al))
        return false;

    release_parser(p);

    return true;
}

bool run_file(const char* al)
{
    auto p = create_parser();

    if (!p->parse_from_file(al))
        return false;

    release_parser(p);

    return true;
}

void file_tests()
{
    TEST_FILE("leaf.al", true);
}

void parse_tests()
{
    auto p = create_parser();

    TEST_PARSE(R"(var b:A)", false);

    TEST_PARSE(R"(var a:int)", true);

    TEST_PARSE(R"(var :int)", true);

    TEST_PARSE(R"( object B (); object A > B ())", true);
    TEST_PARSE(R"( object B (); object A > ^B ())", true);
    TEST_PARSE(R"( object A > C ())", false);
    TEST_PARSE(R"( object A > ^C ())", false);
    TEST_PARSE(R"( object A > int ())", false);

    TEST_PARSE(R"( fun foo : int (var x:int, var y:int) {})", true);
    TEST_PARSE(R"( fun foo : int (var x:A, var y:int) {})", false);
    TEST_PARSE(R"( object A () ; fun foo : int (var x:A, var y:int) {})", true);
    TEST_PARSE(R"( object A () ; fun foo : B (var x:A, var y:int) {})", false);
    TEST_PARSE(R"( object A () ; fun foo : A (var x:A, var y:int) {})", true);

    TEST_PARSE(R"( fun foo:void() { foo(); })", true);
    TEST_PARSE(R"( fun foo:void() { fun:int(){}();})", true);


    TEST_EXEC(R"( fun main:void() { __print("ok"); })", true);

    release_parser(p);
}

int main()
{

    file_tests();
    
}



