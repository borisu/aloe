#include "tests_cmd.h"


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "base/logger.h"
#include "antlr4_parser/parser.h"
#include "llvm_compiler/compiler.h"
#include "compile_cmd.h"


using namespace aloe;
using namespace std;

#define TEST_PARSE_STRING(CMD,E)   run_test(__FUNCTION__, CMD, E)      

void tests_cmd_t::set_validate(bool validate)
{
	this->validate = validate;
}

void tests_cmd_t::set_dump_ir(bool dump_ir)
{
	this->dump_ir = dump_ir;
}

void tests_cmd_t::set_no_debug(bool no_debug)
{
	this->no_debug = no_debug;
}

void tests_cmd_t::set_compile(bool compile)
{
	this->compile = compile;
}

void 
tests_cmd_t::run_test(const char *test_name, const char* al, bool expected)
{
    printf("TEST \n%-20s \n", test_name);

    bool run_res = false;
    bool test_res = false;

    try {

        auto p = create_antlr4_parser();

        auto c = create_llvm_compiler();

        c->set_validate(validate);

        c->set_no_debug(no_debug);

        ast_ptr_t ast;

        istringstream iss(al);

        if (run_res = p->parse_from_stream(iss, ast, "<string>"))
        {
            if (expected && compile)
            {
                stringstream ss;
                run_res = c->compile(ast, ss);
				if (dump_ir)
				{
					printf("\nIR:\n%s\n", ss.str().c_str());
				}
            }
        }
        
        test_res = run_res == expected;
        
    }
	catch (std::exception* ex)
	{
        printf("exception:%s\n", ex->what());
		test_res = false; // never pass the test if an exception is thrown, even if the test expected a failure
	}

    if (!test_res)
	{
		printf("\n%s\n", al);
	}
    
    test_res ? printf("\n...[OK]\n") : printf("\n...[FAIL]\n");
    

	success = success && test_res;
}

void
tests_cmd_t::test_postfix()
{
	TEST_PARSE_STRING(R"(fun root:() -> int { var a : int = 1; return a++; })", true);
}

void
tests_cmd_t::test_prefix()
{
    TEST_PARSE_STRING(R"(fun root:() -> int { var a : int = 1; return ++a; })", true);
}

void
tests_cmd_t::test_var_declarations1()
{
    TEST_PARSE_STRING(R"(var b:A)", false);
    TEST_PARSE_STRING(R"(var a:int)", true);
    TEST_PARSE_STRING(R"(var :int)", false);
    TEST_PARSE_STRING(R"(var a:^^^^^int)", true);
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
    TEST_PARSE_STRING(R"(fun foo:() -> void { foo(); })", true);
    TEST_PARSE_STRING(R"(fun foo:(var a:int)-> void { foo(a); })", true);
    TEST_PARSE_STRING(R"(fun foo:(var a:int)-> void { foo(); })", false);
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

void 
tests_cmd_t::test_mutable_parameters()
{
    TEST_PARSE_STRING(R"( fun foo:(var a:int) -> void { ++a;})", true);
}

void 
tests_cmd_t::test_pointers_cast()
{
    TEST_PARSE_STRING(R"( 
        fun foo:() -> void 
        {
            var a:^int   = ^0
        })",
        false);

    TEST_PARSE_STRING(R"( 
        fun foo:() -> void 
        {
            var a:^int   = 0:^int
            var b:^^int  = 0:^^int  
            var c:^^^int = 0:^^^int
        })",
        true);
}

void 
tests_cmd_t::test_pointers()
{
    TEST_PARSE_STRING(R"( 
        fun foo:() -> void 
        {
            var a:^int
            var b:^^int  = ^a
        })",
        true);
    
}

void 
tests_cmd_t::test_deref()
{

    TEST_PARSE_STRING(R"( 
        fun foo:() -> void 
        {
            var a:int   = 0
            var b:^int  = ^a
            var c:int = @b + 1 
        })",

        true);
}


void tests_cmd_t::test_array_index()
{
    TEST_PARSE_STRING(R"( 
        fun root:() -> int {
            var a:int[10];
            a[5] = 1;
            return a[6];
        })",
        true);
}


void tests_cmd_t::test_layout_type()
{

    TEST_PARSE_STRING(R"( 
        layout {};
        )",
        false);

    TEST_PARSE_STRING(R"( 
        layout A { a:int };
        )",
        true);

    TEST_PARSE_STRING(R"( 
        layout A { 
        a:int 
        b:int
        };
        )",
        true);

    TEST_PARSE_STRING(R"( 
        layout A { 
        a:int 
        b:int ;
        b:int
        };
        )",
        true);

    TEST_PARSE_STRING(R"( 
        layout A { 
        a:layout { c:int  d:int } 
        b:int ;
        b:int
        };
        )",
        true);

    TEST_PARSE_STRING(R"( 
        layout B {b:int}

        layout A > B { 
        a:layout { c:int  d:int } 
        b:int ;
        b:int
        };
        )",
        true);

    TEST_PARSE_STRING(R"( 
        layout B {b:int}

        layout A > B > C { 
        a:layout { c:int  d:int } 
        b:int ;
        b:int
        };
        )",
        false);

    TEST_PARSE_STRING(R"( 
        layout B {b:int}    
        layout C {b:int}    
        layout D {b:int}    

        layout A > B > C > layout { a: int } > D { 
        a:layout { c:int  d:int } 
        b:int ;
        b:int
        };
        )",
        true);
}

bool
tests_cmd_t::run_tests()
{
    success = true;

    if (true)
    {
		compile = false;

		validate = false;
        dump_ir = true;
        no_debug = true;

        test_layout_type();

        return true;

    }
    

    test_var_scope();
    test_funcall_type_mismatch();
    test_return_type_mismatch();
    test_fun_expect1();
    test_var_declarations1();
    test_fun_declarations1();
    test_expressions1();
    test_defaults();
    test_recursion();
    test_postfix();
    test_prefix();
    test_mutable_parameters();
    test_pointers();
    test_deref();
    test_array_index();

   
    
    return success;
}
