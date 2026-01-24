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

bool 
antl4_parser_t::parse_from_string(const string& str)
{
    std::istringstream stream(str);
    return parse_from_stream(stream);
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
        environment_ptr_t env(new  environment_t());
        ast->prog = walk_prog(env, parser.prog());

    }
    catch (parse_exeption_t&)
    {
        loginl("Compilation failed!");
        success = false;
    }
    catch (std::exception& e)
    {
        loginl("Error: %s", e.what());
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
            loginl("error: cannot open file:%s\n", file_name.c_str());
            return false;
        }

        return parse_from_stream(stream);
        
    }
    catch (std::exception& e)
    {
        loginl("unexpected error: %s", e.what());
    }

    return false;
}

void
antl4_parser_t::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
    size_t line, size_t charPositionInLine, const std::string& msg,
    std::exception_ptr e) {
    loginl("syntax Error at line %d:%d - %s\n", line, charPositionInLine, msg.c_str());
}


prog_node_ptr_t
antl4_parser_t::walk_prog(environment_ptr_t env, aloeParser::ProgContext* ctx)
{
    prog_node_ptr_t prog(new prog_node_t());
    bool res = true;

    for (auto& stmt : ctx->declarationStatementList()->declarationStatement())
    {
        try
        {
            if (stmt->varDeclaration() != nullptr)
            {
                walk_var(env, stmt->varDeclaration());
            }
            else if (stmt->funDeclaration() != nullptr)
            {
                walk_function_decalaration(env, stmt->funDeclaration());
            }
            else if (stmt->objectDeclaration() != nullptr)
            {
                walk_object_declaration(env, stmt->objectDeclaration());
            }
        }
        catch (parse_exeption_t)
        {
            res = false; // not failing immediately, giving chance for see other parsing errors;
        }
    }

    if (!res)
        throw parse_exeption_t();
    
    return prog;
}

object_node_ptr_t 
antl4_parser_t::walk_object_declaration( environment_ptr_t env, aloeParser::ObjectDeclarationContext* ctx)
{
    string name = ctx->identifier() ? ctx->identifier()->getText() : string("$anonymous_object_") + std::to_string(++object_id);

    if (env->find_type_definition_by_name(name))
    {
        loginl("error on (line:%zu, pos:%zu) - object type %s already defined.", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), name.c_str());
        throw parse_exeption_t();
    }

    object_node_ptr_t obj (new object_node_t());

    obj->inh_chain = walk_chain_declaration(env, ctx->inheritanceChain());

    // NEW SCOPE
    {
        environment_ptr_t new_env(new environment_t(env));
        obj->fields = walk_var_list(new_env, ctx->varList());
    }
    
    
    env->register_type(name,obj);

    return obj;
}

inh_chain_node_ptr_t
antl4_parser_t::walk_chain_declaration( environment_ptr_t env, aloeParser::InheritanceChainContext* chainCtx)
{
    inh_chain_node_ptr_t inh_chain(new inh_chain_node_t());
    
    if (chainCtx)
    {
        for (auto& typeCtx : chainCtx->type())
        {
            type_node_ptr_t t = walk_type(env, typeCtx);

            if (t->type_type != OBJECT)
            {
                loginl("error on (line:%zu, pos:%zu) - wrong base type '%s' for inheritance", chainCtx->getStart()->getLine(), chainCtx->getStart()->getStartIndex(), typeCtx->getText().c_str());
                throw parse_exeption_t();
            }

            inh_chain->layers.push_back(t);
        }
    }
    

    return inh_chain;
}

