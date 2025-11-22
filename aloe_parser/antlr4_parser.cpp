#include "pch.h"
#include "antlr4_parser.h"
#include "aloe/utils.h"


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

void 
antl4_parser_t::release_node(node_t* node)
{
    
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
    bool success = false;
    try {
       
        ANTLRInputStream input(stream);
        aloeLexer lexer(&input);
        CommonTokenStream tokens(&lexer);

        aloeParser parser(&tokens);
        parser.addErrorListener(this);

        ast_       = new ast_t();
        s_.push(ast_);
        
        
        tree::ParseTreeWalker walker;
        walker.walk(this, parser.prog());

        success = parser.getNumberOfSyntaxErrors() == 0;

    }
    catch (std::exception& e)
    {
        logi("Error: %s", e.what());
    }

    if (!success && ast_)
        release_ast(ast_);
    else
        *ast_tree = ast_;

    return success;
}

bool
antl4_parser_t::parse_from_file(const string& file_name, ast_t** ast_tree)
{
    try {
        std::ifstream stream;
        stream.open(file_name, std::ifstream::in);

        if (!stream.is_open())
        {
            logi("error: cannot open file:%s\n", file_name.c_str());
            return false;
        }

        return parse_from_stream(stream, ast_tree);
        
    }
    catch (std::exception& e)
    {
        logi("unexpected error: %s", e.what());
    }

    return false;

}

void
antl4_parser_t::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
    size_t line, size_t charPositionInLine, const std::string& msg,
    std::exception_ptr e) {
    logi("syntax Error at line %d:%d - %s\n", line, charPositionInLine, msg.c_str());
}

void
antl4_parser_t::enterObjectDeclaration(aloeParser::ObjectDeclarationContext* ctx)
{
    auto curr_node = s_.top();

    object_node_t* node(new object_node_t());
    curr_node->add_object_node(node);
    
    s_.push(node);
}

void 
antl4_parser_t::exitObjectDeclaration(aloeParser::ObjectDeclarationContext* ctx)
{
    s_.pop();
}

void 
antl4_parser_t::enterInheritanceChain(aloeParser::InheritanceChainContext* /*ctx*/)
{
    auto curr_node = s_.top();

    inheritance_chain_node_t* node(new inheritance_chain_node_t());
    curr_node->add_inheritance_chain_node(node);

    s_.push(node);
}

void 
antl4_parser_t::exitInheritanceChain(aloeParser::InheritanceChainContext* /*ctx*/)
{
    s_.pop();
}

void 
antl4_parser_t::enterIdentifier(aloeParser::IdentifierContext* ctx) 
{
    auto curr_node = s_.top();

    identifier_node_t* node(new identifier_node_t());
    node->name = ctx->getText();

    curr_node->add_identifier_node(node);

    s_.push(node);
    
}

void 
antl4_parser_t::enterInheritedVirtualType(aloeParser::InheritedVirtualTypeContext* /*ctx*/)  
{
    s_.top()->mark_pointer(true);
}

void 
antl4_parser_t::exitInheritedVirtualType(aloeParser::InheritedVirtualTypeContext* /*ctx*/) 
{
    s_.top()->mark_pointer(false);
}

void 
antl4_parser_t::exitIdentifier(aloeParser::IdentifierContext* /*ctx*/)
{
    s_.pop();
}
