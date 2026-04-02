#include "tests_cmd.h"


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "aloe/logger.h"
#include "aloe/parser.h"
#include "aloe/compiler.h"
#include "compile_cmd.h"


using namespace aloe;
using namespace std;

#define TEST_PARSE_STRING(CMD,E) { printf("TEST PARSE STRING \"%-50s\"",CMD);  parse_string(CMD) == E ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

#define TEST_PARSE_FILE(CMD,E)   { printf("TEST PARSE FILE \"%-50s\"", CMD);   parse_file(CMD) == true ? printf("...[OK]\n") : printf("...[FAIL]\n"); }

#define TEST_COMPILE_FILE(CMD,E) { printf("TEST COMPILE FILE \"%-50s\"", CMD); compile_cmd_t().compile_cmd(CMD,nullptr) == true ? printf("...[OK]\n") : printf("...[FAIL]\n"); }



bool 
tests_cmd_t::parse_file(const char* al)
{
    auto p = create_parser();

    ast_ptr_t ast;

	ifstream ifs(al);

    return p->parse_from_stream(ifs, ast, al);
}

bool 
tests_cmd_t::parse_string(const char* al)
{
    auto p = create_parser();

    ast_ptr_t ast;

	istringstream iss(string("al"));

    return p->parse_from_stream(iss,  ast, al);
}

void 
tests_cmd_t::parse_file_tests()
{
    TEST_PARSE_FILE("leaf.al", true);
}

void 
tests_cmd_t::compile_file_tests()
{
    //TEST_COMPILE_FILE("leaf.al", true);
}

void
tests_cmd_t::test_var_declarations1()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"(var b:A)", false);
    TEST_PARSE_STRING(R"(var a:int)", true);
    TEST_PARSE_STRING(R"(var :int)", true);
    TEST_PARSE_STRING(R"(var :^^^^^int)", true);
}




void
tests_cmd_t::test_object_declarations1()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"( object B (); object A > B ())", true);
    TEST_PARSE_STRING(R"( object B (); object A > ^B ())", true);
    TEST_PARSE_STRING(R"( object A > C ())", false);
    TEST_PARSE_STRING(R"( object A > ^C ())", false);
    TEST_PARSE_STRING(R"( object A > int ())", false);
}

void
tests_cmd_t::test_fun_declarations1()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"( fun foo : int (var x:int, var y:int) {})", true);
    TEST_PARSE_STRING(R"( fun foo : int (var x:A, var y:int) {})", false);
    TEST_PARSE_STRING(R"( object A () ; fun foo : int (var x:A, var y:int) {})", true);
    TEST_PARSE_STRING(R"( object A () ; fun foo : B (var x:A, var y:int) {})", false);
    TEST_PARSE_STRING(R"( object A () ; fun foo : A (var x:A, var y:int) {})", true);

    TEST_PARSE_STRING(R"( fun foo:void() { foo(); })", true);
    TEST_PARSE_STRING(R"( fun foo:void() { fun:int(){}();})", true);

    

}

void
tests_cmd_t::test_fun_expect1()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"(expect fun foo:void())", true);
    TEST_PARSE_STRING(R"(expect fun foo:void() {})", false);

}



void 
tests_cmd_t::test_expressions1()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"( object A(); fun foo:void(var a:A) { a;})", true);
    TEST_PARSE_STRING(R"( object A(); fun foo:void() { "a";})", true);
    TEST_PARSE_STRING(R"( object A(); fun foo:void() { '\n';})", true);
    TEST_PARSE_STRING(R"( object A(); fun foo:void() { 12345;})", true);
    TEST_PARSE_STRING(R"( object A(); fun foo:void() { (12345);})", true);
    TEST_PARSE_STRING(R"( object A(); fun foo:void(var a:A) { a++;})", true);

}


void
tests_cmd_t::run_tests()
{
    TEST_PARSE_STRING(R"(var :^^^^^int)", true);

    return;

    test_fun_expect1();
    test_var_declarations1();
    test_object_declarations1();
    test_fun_declarations1();
    test_expressions1();
}
