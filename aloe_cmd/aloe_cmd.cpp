// aloe_cmd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>

#include "aloe/logger.h"
#include "aloe/parser.h"
#include "aloe/compiler.h"


using namespace aloe;

void aloe::log1(FILE* stream, const char* text)
{
    fprintf(stream, text);
}

void aloe::log1nl(FILE* stream)
{
    fprintf(stream, ";");
}

#define TEST_PARSE_STRING(CMD,E) { printf("TEST PARSE STRING \"%-50s\"",CMD);  parse_string(CMD) == E ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

#define TEST_PARSE_FILE(CMD,E)   { printf("TEST PARSE FILE \"%-50s\"", CMD);   parse_file(CMD) == true ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

#define TEST_COMPILE_FILE(CMD,E) { printf("TEST COMPILE FILE \"%-50s\"", CMD); compile_file(CMD) == true ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

bool compile_file(const char *al)
{
    auto p = create_parser();

    ast_ptr_t ast;

    if (!p->parse_from_file(al,ast))
        return false;

    auto c = create_compiler();
    return c->compile(ast);
    
}

bool parse_file(const char* al)
{
    auto p = create_parser();

    ast_ptr_t ast;

    return p->parse_from_file(al, ast);
}

bool parse_string(const char* al)
{
    auto p = create_parser();

    ast_ptr_t ast;

    return p->parse_from_string(al, ast);
}

void parse_file_tests()
{
    TEST_PARSE_FILE("leaf.al", true);
}

void compile_file_tests()
{
    TEST_PARSE_FILE("leaf.al", true);
}

void parse_string_tests()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"(var b:A)", false);
    TEST_PARSE_STRING(R"(var a:int)", true);
    TEST_PARSE_STRING(R"(var :int)", true);

    TEST_PARSE_STRING(R"( object B (); object A > B ())", true);
    TEST_PARSE_STRING(R"( object B (); object A > ^B ())", true);
    TEST_PARSE_STRING(R"( object A > C ())", false);
    TEST_PARSE_STRING(R"( object A > ^C ())", false);
    TEST_PARSE_STRING(R"( object A > int ())", false);

    TEST_PARSE_STRING(R"( fun foo : int (var x:int, var y:int) {})", true);
    TEST_PARSE_STRING(R"( fun foo : int (var x:A, var y:int) {})", false);
    TEST_PARSE_STRING(R"( object A () ; fun foo : int (var x:A, var y:int) {})", true);
    TEST_PARSE_STRING(R"( object A () ; fun foo : B (var x:A, var y:int) {})", false);
    TEST_PARSE_STRING(R"( object A () ; fun foo : A (var x:A, var y:int) {})", true);

    TEST_PARSE_STRING(R"( fun foo:void() { foo(); })", true);
    TEST_PARSE_STRING(R"( fun foo:void() { fun:int(){}();})", true);

    
}

int main()
{
    compile_file("leaf.al");
}



