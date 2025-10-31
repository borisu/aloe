#include "pch.h"
#include "antlr4_parser.h"
#include "D:\Projects\aloe\aloe_antlr\gengen\aloeParser.h"

using namespace aloe;
using namespace std;
using namespace antlr4;

parser_t* 
aloe::create_parser()
{
    return new antl4_parser_t();
}

void
aloe::release_parser(parser_t* p)
{
    delete p;
}

void 
antl4_parser_t::release_ast(ast_t* ast)
{
    delete ast;
}


bool 
antl4_parser_t::parse_from_string(const string& str, ast_t** ast_tree)
{
    std::istringstream stream(str);
    
    return parse_from_stream(stream, ast_tree);
}

bool 
antl4_parser_t::parse_from_stream(istream& stream, ast_t** ast_tree)
{
    try {
       
        ANTLRInputStream input(stream);
        aloeLexer lexer(&input);
        CommonTokenStream tokens(&lexer);

        aloeParser parser(&tokens);
        parser.addErrorListener(this);

        tree::ParseTreeWalker walker;
        walker.walk(this, parser.prog());

        return parser.getNumberOfSyntaxErrors() == 0;
    }
    catch (std::exception& e)
    {
        logi("Error: %s", e.what());
    }

    return false;
}

bool
antl4_parser_t::parse_from_file(const string& file_name, ast_t** ast_tree)
{
    try {
        std::ifstream stream;
        stream.open(file_name, std::ifstream::in);

        if (!stream.is_open())
        {
            logi("fatal: Cannot open file:%s\n", file_name.c_str());
            return false;
        }

        return parse_from_stream(stream, ast_tree);
        
    }
    catch (std::exception& e)
    {
        logi("Error: %s", e.what());
    }

    return false;

}


void
antl4_parser_t::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
    size_t line, size_t charPositionInLine, const std::string& msg,
    std::exception_ptr e) {
    logi("Syntax Error at line %d:%d - %s\n", line, charPositionInLine, msg.c_str());
}

void
antl4_parser_t::enterObjectDeclaration(aloeParser::ObjectDeclarationContext* ctx)
{
     string s = ctx->identifier()->getText();
}

void
antl4_parser_t::exitObjectDeclaration(aloeParser::ObjectDeclarationContext* ctx)
{
    
}