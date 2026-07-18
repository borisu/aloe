#include "compile_cmd.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "base/logger.h"
#include "llvm_compiler/compiler.h"
#include "antlr4_parser/parser.h"


using namespace aloe;
using namespace std;


bool
compile_cmd_t::compile_cmd(istream& is, ostream& os, const string& source_id,bool no_debug )
{
    auto p = create_antlr4_parser();

    ast_ptr_t ast;

    if (!p->parse_from_stream(is, ast, source_id))
    {
        return false;
    }

    auto c = create_llvm_compiler();
	c->set_no_debug(no_debug);
    
    if (!c->compile(ast, os))
    {
        return false;
    }
    
    return true;

}
