// aloe_cmd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <fstream>
#include <string>

#include "aloe/logger.h"
#include "aloe/parser.h"
#include "aloe/compiler.h"
#include "tests_cmd.h"
#include "compile_cmd.h"


using namespace aloe;

char G_NL[1] = { '\n' };

void aloe::log1(FILE* stream, const char* text)
{
    fprintf(stream, text);
}

void aloe::log1nl(FILE* stream)
{
    fprintf(stream, G_NL);
}

enum ALOE_CMD_MODE
{
    MODE_UNKNOWN,
    MODE_COMPILE,
    MODE_TEST
};
    

int main(int argc, char* argv[])
{
    std::string input_file;
    std::string output_file;
    std::string output_dir;
    bool verbose = false;

    auto mode = MODE_UNKNOWN;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-c" && i + 1 < argc) {
            if (mode != MODE_UNKNOWN)
            {
                std::cerr << "Options error: multiple modes set.";
                exit(1);
            }
            mode = MODE_COMPILE;
            input_file = argv[++i]; // consume next argument
        }
        else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i]; // consume next argument
        }
        else if (arg == "-t") {

            G_NL[0] = ';';

            if (mode != MODE_UNKNOWN)
            {
                std::cerr << "Options error: multiple modes set.";
                exit(1);
            }
            mode = MODE_TEST;
            break;
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
        }
    }

    auto err_code = 1;

    switch (mode){
    case MODE_UNKNOWN:
    {
        std::cerr << "Please set mode of operation";
        break;
    }
    case MODE_COMPILE:
    {
        err_code =  compile_cmd_t().compile_file(input_file.c_str(), output_file.c_str()) ? 0 : 1;
        break;
    }
    case MODE_TEST:
    {
        err_code = 0;
        tests_cmd_t().run_tests();
    }
    }


    return err_code;
}



