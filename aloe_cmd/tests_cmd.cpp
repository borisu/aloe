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
    printf("TEST PARSE STRING -- \n%-20s \n",CMD);                                          \
    bool test_success = parse_string(CMD) == E;                                         \
    test_success ? printf("\n...[OK]\n") : printf("\n...[FAIL]\n");                         \
    success &= test_success ;                                                           \
}


bool 
tests_cmd_t::parse_string(const char* al)
{
    auto p = create_parser();

    ast_ptr_t ast;

	istringstream iss(al);

    return p->parse_from_stream(iss, ast, "<string>");
}


void
tests_cmd_t::test_var_declarations1()
{
    TEST_PARSE_STRING(R"(var b:A)", false);
    TEST_PARSE_STRING(R"(var a:int)", true);
    TEST_PARSE_STRING(R"(var :int)", true);
    TEST_PARSE_STRING(R"(var :^^^^^int)", true);
    TEST_PARSE_STRING(R"(var b:int =  1 + 1)", false);
    TEST_PARSE_STRING(R"(var b:int = 1)", true);

    TEST_PARSE_STRING(R"(fun foo:() -> void { var b:int = 1 + 1 } )", true);
    TEST_PARSE_STRING(R"(fun foo:() -> void { var b:int = 1 } )", true);

    TEST_PARSE_STRING(R"( fun foo:(var a:int = 1) -> void { "a";})", false);
}


void
tests_cmd_t::test_fun_declarations1()
{
    
    TEST_PARSE_STRING(R"(fun foo:()-> void { fun xxx:()->int {}; })", true);
    TEST_PARSE_STRING(R"(fun foo:()-> void { fun :()->int {}; })", true);

}
void tests_cmd_t::test_var_scope()
{
    TEST_PARSE_STRING(R"(
      var a:char  = 'a';
      var a:int   = 10;)", false);

    TEST_PARSE_STRING(R"(
      var a:int = 10;
      fun bar :(var a:int)-> void { })", true);

    TEST_PARSE_STRING(R"(
      fun bar :(var a:int)-> void {var a:int })", false);
}


void tests_cmd_t::test_funcall_type_mismatch()
{
    TEST_PARSE_STRING(R"(
      fun foo:(var:int)-> void {  }
      fun bar :()-> void { foo (10);})", true);

    TEST_PARSE_STRING(R"(
      fun foo:(var:int)-> void {  }
      fun bar :()-> void { foo ('a');})", false);

    TEST_PARSE_STRING(R"(
      fun foo:(var:int, var:char)-> void {  }
      fun bar :()-> void { foo (1,1);})", false);
    
}

void tests_cmd_t::test_return_type_mismatch()
{
    TEST_PARSE_STRING(R"(fun foo:()-> int { return; })", false);
    TEST_PARSE_STRING(R"(fun foo:()-> void { return 10; })", false);
    TEST_PARSE_STRING(R"(fun foo:()-> void { return (10); })", false);
    TEST_PARSE_STRING(R"(fun foo:()-> void { var a:char = 'a'; return a; })", false);
    TEST_PARSE_STRING(R"(fun foo:()-> void { var a:int = 10; return a++; })", false);
    TEST_PARSE_STRING(R"(fun foo:()-> void { var a:int = 10; return a--; })", false);
    
}


void 
tests_cmd_t::test_defaults()
{
    TEST_PARSE_STRING(R"(var a:void     =   0)", false);
    TEST_PARSE_STRING(R"(var a:int     =   0)", true);
    TEST_PARSE_STRING(R"(var a:^void   =  ^0)", true);
    TEST_PARSE_STRING(R"(var a:^^void  = ^^0)", true);
    TEST_PARSE_STRING(R"(var a:^char   = "ok")", true);
    TEST_PARSE_STRING(R"(var a:char    = 'o')", true);
    TEST_PARSE_STRING(R"(var a:char    = 123)", false);
}
    

void
tests_cmd_t::test_fun_expect1()
{

    TEST_PARSE_STRING(R"(expect fun foo:() -> void)", true);
    TEST_PARSE_STRING(R"(expect fun foo:() -> void {})", false);
    TEST_PARSE_STRING(R"(
        expect fun foo:()-> void; 
        fun foo:()-> void { }
    )", true);

    TEST_PARSE_STRING(R"(
            fun foo:()-> void { }
            expect fun foo:()-> void; 
    )", true);

    TEST_PARSE_STRING(R"(
        expect fun foo:()-> void; 
        fun foo:()-> int { }
    )", false);

    TEST_PARSE_STRING(R"(
            fun foo:()-> int { }
            expect fun foo:()-> void; 
    )", false);

}

void 
tests_cmd_t::test_recursion()
{
    TEST_PARSE_STRING(R"( fun foo:() -> void { foo(); })", true);
    TEST_PARSE_STRING(R"(fun foo:(var a:int)-> void { foo(a); })", true);
    TEST_PARSE_STRING(R"(fun foo:(var a:int)-> void { foo(); })", false);
}

void 
tests_cmd_t::test_ref_deref()
{
    
    
}

void 
tests_cmd_t::test_expressions1()
{

    TEST_PARSE_STRING(R"( fun foo:() -> void { "a";})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { '\n';})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { 12345;})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { (12345);})", true);
    TEST_PARSE_STRING(R"( fun foo:() -> void { }; fun bar:()-> void { foo(); })", true);
    

}


bool
tests_cmd_t::run_tests()
{
    success = true;

    

    test_var_scope();
    test_funcall_type_mismatch();
    test_return_type_mismatch();
    test_fun_expect1();
    test_var_declarations1();
    test_fun_declarations1();
    test_expressions1();
    test_defaults();
    test_recursion();
	
    
    return success;
}
