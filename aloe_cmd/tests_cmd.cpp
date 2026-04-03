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

#define TEST_PARSE_STRING(CMD,E) {                                                      \
    printf("TEST PARSE STRING \"%-50s\"",CMD);                                          \
    bool test_success = parse_string(CMD) == E;                                         \
    test_success ? printf("...[OK]\n") : printf("...[FAIL]\n");                         \
    success &= test_success ;                                                           \
}



bool 
tests_cmd_t::parse_string(const char* al)
{
    auto p = create_parser();

    ast_ptr_t ast;

	istringstream iss(al);

    return p->parse_from_stream(iss,  ast, al);
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
tests_cmd_t::test_fun_declarations1()
{
    auto p = create_parser();
    
    TEST_PARSE_STRING(R"(fun foo:()-> void { fun xxx:()->int {}; })", true);

}

void
tests_cmd_t::test_fun_expect1()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"(expect fun foo:() -> void)", true);
    TEST_PARSE_STRING(R"(expect fun foo:() -> void {})", false);

}

void 
tests_cmd_t::test_expressions1()
{
    auto p = create_parser();

    TEST_PARSE_STRING(R"( fun foo:() -> void { "a";})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { '\n';})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { 12345;})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { (12345);})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { foo(); })", true);

}


bool
tests_cmd_t::run_tests()
{
    success = true;

    test_fun_expect1();
    test_var_declarations1();
    test_fun_declarations1();
    test_expressions1();

    return success;
}