type_node_ptr_t
antl4_parser_t::walk_type( environment_ptr_t env, aloeParser::TypeContext* ctx, int ref_count)
{
    if (ctx->pointerType())
    {
        return walk_type(env, ctx->pointerType()->type(), ++ref_count);
    }
    else if (ctx->objectDeclaration())
    {
        auto obj  = walk_object_declaration(env, ctx->objectDeclaration());
        return type_node_ptr_t(new type_node_t(OBJECT, obj));
    }
    else if (ctx->builtinType())
    {
        if (ctx->builtinType()->int_())
        {
            return type_node_ptr_t(new type_node_t(INT));
        }
        else if (ctx->builtinType()->void_())
        {
            return type_node_ptr_t(new type_node_t(VOID));
        }
        else if (ctx->builtinType()->char_())
        {
            return type_node_ptr_t(new type_node_t(CHAR));
        }
        else if (ctx->builtinType()->double_())
        {
            return type_node_ptr_t(new type_node_t(DOUBLE));
        }
        else if (ctx->builtinType()->opaque())
        {
            return type_node_ptr_t(new type_node_t(OPAQUE));
        }
    }
    else if (ctx->identifier())
    {
        node_ptr_t type_def = env->find_type_definition_by_name(ctx->identifier()->getText());
        if (!type_def)
        {
            loginl("error on (line:%zu, pos:%zu) - unknown type %s", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), ctx->identifier()->getText().c_str());
            throw parse_exeption_t();
        }

        switch (type_def->type)
        {
        case OBJECT_NODE:
            return type_node_ptr_t(new type_node_t(OBJECT, type_def));
        case FUNCTION_NODE:
            return type_node_ptr_t(new type_node_t(FUNCTION, type_def));
        default: {}
        }
    }
  
   throw parse_exeption_t();
}


fun_node_ptr_t
antl4_parser_t::walk_function_decalaration( environment_ptr_t env, aloeParser::FunDeclarationContext* ctx)
{
    string name = ctx->identifier() ? ctx->identifier()->getText() : string("$anonymous_function_") + std::to_string(++function_id);

    if (env->find_type_definition_by_name(name))
    {
        loginl("error on (line:%zu, pos:%zu) - function %s already defined", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), name.c_str());
        throw parse_exeption_t();
    }

    fun_node_ptr_t fun = fun_node_ptr_t(new fun_node_t());

    fun->ret_type  = walk_type(env, ctx->funType()->type());

    // NEW SCOPE
    {
       environment_ptr_t new_env(new environment_t(env));
       fun->params = walk_var_list(new_env, ctx->funType()->varList());
    
       for (auto& exec_ctx : ctx->executionStatement())
       {
           if (exec_ctx->varDeclaration())
           {
               walk_var(new_env, exec_ctx->varDeclaration());
           }
           else if (exec_ctx->funDeclaration())
           {
               walk_function_decalaration(new_env, exec_ctx->funDeclaration());
           }
           else if (exec_ctx->objectDeclaration())
           {
               walk_object_declaration(new_env, exec_ctx->objectDeclaration());
           }
           else if (exec_ctx->expression())
           {
              walk_expression(new_env, exec_ctx->expression());
        
           }
       } // for 
    } // SCOPE

    return fun;
}

var_list_node_ptr_t
antl4_parser_t::walk_var_list( environment_ptr_t env, aloeParser::VarListContext* ctx)
{
    var_list_node_ptr_t var_list(new var_node_list_t());
    

    bool err = false;
    for (auto& varCtx : ctx->varDeclaration())
    {
        auto var_ptr = walk_var(env, varCtx);
        env->register_var(var_ptr->name, var_ptr);
    };
    
    return var_list;
}

var_node_ptr_t 
antl4_parser_t::walk_var( environment_ptr_t env, aloeParser::VarDeclarationContext* ctx)
{
    string name = ctx->identifier() ? ctx->identifier()->getText() : string("$anonymous_var_") + std::to_string(++function_id);
    if (env->find_type_definition_by_name(name))
    {
        loginl("error on (line:%zu, pos:%zu) - var %s already defined", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), name.c_str());
        throw parse_exeption_t();
    }

    var_node_ptr_t var = var_node_ptr_t(new var_node_t());
  
    var->name = name;
    var->type = walk_type(env, ctx->type());

    return var;
}

expr_node_ptr_t 
antl4_parser_t::walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx)
{
    
    return nullptr;
}