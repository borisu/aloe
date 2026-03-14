#include "compile_cmd.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "aloe/logger.h"
#include "aloe/parser.h"
#include "aloe/compiler.h"

using namespace aloe;
using namespace std;


bool
compile_cmd_t::compile_cmd(istream& is, ostream& os, const string& source_id)
{
    auto p = create_parser();

    ast_ptr_t ast;

    if (!p->parse_from_stream(is, ast, source_id))
    {
        return false;
    }

    auto c = create_compiler();
    
    if (!c->compile(ast, os))
    {
        return false;
    }
    
    return true;

}
