#include "pch.h"
#include "antlr4_parser.h"
#include "aloe/utils.h"


using namespace aloe;
using namespace std;
using namespace antlr4;

static int object_id = 0;
static int function_id = 0;

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

#define PRINT_ERROR(  )

bool 
antl4_parser_t::parse_from_string(const string& str)
{
    std::istringstream stream(str);
    
    return parse_from_stream(stream);
}

scope_node_ptr_t
antl4_parser_t::find_my_scope_node(node_ptr_t first)
{
    node_ptr_t curr_node = first;

    while (curr_node != nullptr)
    {
        scope_node_ptr_t scope_node = std::dynamic_pointer_cast<scope_node_t>(curr_node);

        if (scope_node)
            return scope_node;

        curr_node = curr_node->prev;
    }

    return nullptr;
}


node_ptr_t
antl4_parser_t::find_type_definition_by_name(node_ptr_t first, const string& name)
{
    scope_node_ptr_t scope_node = find_my_scope_node(first);

    while (scope_node != nullptr)
    {
        if (scope_node->type_defs.count(name) > 0)
        {
            return scope_node->type_defs[name];
        }
        scope_node = find_my_scope_node(scope_node->prev);
    }
    return nullptr;
}

bool 
antl4_parser_t::parse_from_stream(istream& stream)
{
    bool success = true;

    try {
       
        ANTLRInputStream input(stream);
        aloeLexer lexer(&input);
        CommonTokenStream tokens(&lexer);

        aloeParser parser(&tokens);
        parser.addErrorListener(this);

        ast_ptr_t ast(new ast_t());
        success &= walk_prog(ast, parser.prog());
        success &= parser.getNumberOfSyntaxErrors() == 0;
    }
    catch (std::exception& e)
    {
        logi("Error: %s", e.what());
        success = false;
    }
    
    return success;
}

bool
antl4_parser_t::parse_from_file(const string& file_name)
{
    try {

        std::ifstream stream;
        stream.open(file_name, std::ifstream::in);

        if (!stream.is_open())
        {
            logi("error: cannot open file:%s\n", file_name.c_str());
            return false;
        }

        return parse_from_stream(stream);
        
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


bool 
antl4_parser_t::walk_prog(ast_ptr_t ast, aloeParser::ProgContext* ctx)
{
    
    prog_node_ptr_t root(new prog_node_t());

    ast->root = root;

    bool res = true;

    for (auto& statement_ctx : ctx->globalScopeStatements())
    {
        if (statement_ctx->varDeclaration() != nullptr)
        {
            printf("variable!");
        }
        else if (statement_ctx->funDeclaration() != nullptr)
        {
            printf("function!");
        }
        else if (statement_ctx->objectDeclaration() != nullptr)
        {
            object_node_ptr_t obj = walk_object_declaration(root, statement_ctx->objectDeclaration());
            if (!obj)
                res = false;
        }
    }

    return res;
    
}

object_node_ptr_t 
antl4_parser_t::walk_object_declaration(node_ptr_t parent, aloeParser::ObjectDeclarationContext* ctx)
{
    string name = ctx->identifier() ? ctx->identifier()->getText() : string("$anonymous_object_") + std::to_string(++object_id);

    if (find_type_definition_by_name(parent, name))
    {
        printf("error on (line:%zu, pos:%zu) - object type %s already defined.", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), name.c_str());
        return nullptr;
    }

    object_node_ptr_t obj (new object_node_t());
    obj->prev = parent;

    if (!walk_chain_declaration(obj, ctx->inheritanceChain()))
        return nullptr;

    find_my_scope_node(parent)->type_defs[name] = obj;

    return obj;
}

bool 
antl4_parser_t::walk_chain_declaration(object_node_ptr_t obj, aloeParser::InheritanceChainContext* chainCtx)
{
    if (!chainCtx)
        return true;

    for (auto& typeCtx : chainCtx->type())
    {
        type_node_ptr_t t = walk_type(dynamic_pointer_cast<node_t>(obj), typeCtx);
        if (!t)
            return false;
        
        if (t->type_type != OBJECT)
        {
            printf("error on (line:%zu, pos:%zu) - wrong base type for inheritance", typeCtx->funDeclaration()->getStart()->getLine(), typeCtx->funDeclaration()->getStart()->getStartIndex());
            return false;
        }

        obj->layers.push_back(t);
    }

    return true;
}

type_node_ptr_t
antl4_parser_t::walk_type(node_ptr_t parent, aloeParser::TypeContext* ctx, int ref_count)
{
    if (ctx->pointerType())
    {
        return walk_type(parent, ctx->pointerType()->type(), ++ref_count);
    }
    else if (ctx->objectDeclaration())
    {
        auto obj  = walk_object_declaration(parent, ctx->objectDeclaration());
        if (!obj)
            return nullptr;
        
        return type_node_ptr_t(new type_node_t(OBJECT, obj));
    }
    else if (ctx->identifier())
    {
        if (ctx->identifier()->getText() == "int")
        {
            return type_node_ptr_t(new type_node_t(INT));
        }
        else if (ctx->identifier()->getText() == "double")
        {
            return type_node_ptr_t(new type_node_t(DOUBLE));
        }
        else if (ctx->identifier()->getText() == "char")
        {
            return type_node_ptr_t(new type_node_t(CHAR));
        }
        else
        {
            node_ptr_t type_def = find_type_definition_by_name(parent, ctx->identifier()->getText());
            if (!type_def)
            {
                printf("error on (line:%zu, pos:%zu) - unknown type %s", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), ctx->identifier()->getText().c_str());
                return nullptr;
            }

            switch (type_def->type)
            {
            case OBJECT_NODE:
                return type_node_ptr_t(new type_node_t(OBJECT, type_def));
            case FUNCTION_NODE:
                return type_node_ptr_t(new type_node_t(FUNCTION, type_def));
            default:
                return nullptr;
            }
        }
    }
  
    return nullptr;
}


fun_node_ptr_t
antl4_parser_t::walk_function_decalaration(node_ptr_t parent, aloeParser::FunDeclarationContext* ctx)
{

    string name = ctx->identifier() ? ctx->identifier()->getText() : string("$anonymous_function_") + std::to_string(++function_id);

    if (find_type_definition_by_name(parent, name))
    {
        printf("error on (line:%zu, pos:%zu) - function %s already defined", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), name.c_str());
        return nullptr;
    }

    string ret_type = ctx->funType()->getText();
    

    return nullptr;
}