#include "pch.h"
#include "aloe/parser.h"

using namespace aloe;
using namespace std;
using namespace antlr4;

bool
parser_t::parse_from_file(const string& file_name, ast_t** ast_tree)
{
    std::ifstream stream;
    stream.open(file_name, std::ifstream::in);
    
    if (!stream.is_open())
    {
        logi("fatal: Cannot open file:%s\n", file_name.c_str());
        return false;
   }


    ANTLRInputStream input(stream);
    aloeLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    aloeParser parser(&tokens);

    parser.addErrorListener(this);

    tree::ParseTree* tree = parser.prog();

    return parser.getNumberOfSyntaxErrors() == 0;
	
}


void 
parser_t::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
	size_t line, size_t charPositionInLine, const std::string& msg,
	std::exception_ptr e) {
	logi("Syntax Error at line %d:%d - %s\n",  line , charPositionInLine, msg.c_str());
}