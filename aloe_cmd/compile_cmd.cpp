#include "compile_cmd.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "aloe/logger.h"
#include "aloe/parser.h"
#include "aloe/compiler.h"

using namespace aloe;

bool
compile_cmd_t::compile_file(const char* in_filename,  const char* out_filename)
{
    auto p = create_parser();

    ast_ptr_t ast;

    if (!p->parse_from_file(in_filename, ast))
        return false;

    auto c = create_compiler();
    string ir_str;

    std::filesystem::path pth(in_filename);

    if (!c->compile(ast, ir_str))
        return false;

    std::ofstream out(out_filename);
    if (!out) {
        // handle error
        return 1;
    }

    out << ir_str;
    out.close(); // optional; RAII handles it

    return true;

}
