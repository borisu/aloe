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

node_ptr_t
antl4_parser_t::find_type_definition(node_ptr_t parent, const string& name)
{
    node_ptr_t curr_node = parent;

    while (curr_node != nullptr)
    {
        if (curr_node->type_ids.count(name) > 0)
        {
            return curr_node->type_ids[name];
        }
        curr_node = curr_node->prev;
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

        auto prog_ctx = parser.prog();

        ast_t_ptr     ast (new ast_t);
        ns_node_ptr_t root(new ns_node_t());
        
        ast->root = root;

        for (auto& ctx : prog_ctx->statement())
        {
            success &= walk_global_statement(root, ctx);
        }

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
antl4_parser_t::walk_global_statement(ns_node_ptr_t sn, aloeParser::StatementContext *ctx)
{
    bool res = true;
    if (ctx->varDeclaration() != nullptr)
    {
        printf("variable!");
    }
    else if (ctx->funDeclaration() != nullptr)
    {
        fun_node_ptr_t fun = walk_function_decalaration(sn, ctx->funDeclaration());
        if (!fun)
            res = false;
    }
    else if (ctx->objectDeclaration() != nullptr)
    {
        object_node_ptr_t obj =  walk_object_declaration(sn, ctx->objectDeclaration());
        if (!obj)
            res = false;
    }

    return res;
    
}

object_node_ptr_t 
antl4_parser_t::walk_object_declaration(node_ptr_t parent, aloeParser::ObjectDeclarationContext* ctx)
{
    string name = ctx->identifier() ? ctx->identifier()->getText() : string("$anonymous_object_") + std::to_string(++object_id);
    if (find_type_definition(parent, name))
    {
        printf("error on (line:%zu, pos:%zu) - object type %s already defined.", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), name.c_str());
        return nullptr;
    }

    object_node_ptr_t obj (new object_node_t());
    obj->prev = parent;

    if (!walk_chain_declaration(obj, ctx->inheritanceChain()))
        return nullptr;

    parent->type_ids[name] = obj;
    obj = obj;

    return obj;
}

bool 
antl4_parser_t::walk_chain_declaration(object_node_ptr_t obj, aloeParser::InheritanceChainContext* chainCtx)
{
    if (!chainCtx)
        return true;

    for (auto& typeCtx : chainCtx->type())
    {
        type_ptr_t t = walk_type(dynamic_pointer_cast<node_t>(obj), typeCtx);
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

type_ptr_t 
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

        type_ptr_t type(new type_t(OBJECT, obj->name));
        return type;
    }
    else if (ctx->identifier())
    {
        if (ctx->identifier()->getText() == "int")
        {
            return type_ptr_t(new type_t(INT, "int"));
        }
        else if (ctx->identifier()->getText() == "double")
        {
            return type_ptr_t(new type_t(DOUBLE, "double"));
        }
        else if (ctx->identifier()->getText() == "char")
        {
            return type_ptr_t(new type_t(CHAR, "char"));
        }
        else
        {
            node_ptr_t type_node = find_type_definition(parent, ctx->identifier()->getText());
            if (!type_node)
            {
                printf("error on (line:%zu, pos:%zu) - unknown type %s", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), ctx->identifier()->getText().c_str());
                return nullptr;
            }

            switch (type_node->type)
            {
            case OBJECT_NODE:
                return type_ptr_t(new type_t(OBJECT, dynamic_pointer_cast<object_node_t>(type_node)->name));
            case FUNCTION_NODE:
                return type_ptr_t(new type_t(FUNCTION, dynamic_pointer_cast<object_node_t>(type_node)->name));
            default:
                return nullptr;
            }
        }
    }
  
    return nullptr;
}


fun_node_ptr_t
antl4_parser_t::walk_function_decalaration(ns_node_ptr_t parent, aloeParser::FunDeclarationContext* ctx)
{

    string name = ctx->identifier() ? ctx->identifier()->getText() : string("$anonymous_function_") + std::to_string(++function_id);

    if (find_type_definition(parent, name))
    {
        printf("error on (line:%zu, pos:%zu) - function %s already defined", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), name.c_str());
        return nullptr;
    }

    string ret_type = ctx->funType()->getText();
    

    return nullptr;
}