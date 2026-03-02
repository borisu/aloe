#include "pch.h"
#include "antlr4_parser.h"
#include "utils.h"



using namespace aloe;
using namespace std;
using namespace antlr4;

static int object_id = 0;
static int anonymous_id_counter = 0;

parser_ptr_t
aloe::create_parser()
{
    return parser_ptr_t(new antl4_parser_t());
}

bool 
antl4_parser_t::parse_from_string(const string& str, ast_ptr_t& ast)
{
    std::istringstream stream(str);
    return parse_from_stream(stream,ast);
}

bool 
antl4_parser_t::parse_from_stream(istream& stream, ast_ptr_t& ast)
{
    bool success = true;

    try {
       
        ANTLRInputStream input(stream);
        aloeLexer lexer(&input);
        CommonTokenStream tokens(&lexer);

        aloeParser parser(&tokens);
        parser.addErrorListener(this);

        ast.reset(new ast_t());
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
antl4_parser_t::parse_from_file(const string& file_name, ast_ptr_t& ast)
{
    try {

        std::ifstream stream;
        stream.open(file_name, std::ifstream::in);

        if (!stream.is_open())
        {
            loginl("error: cannot open file:%s\n", file_name.c_str());
            return false;
        }

        return parse_from_stream(stream,ast);
        
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
    environment_ptr_t new_env(new environment_t(env));
    object_node_ptr_t obj (new object_node_t());

    obj->id         = walk_identifier(env, ctx->identifier(), true, ID_TYPE);
    obj->inh_chain  = walk_chain_declaration(env, ctx->inheritanceChain());
    obj->fields     = walk_var_list(new_env, ctx->varList());
    

    env->register_id(obj->id, obj);
    
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

            if (t->type_type != TYPE_OBJECT)
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
        return type_node_ptr_t(new type_node_t(TYPE_OBJECT, obj));
    }
    else if (ctx->funDeclaration())
    {
        auto obj = walk_function_decalaration(env, ctx->funDeclaration());
        return type_node_ptr_t(new type_node_t(TYPE_OBJECT, obj));
    }
    else if (ctx->builtinType())
    {
        if (ctx->builtinType()->int_())
        {
            return type_node_ptr_t(new type_node_t(TYPE_INT));
        }
        else if (ctx->builtinType()->void_())
        {
            return type_node_ptr_t(new type_node_t(TYPE_VOID));
        }
        else if (ctx->builtinType()->char_())
        {
            return type_node_ptr_t(new type_node_t(TYPE_CHAR));
        }
        else if (ctx->builtinType()->double_())
        {
            return type_node_ptr_t(new type_node_t(TYPE_DOUBLE));
        }
        else if (ctx->builtinType()->opaque())
        {
            return type_node_ptr_t(new type_node_t(TYPE_OPAQUE));
        }
    }
    else if (ctx->identifier())
    {
        identifier_node_ptr_t id_node = walk_identifier(env, ctx->identifier(), false, ID_TYPE);

        type_node_ptr_t type_node(new type_node_t(TYPE_OBJECT, env->find_id(id_node)));
        type_node->id = id_node;

        node_ptr_t def = env->find_id(id_node);

        switch (def->type)
        {
        case OBJECT_NODE:
            return type_node_ptr_t(new type_node_t(TYPE_OBJECT, def));
        case FUNCTION_NODE:
            return type_node_ptr_t(new type_node_t(TYPE_FUNCTION, def));
        default: {}
        }
    }
    else
    {
        throw parse_exeption_t();
    }
  
    throw parse_exeption_t();
}


fun_node_ptr_t
antl4_parser_t::walk_function_decalaration( environment_ptr_t env, aloeParser::FunDeclarationContext* ctx)
{
   
    environment_ptr_t new_env(new environment_t(env));
    fun_node_ptr_t fun_node = fun_node_ptr_t(new fun_node_t());
    fun_node->id = walk_identifier(env, ctx->identifier(), true, ID_TYPE);
    fun_node->ret_type  = walk_type(env, ctx->funType()->type());
    fun_node->params = walk_var_list(new_env, ctx->funType()->varList());
    
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

    return fun_node;
}

var_list_node_ptr_t
antl4_parser_t::walk_var_list( environment_ptr_t env, aloeParser::VarListContext* ctx)
{
    var_list_node_ptr_t var_list(new var_node_list_t());
    
    bool err = false;
    for (auto& varCtx : ctx->varDeclaration())
    {
        auto var_ptr = walk_var(env, varCtx);
        
    };
    
    return var_list;
}

var_node_ptr_t 
antl4_parser_t::walk_var(environment_ptr_t env, aloeParser::VarDeclarationContext* ctx)
{
  
    var_node_ptr_t var_node  = var_node_ptr_t(new var_node_t());
    var_node->id = walk_identifier(env, ctx->identifier(), true, ID_VAR);
    var_node->type           = walk_type(env, ctx->type());
    env->register_id(var_node->id, var_node);

    return var_node;
}

identifier_node_ptr_t  
antl4_parser_t::walk_identifier(environment_ptr_t env, aloeParser::IdentifierContext* ctx, bool id_declaration, identifier_type_e expected_id_type)
{
    if (!ctx)
        return identifier_node_ptr_t();

    identifier_node_ptr_t id_node = identifier_node_ptr_t(new identifier_node_t());

    id_node->name    =ctx->getText() ;
    id_node->id_type = expected_id_type;

    auto already_exists_node_ptr = env->find_id(id_node);
        
    if (id_declaration && already_exists_node_ptr)
    {
        loginl("error on (line:%zu, pos:%zu) - identifier %s was already defined", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), id_node->name.c_str());
        throw parse_exeption_t();
    }
    else if (!id_declaration && !already_exists_node_ptr)
    {
        loginl("error on (line:%zu, pos:%zu) - identifier %s was not defined", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), id_node->name.c_str());
        throw parse_exeption_t();
    }
    
    return id_node;
    
}

