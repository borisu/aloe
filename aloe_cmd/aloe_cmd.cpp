// aloe_cmd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>


#include "aloe/logger.h"
#include "aloe/parser.h"

using namespace aloe;

void aloe::log1(FILE* stream, const char* text)
{
    fprintf(stream, text);
}


int main()
{

    auto p = create_parser();

    ast_t* ast = nullptr;

    p->parse_from_string(R"( object A {} )", &ast);

    release_parser(p);
    
}

