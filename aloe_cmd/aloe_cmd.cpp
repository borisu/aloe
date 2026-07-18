// aloe_cmd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <fstream>
#include <string>

#include "base/logger.h"
#include "lang/parser.h"
#include "lang/compiler.h"
#include "tests_cmd.h"
#include "compile_cmd.h"


using namespace aloe;

char G_NL[1] = { ';' };

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

    bool verbose = false;
	bool validate_ir = true;
	bool dump_ir = false;
	bool no_debug = false;

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
        }
        else if (arg == "--no-validate") {
            validate_ir = false;
        }
        else if (arg == "--dump-ir") {
            dump_ir = true;
        }
        else if (arg == "--no-debug") {
            no_debug = true;
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
        std::ifstream ifs;
        std::istream* in;

        if (input_file.empty()) {
            in = &std::cin;
        }
        else {
            ifs.open(input_file);
            in = &ifs;
        }

        std::ofstream ofs;
        std::ostream* out;

        if (output_file.empty()) {
            out = &std::cout;
        }
        else {
            ofs.open(output_file);
            out = &ofs;
        }

       err_code = compile_cmd_t().compile_cmd(
             *in,
			 *out,
             input_file.empty () ? "<source>" : input_file,
           no_debug) ? 0 : 1;

        break;
    }
    case MODE_TEST:
    {
		tests_cmd_t test_runner;
		test_runner.set_validate(validate_ir);
        test_runner.set_dump_ir(dump_ir);
        test_runner.set_no_debug(no_debug);

        err_code = tests_cmd_t().run_tests() ? 0 : 1;
        break;
    }
    }


    return err_code;
}