literal_node_ptr_t 
antl4_parser_t::walk_literal(environment_ptr_t env, aloeParser::LiteralContext* ctx)
{
    literal_node_ptr_t literal_node(new literal_node_t());

    if (ctx->DigitSequence())
    {
        literal_node->value = std::stoi(ctx->DigitSequence()->getText());
    }
    else if (ctx->StringLiteral().size() > 0)
    {
        string sf;
        for (auto& s :ctx->StringLiteral())
        {
            sf += s->getText();
        }
        literal_node->value = unescape(sf.substr(1, sf.size() - 2));
    }
    else if (ctx->CharacterConstant())
    {
        
        literal_node->value = unescape(ctx->CharacterConstant()->getText())[0];
    }
    else
    {
        loginl("parse error (line:%zu, pos:%zu): Cannot parse literal %s", ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), ctx->getText().c_str());
        throw parse_exeption_t();
    }

    return literal_node;
}

arglist_node_ptr_t 
antl4_parser_t::walk_arglist(environment_ptr_t env, aloeParser::ArgumentExpressionListContext* ctx)
{
    arglist_node_ptr_t arg_list(new arglist_node_t());
    for (auto& exprCtx : ctx->expression())
    {
        arg_list->args.push_back(walk_expression(env, exprCtx));
    }
    return arg_list;
}

expr_node_ptr_t 
antl4_parser_t::walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx)
{

#define INSTANCE_OF(C) C* e = dynamic_cast<C*>(ctx)    

   if (INSTANCE_OF(aloeParser::Expr_identifierContext)) {
       NEW_EXPR_NODE(expr_node, identifier);

       expr_node->id = walk_identifier(env, e->identifier(), false,ID_VAR);
	   expr_node->id_def = env->find_id(expr_node->id);

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_literalContext)) {
       NEW_EXPR_NODE(expr_node, literal);

       expr_node->literal  = walk_literal(env, e->literal());

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_bracketedContext)) {
      
       return walk_expression(env, e->expression());
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_sfxplusplusContext)) {
       NEW_EXPR_NODE(expr_node, sfxplusplus);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_sfxminminContext)) {

       NEW_EXPR_NODE(expr_node, sfxminmin);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_funcallContext)) {
       NEW_EXPR_NODE(expr_node, funcall);

	   expr_node->function = walk_expression(env, e->expression());

       if (e->argumentExpressionList())
       {
		   expr_node->arg_list = walk_arglist(env, e->argumentExpressionList());
       }

       return expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_indexContext)) {
	   NEW_EXPR_NODE(expr_node, index);

	   expr_node->operand1 = walk_expression(env, e->expression(0));
       expr_node->operand2 = walk_expression(env, e->expression(0));

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_dotContext)) {

       NEW_EXPR_NODE(expr_node, dot);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), false, ID_VAR);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_arrowContext)) {

       NEW_EXPR_NODE(expr_node, arrow);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), false, ID_VAR);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preplusplusContext)) {
       NEW_EXPR_NODE(expr_node, preplusplus);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preminminContext)) {
       NEW_EXPR_NODE(expr_node, preminmin);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_plusContext)) {

       NEW_EXPR_NODE(expr_node, plus);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_minContext)) {

       NEW_EXPR_NODE(expr_node, min);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_notContext)) {

       NEW_EXPR_NODE(expr_node, not);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_bwsnotContext)) {

       NEW_EXPR_NODE(expr_node, bwsnot);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_castContext)) {

       NEW_EXPR_NODE(expr_node, cast);

       expr_node->type_node = walk_type(env, e->type());
       expr_node->operand   = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_derefContext)) {

       NEW_EXPR_NODE(expr_node, deref);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addressofContext)) {

       NEW_EXPR_NODE(expr_node, addressof);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeofexprContext)) {

       NEW_EXPR_NODE(expr_node, sizeofexpr);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeoftypeContext)) {

       NEW_EXPR_NODE(expr_node, sizeoftype);

       expr_node->type_node = walk_type(env, e->type());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_multContext)) {

       NEW_EXPR_NODE(expr_node, mult);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_divContext)) {

       NEW_EXPR_NODE(expr_node, div);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_modContext)) {

       NEW_EXPR_NODE(expr_node, mod);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addContext)) {

       NEW_EXPR_NODE(expr_node, add);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_subContext)) {

       NEW_EXPR_NODE(expr_node, sub);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftleftContext)) {

       NEW_EXPR_NODE(expr_node, shiftleft);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftrightContext)) {

       NEW_EXPR_NODE(expr_node, shiftright);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lessContext)) {

       NEW_EXPR_NODE(expr_node, less);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lesseeqContext)) {

       NEW_EXPR_NODE(expr_node, lesseeq);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreContext)) {

       NEW_EXPR_NODE(expr_node, more);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreeqContext)) {

       NEW_EXPR_NODE(expr_node, moreeq);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicaleqContext)) {

       NEW_EXPR_NODE(expr_node, logicaleq);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_noteqContext)) {

       NEW_EXPR_NODE(expr_node, noteq);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_andContext)) {

       NEW_EXPR_NODE(expr_node, and);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_xorContext)) {

       NEW_EXPR_NODE(expr_node, xor);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_orContext)) {

       NEW_EXPR_NODE(expr_node, or);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalandContext)) {

       NEW_EXPR_NODE(expr_node, logicaland);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalorContext)) {

       NEW_EXPR_NODE(expr_node, logicalor);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_ternaryContext)) {
       NEW_EXPR_NODE(expr_node, ternary);

       expr_node->condition  = walk_expression(env, e->expression()[0]);
       expr_node->true_expr  = walk_expression(env, e->expression()[1]);
       expr_node->false_expr = walk_expression(env, e->expression()[2]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_assignContext)) {

       NEW_EXPR_NODE(expr_node, assign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addassignContext)) {
       NEW_EXPR_NODE(expr_node, addassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_subassignContext)) {
       NEW_EXPR_NODE(expr_node, subassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_multassignContext)) {
       NEW_EXPR_NODE(expr_node, multassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_divassignContext)) {
       NEW_EXPR_NODE(expr_node, divassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_modassignContext)) {
       NEW_EXPR_NODE(expr_node, modassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftleftassignContext)) {
       NEW_EXPR_NODE(expr_node, shiftleftassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftrightassignContext)) {
       NEW_EXPR_NODE(expr_node, shiftrightassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_andassignContext)) {
       NEW_EXPR_NODE(expr_node, andassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_xorassignContext)) {

       NEW_EXPR_NODE(expr_node, xorassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_orassignContext)) {

       NEW_EXPR_NODE(expr_node, orassign);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_commaContext)) {
       NEW_EXPR_NODE(expr_node, comma);

       if (e->argumentExpressionList())
       {
           expr_node->arg_list = walk_arglist(env, e->argumentExpressionList());
       }

       return expr_node;

   }
 
   return nullptr;

#undef INSTANCE_OF
}

