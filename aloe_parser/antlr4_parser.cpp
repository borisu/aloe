#include "pch.h"
#include "antlr4_parser.h"
#include "parse_exception.h"
#include "utils.h"
#include "aloe/defs.h"
#include "aloe/aloe_type.h"
#include "aloe/ast.h"

using namespace aloe;
using namespace std;
using namespace antlr4;

static int object_id = 0;
static int anonymous_id_counter = 0;

#define INSTANCE_OF(C) C* e = dynamic_cast<C*>(ctx)    

#define CHECK_TYPE(type, expected_type) \
    if (*type != *expected_type) \
    { \
        throw parse_exeption_t("%s:%zu:%zu: error: expected expression of type '%s' but got '%s'", \
            env->source_id.c_str(), \
            ctx->getStart()->getLine(), \
            ctx->getStart()->getStartIndex(), \
            expected_type->content.c_str(), \
            type->content.c_str()); \
    } \

parser_ptr_t
aloe::create_parser()
{
    return parser_ptr_t(new antl4_parser_t());
}

antl4_parser_t::antl4_parser_t()
{
    syntax_error_occurred = false;
}

bool
antl4_parser_t::parse_from_stream(istream& stream, ast_ptr_t& ast, const string& source_id)
{
    bool success = true;

    syntax_error_occurred = false;

    try {

        ANTLRInputStream input(stream);
        aloeLexer lexer(&input);
        CommonTokenStream tokens(&lexer);

        aloeParser parser(&tokens);
        parser.addErrorListener(this);

        ast.reset(new ast_t());
        ast->source_id = source_id;

        environment_ptr_t env(new  environment_t());
        env->source_id = source_id;

        ast->prog = walk_prog(env, parser.prog());

        if (syntax_error_occurred)
        {
            throw parse_exeption_t("compilation failed: see previous errors for more details.");
		}

    }
    catch (parse_exeption_t& e)
    {
        loginl("%s", e.what());
        success = false;
    }
    catch (std::exception& e)
    {
        loginl("panic:%s", e.what());
        success = false;
    }

    return success;
}

void
antl4_parser_t::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
    size_t line, size_t charPositionInLine, const std::string& msg,
    std::exception_ptr e) {
    loginl("syntax Error at line %d:%d - %s\n", line, charPositionInLine, msg.c_str());
    syntax_error_occurred = true;
}

#define INIT_POS(node, ctx) \
    node->line = (int)ctx->getStart()->getLine(); \
    node->pos  = (int)ctx->getStart()->getStartIndex();

#define INIT_END_POS(node, ctx) \
    node->line = (int)ctx->getStop()->getLine(); \
    node->pos  = (int)ctx->getStop()->getStartIndex();

prog_node_ptr_t
antl4_parser_t::walk_prog(environment_ptr_t env, aloeParser::ProgContext* ctx)
{
    prog_node_ptr_t prog(new prog_node_t());
    INIT_POS(prog, ctx);

	bool res = true;

    if (syntax_error_occurred)
    {
        throw parse_exeption_t("compilation failed: see previous errors for more details.");
	}

	if (ctx->moduleStatement())
    {
        prog->module_name = walk_identifier(env,ctx->moduleStatement()->identifier(), ID_MODULE,false);
    }

    for (auto& stmt : ctx->declarationStatementList()->declarationStatement())
    {
        try
        {
            if (stmt->varDeclaration())
            {
                prog->decl_statements.push_back(walk_var(env, stmt->varDeclaration(),CF_NONE));
            }
            else if (stmt->funDeclaration())
            {
                prog->decl_statements.push_back(walk_fun_declaration(env, stmt->funDeclaration()));
            }
            
        }
        catch (parse_exeption_t &e)
        {
            loginl("%s", e.what());
            res = false; // not failing immediately, giving chance for see other parsing errors;
        }
    }

    if (!res)
        throw parse_exeption_t("compilation failed: see previous errors for more details.");
    
    return prog;
}

type_node_ptr_t
antl4_parser_t::walk_type( environment_ptr_t env, aloeParser::TypeContext* ctx)
{
    type_node_ptr_t out (new type_node_t());
    INIT_POS(out, ctx);

    if (INSTANCE_OF(aloeParser::Type_intContext)) 
    {
		out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_INT));
    }
    else if (INSTANCE_OF(aloeParser::Type_charContext))
    {
		out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_CHAR));
    }
    else if (INSTANCE_OF(aloeParser::Type_doubleContext))
    {
		out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_DOUBLE));
    }
    else if (INSTANCE_OF(aloeParser::Type_voidContext))
    {
		out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_VOID));
	}
    else if (INSTANCE_OF(aloeParser::Type_funContext))
    {
        out = walk_fun_type(env, e->funType());
    }
    else if (INSTANCE_OF(aloeParser::Type_groupedContext))
    {
		out = walk_type(env, e->type());
    }
    else if (INSTANCE_OF(aloeParser::Type_pointerContext))
    {
		out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_PTR));
		out->ptr_pointee_type_node = walk_type(env, e->type());
		out->type->ptr_pointee_type = out->ptr_pointee_type_node->type;
    }
	else if (INSTANCE_OF(aloeParser::Type_arrayContext))
    {
		out->arr_element_type_node = walk_type(env, e->type());
		out->type->arr_element_type = out->arr_element_type_node->type;
		out->type->arr_size = e->DigitSequence() ? stoul(e->DigitSequence()->getText()) : -1;
    }
    else
    {
        throw parse_exeption_t("%s:%zu:%zu: error: unknown type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            ctx->getText().c_str());
    }
  
    return out;
}

type_node_ptr_t 
antl4_parser_t::walk_fun_type(environment_ptr_t env, aloeParser::FunTypeContext* ctx)
{
    type_node_ptr_t out(new type_node_t());
    INIT_POS(out, ctx);

    out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_FUNCTION));

    out->fun_ret_type_node = walk_type(env, ctx->type());
    out->type->fun_ret_type = out->fun_ret_type_node->type;

    out->fun_params_node = walk_var_list(env, ctx->varList(), CF_NO_INITIALIZATION);
    for (auto& var : out->fun_params_node->vars_v)
    {
        out->type->fun_param_types.push_back(var.second->type);
    }

    return out;
}


fun_node_ptr_t
antl4_parser_t::walk_fun_declaration( environment_ptr_t env, aloeParser::FunDeclarationContext* ctx)
{
    fun_node_ptr_t fun_node = fun_node_ptr_t(new fun_node_t());
    INIT_POS(fun_node, ctx);

    if (ctx->expect() && ctx->executionBlock())
    {
        throw parse_exeption_t("%s:%zu:%zu: error: function was declared as 'expect' but has body",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex());
    }

    if (!ctx->expect() && !ctx->executionBlock())
    {
        throw parse_exeption_t("%s:%zu:%zu: error: function was not declared as 'expect' but has no body",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex());
    }
    
	fun_node->is_defined = ctx->expect() == nullptr;

    fun_node->id        = walk_identifier(env, ctx->identifier(), ID_NONTYPE,false);

    auto prev_node      = fun_node->id  ? env->find_id(fun_node->id) : nullptr;
    auto prev_fun       = prev_node ? PCAST(fun_node_t, prev_node->target) : nullptr;

    // check that function is defined twice
    if (prev_node)
    {
        if (prev_fun->is_defined && fun_node->is_defined)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: function %s was already defined",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                fun_node->id->name.c_str());
        }
    }
    
    environment_ptr_t new_env(new environment_t(env));
    fun_node->type_node = walk_fun_type(new_env, ctx->funType());
	fun_node->type = fun_node->type_node->type;

	// check that function is not defined with different type
    if (prev_node)
    {
        if (*prev_fun->type_node->type != *fun_node->type_node->type)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: function %s was already declared with different type",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                fun_node->id->name.c_str());
        }

    }

    // if we already stored definition node do not attempt to store this one
    if (prev_fun)
    {
        if (prev_fun->is_defined)
        {
			fun_node->ignore = true; 
        }
    }

    if (!fun_node->ignore)
        env->register_id(fun_node->id, fun_node); // it will mark previous node as ignore
   
    if (fun_node->is_defined)
    {
		new_env->push_fun(fun_node);
        fun_node->exec_block = walk_execution_block(new_env, ctx->executionBlock());
    }

	fun_node->end_of_fun = marker_node_ptr_t(new marker_node_t());
	INIT_END_POS(fun_node->end_of_fun, ctx);
   
    return fun_node;
}

exec_block_node_ptr_t
antl4_parser_t::walk_execution_block(environment_ptr_t env, aloeParser::ExecutionBlockContext* ctx)
{
    exec_block_node_ptr_t block_node = exec_block_node_ptr_t(new exec_block_node_t());
    INIT_POS(block_node, ctx);

    for (auto& exec_ctx : ctx->executionStatement())
    {
        if (exec_ctx->varDeclaration())
        {
            block_node->exec_statements.push_back(walk_var(env, exec_ctx->varDeclaration(), CF_NONE));
        }
        else if (exec_ctx->funDeclaration())
        {
            block_node->exec_statements.push_back(walk_fun_declaration(env, exec_ctx->funDeclaration()));
        }
        else if (exec_ctx->expression())
        {
            block_node->exec_statements.push_back(walk_expression(env, exec_ctx->expression()));
        }
        else if (exec_ctx->returnStatement())
        {
            block_node->exec_statements.push_back(walk_return(env, exec_ctx->returnStatement()));
        }
		
    } // for 

	return block_node;
}

var_list_node_ptr_t
antl4_parser_t::walk_var_list( environment_ptr_t env, aloeParser::VarListContext* ctx, compile_flags_e flags)
{
    var_list_node_ptr_t var_list(new var_list_node_t());
    INIT_POS(var_list, ctx);
    
    bool err = false;
    
    for (auto& varCtx : ctx->varDeclaration())
    {
        auto var_ptr = walk_var(env, varCtx, flags);
		var_list->vars_m[var_ptr->id] = var_ptr;
		var_list->vars_v.push_back(var_id_t(var_ptr->id, var_ptr));
    };
    
    return var_list;
}

var_node_ptr_t 
antl4_parser_t::walk_var(environment_ptr_t env, aloeParser::VarDeclarationContext* ctx, compile_flags_e flags)
{
  
    var_node_ptr_t out  = var_node_ptr_t(new var_node_t());
    INIT_POS(out, ctx);

    out->id = walk_identifier(env, ctx->identifier(), ID_NONTYPE, false);

    if (out->id)
    {
        auto prev_node = env->find_id(out->id,true);
        if (prev_node)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: var %s was already defined",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                out->id->name.c_str());
        }
    }
    
    out->type_node = walk_type(env, ctx->type());
	out->type = out->type_node->type;
   
    if (ctx->expression())
    {
        if (flags & CF_NO_INITIALIZATION)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: variable initialization is not allowed in this context",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex());
        }

        if (env->top_fun() == nullptr && !dynamic_cast<aloeParser::Expr_literalContext*>(ctx->expression()))
        {
            throw parse_exeption_t("%s:%zu:%zu: error: only literal expressions are allowed for global variable initialization",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex());
        }

        out->initializer = walk_expression(env, ctx->expression());
        if (*out->initializer->type != *out->type_node->type)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: cannot initialize variable of type '%s' with expression of type '%s'",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                ctx->type()->getText().c_str(),
				ctx->expression()->getText().c_str());
        }
    }

    if (out->id)
    {
        env->register_id(out->id, out);
    }

    return out;
}

identifier_node_ptr_t  
antl4_parser_t::walk_identifier(environment_ptr_t env, aloeParser::IdentifierContext* ctx, identifier_type_e expected_id_type, bool must_exist)
{
    if (!ctx)
        return identifier_node_ptr_t();

    identifier_node_ptr_t id_node = identifier_node_ptr_t(new identifier_node_t());
    INIT_POS(id_node, ctx);

    id_node->name    = ctx->getText() ;
    id_node->idt_type_id = expected_id_type;

    auto prev_node = env->find_id(id_node);
    if (!prev_node && must_exist)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: identifier '%s' is not defined",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
			id_node->name.c_str());
    }
        
    return id_node;
}

literal_node_ptr_t 
antl4_parser_t::walk_literal(environment_ptr_t env, aloeParser::LiteralContext* ctx)
{
    literal_node_ptr_t literal_node(new literal_node_t());

    INIT_POS(literal_node, ctx);

    if (ctx->DigitSequence())
    {
        literal_node->lit_type_id = LIT_INT;
        literal_node->value = std::stoi(ctx->DigitSequence()->getText());
		literal_node->type  = make_shared<aloe_type_t>(ALOE_TYPE_INT);
    }
    else if (ctx->StringLiteral().size() > 0)
    {
		literal_node->lit_type_id = LIT_STRING;
        string sf;
        for (auto& s :ctx->StringLiteral())
        {
            sf += s->getText();
        }
        literal_node->value = unescape(sf.substr(1, sf.size() - 2));
		literal_node->type  = make_shared<aloe_type_t>(ALOE_TYPE_PTR);
		literal_node->type->ptr_pointee_type = make_shared<aloe_type_t>(ALOE_TYPE_CHAR);
		
    }
    else if (ctx->CharacterConstant())
    {
		literal_node->lit_type_id = LIT_CHAR;
        literal_node->value = unescape(ctx->CharacterConstant()->getText())[0];
        literal_node->type  = make_shared<aloe_type_t>(ALOE_TYPE_CHAR);
		
    }
    else if (ctx->pointerToVoid())
    {
        literal_node->lit_type_id = LIT_POINTER_VOID;
        literal_node->value = std::stoi(ctx->pointerToVoid()->DigitSequence()->getText());
        literal_node->type = make_shared<aloe_type_t>(ALOE_TYPE_PTR);
        literal_node->type->ptr_pointee_type = make_shared<aloe_type_t>(ALOE_TYPE_VOID);
    }
    else
    {
        throw 
            parse_exeption_t("%s:%zu:%zu: error: cannot parse literal %s", env->source_id.c_str(), ctx->getStart()->getLine(), ctx->getStart()->getStartIndex(), ctx->getText().c_str());
    }

    return literal_node;
}

arglist_node_ptr_t 
antl4_parser_t::walk_arg_list(environment_ptr_t env, aloeParser::ArgumentExpressionListContext* ctx)
{
    arglist_node_ptr_t arg_list(new arglist_node_t());
    INIT_POS(arg_list, ctx);

    for (auto& exprCtx : ctx->expression())
    {
        arg_list->args.push_back(walk_expression(env, exprCtx));
    }
    
    return arg_list;
}

return_node_ptr_t  
antl4_parser_t::walk_return(environment_ptr_t env, aloeParser::ReturnStatementContext* ctx)
{
    if (env->top_fun() == nullptr)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: 'return' statement is not allowed outside of function",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex());
    }

    return_node_ptr_t return_node(new return_node_t());
    INIT_POS(return_node, ctx);

    if (ctx->expression())
    {
        return_node->return_expr = walk_expression(env, ctx->expression());

        if (*return_node->return_expr->type != *env->top_fun()->type->fun_ret_type)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: cannot return expression of type '%s' from function with return type '%s'",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                ctx->expression()->getText().c_str(),
                return_node->return_expr->type->to_str().c_str(),
                env->top_fun()->type->to_str().c_str());
        }
    } 
    else if (env->top_fun()->type->fun_ret_type->type_id != ALOE_TYPE_VOID) 
    {
        throw parse_exeption_t("%s:%zu:%zu: error: must return expression of type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            env->top_fun()->type->to_str().c_str());
    }

	return return_node;
}

expr_node_ptr_t 
antl4_parser_t::walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx)
{
   expr_node_ptr_t out;

   if (INSTANCE_OF(aloeParser::Expr_identifierContext)) {
       NEW_EXPR_NODE(expr_node, identifier);
     
       expr_node->id = walk_identifier(env, e->identifier(),ID_NONTYPE,true);
	   expr_node->ast_def = env->find_id(expr_node->id);
       
       switch (expr_node->ast_def->target->node_type_id)
       {
       case VAR_NODE:
       {
           expr_node->type = PCAST(var_node_t, expr_node->ast_def->target)->type_node->type    ;
           break;
       }
       case FUNCTION_NODE:
       {
           expr_node->type = PCAST(fun_node_t, expr_node->ast_def->target)->type_node->type;
		   expr_node->is_lvalue = false;
           break;
       }
       default:
       {
           throw parse_exeption_t("%s:%zu:%zu: error: identifier '%s' is not a variable or function",
               env->source_id.c_str(),
               ctx->getStart()->getLine(),
               ctx->getStart()->getStartIndex(),
               expr_node->id->name.c_str());
       }
       }
       
       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_literalContext)) {
       NEW_EXPR_NODE(expr_node, literal);
       INIT_POS(expr_node, ctx);

       expr_node->literal   = walk_literal(env, e->literal());
	   expr_node->type = expr_node->literal->type;
       expr_node->is_lvalue = false;
	  
       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_bracketedContext)) {
      
       out = walk_expression(env, e->expression());
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_sfxplusplusContext)) {
       NEW_EXPR_NODE(expr_node, sfxplusplus);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;
       expr_node->is_lvalue = true;

       check_unary_arithmetic(env, ctx, expr_node, "++");
       check_lvalue(env, ctx, expr_node, "++");

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_sfxminminContext)) {

       NEW_EXPR_NODE(expr_node, sfxminmin);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;
       expr_node->is_lvalue = true;
      
       check_unary_arithmetic(env, ctx, expr_node, "--");
       check_lvalue(env, ctx, expr_node, "--");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_funcallContext)) {
       NEW_EXPR_NODE(expr_node, funcall);
       INIT_POS(expr_node, ctx);

	   expr_node->fun_expr = walk_expression(env, e->expression());
       if (expr_node->fun_expr->type->type_id != ALOE_TYPE_FUNCTION)
       {
           throw parse_exeption_t("%s:%zu:%zu: error: expression %s is not of a function type",
               env->source_id.c_str(),
               ctx->getStart()->getLine(),
               ctx->getStart()->getStartIndex(),
               ctx->getText().c_str());
       }

       auto fun_node_type = expr_node->fun_expr->type;

	   expr_node->type = fun_node_type->fun_ret_type;
	   expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());

       if (expr_node->arg_list->args.size() != fun_node_type->fun_param_types.size())
       {
           throw parse_exeption_t("%s:%zu:%zu: error: expected %zu arguments in function call but %zu were provided",
               env->source_id.c_str(),
               ctx->getStart()->getLine(),
               ctx->getStart()->getStartIndex(),
               fun_node_type->fun_param_types.size(),
			   expr_node->arg_list->args.size());
       }
       
       for (int i=0; i < expr_node->arg_list->args.size(); i++)
       {
		   auto arg     = expr_node->arg_list->args[i];
		   auto param   = fun_node_type->fun_param_types[i];
		   if (*arg->type != *param)
           {
               throw parse_exeption_t("%s:%zu:%zu: error: cannot convert argument %d of type '%s' to parameter of type '%s'",
                   env->source_id.c_str(),
                   ctx->getStart()->getLine(),
                   ctx->getStart()->getStartIndex(),
                   i+1,
                   arg->type->to_str().c_str(),       
                   param->to_str().c_str());
           }
       }

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_indexContext)) {
	   NEW_EXPR_NODE(expr_node, index);
       INIT_POS(expr_node, ctx);

	   expr_node->operand1 = walk_expression(env, e->expression(0));
       expr_node->operand2 = walk_expression(env, e->expression(0));

       throw;  // TBD: array indexing is not supported yet'

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_dotContext)) {

       NEW_EXPR_NODE(expr_node, dot);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), ID_NONTYPE,true);

       throw;  // TBD: array indexing is not supported yet'

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_arrowContext)) {
       NEW_EXPR_NODE(expr_node, arrow);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), ID_NONTYPE,true);

       throw;  // TBD: array indexing is not supported yet'

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preplusplusContext)) {
       NEW_EXPR_NODE(expr_node, preplusplus);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;
       expr_node->is_lvalue = true;
       
       check_lvalue(env, ctx, expr_node, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preminminContext)) {
       NEW_EXPR_NODE(expr_node, preminmin);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       expr_node->type = expr_node->operand->type;
       expr_node->is_lvalue = true;

       check_lvalue(env, ctx, expr_node, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_plusContext)) {
       NEW_EXPR_NODE(expr_node, plus);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

	   expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_minContext)) {
       NEW_EXPR_NODE(expr_node, min);
       INIT_POS(expr_node, ctx);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_notContext)) {
       NEW_EXPR_NODE(expr_node, not);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "!");

       out = expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_bwsnotContext)) {
       NEW_EXPR_NODE(expr_node, bwsnot);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "!");
       
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_castContext)) {
       NEW_EXPR_NODE(expr_node, cast);
       INIT_POS(expr_node, ctx);

       expr_node->type_node = walk_type(env, e->type());
       expr_node->type = expr_node->type_node->type;
       expr_node->operand   = walk_expression(env, e->expression());
	   

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_derefContext)) {
       NEW_EXPR_NODE(expr_node, deref);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

	   check_pointer(env, ctx, expr_node->operand, "@");

       expr_node->type = expr_node->operand->type->ptr_pointee_type;
       expr_node->is_lvalue = true;
       
       out = expr_node;
       

   }
   else if (INSTANCE_OF(aloeParser::Expr_addressofContext)) {
       NEW_EXPR_NODE(expr_node, addressof);
       INIT_POS(expr_node, ctx);

       
       expr_node->operand = walk_expression(env, e->expression());
       check_lvalue(env, ctx, expr_node->operand, "^");

       expr_node->is_lvalue = false;
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeofexprContext)) {
       NEW_EXPR_NODE(expr_node, sizeofexpr);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
	   expr_node->type = make_shared<aloe_type_t>(ALOE_TYPE_INT); // sizeof operator always returns int
       expr_node->is_lvalue = false;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeoftypeContext)) {
       NEW_EXPR_NODE(expr_node, sizeoftype);
       INIT_POS(expr_node, ctx);

       expr_node->type_node = walk_type(env, e->type());
	   expr_node->type = expr_node->type_node->type;   
       expr_node->is_lvalue = false;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_multContext)) {
        NEW_EXPR_NODE(expr_node, mult);
        INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = true;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "*");
       check_binary_arithmetic(env,  ctx, expr_node, "*");

       out = expr_node;
       

   }
   else if (INSTANCE_OF(aloeParser::Expr_divContext)) {
       NEW_EXPR_NODE(expr_node, div);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "/");
       check_binary_arithmetic(env, ctx, expr_node, "/");

       out = expr_node;
       

   }
   else if (INSTANCE_OF(aloeParser::Expr_modContext)) {
       NEW_EXPR_NODE(expr_node, mod);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "%");
       check_binary_arithmetic(env, ctx, expr_node, "%");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addContext)) {
       NEW_EXPR_NODE(expr_node, add);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "+");
       check_binary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_subContext)) {
       NEW_EXPR_NODE(expr_node, sub);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);
       
       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "-");
       check_binary_arithmetic(env, ctx, expr_node, "-");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftleftContext)) {
       NEW_EXPR_NODE(expr_node, shiftleft);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<<");
       check_binary_arithmetic(env, ctx, expr_node, "<<");

       out = expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftrightContext)) {
       NEW_EXPR_NODE(expr_node, shiftright);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">>");
       check_binary_arithmetic(env, ctx, expr_node, ">>");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lessContext)) {
       NEW_EXPR_NODE(expr_node, less);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<");
       check_binary_arithmetic(env, ctx, expr_node, "<");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lesseeqContext)) {
       NEW_EXPR_NODE(expr_node, lesseeq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<=");
       check_binary_arithmetic(env, ctx, expr_node, "<=");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreContext)) {
       NEW_EXPR_NODE(expr_node, more);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">");
       check_binary_arithmetic(env, ctx, expr_node, ">");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreeqContext)) {
       NEW_EXPR_NODE(expr_node, moreeq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">=");
       check_binary_arithmetic(env, ctx, expr_node, ">=");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicaleqContext)) {
       NEW_EXPR_NODE(expr_node, logicaleq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "==");
       check_binary_arithmetic(env, ctx, expr_node, "==");

       out = expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_noteqContext)) {
       NEW_EXPR_NODE(expr_node, noteq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "!=");
       check_binary_arithmetic(env, ctx, expr_node, "!=");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_andContext)) {
       NEW_EXPR_NODE(expr_node, and);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "&");
       check_binary_arithmetic(env, ctx, expr_node, "&");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_xorContext)) {
       NEW_EXPR_NODE(expr_node, xor);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "^");
       check_binary_arithmetic(env, ctx, expr_node, "^");

       out = expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_orContext)) {
       NEW_EXPR_NODE(expr_node, or);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "|");
       check_binary_arithmetic(env, ctx, expr_node, "|");
       
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalandContext)) {
       NEW_EXPR_NODE(expr_node, logicaland);
	   INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "&&");
       check_binary_arithmetic(env, ctx, expr_node, "&&");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalorContext)) {
       NEW_EXPR_NODE(expr_node, logicalor);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->is_lvalue = false;
       expr_node->type = expr_node->operand1->type;

       check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "||");
       check_binary_arithmetic(env, ctx, expr_node, "||");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_ternaryContext)) {
       NEW_EXPR_NODE(expr_node, ternary);
       INIT_POS(expr_node, ctx);

       expr_node->condition  = walk_expression(env, e->expression()[0]);
       expr_node->true_expr  = walk_expression(env, e->expression()[1]);
       expr_node->false_expr = walk_expression(env, e->expression()[2]);

       expr_node->is_lvalue = expr_node->true_expr->is_lvalue && expr_node->false_expr->is_lvalue;
       expr_node->type = expr_node->true_expr->type;

       check_type_equality(env, ctx, expr_node->true_expr, expr_node->false_expr, "?");
       if (!is_arithmetic(expr_node->condition->type->type_id))
       {
           throw parse_exeption_t("%s:%zu:%zu: error: operator '%s' cannot be applied to conditional expressions of type '%s'",
               env->source_id.c_str(),
               ctx->getStart()->getLine(),
               ctx->getStart()->getStartIndex(),
               "?",
               expr_node->condition->type->to_str().c_str());
       }

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_assignContext)) {
       NEW_EXPR_NODE(expr_node, assign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);
       
       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_addassignContext)) {
       NEW_EXPR_NODE(expr_node, addassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "+=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;
       
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_subassignContext)) {
       NEW_EXPR_NODE(expr_node, subassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "-=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_multassignContext)) {
       NEW_EXPR_NODE(expr_node, multassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "*=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_divassignContext)) {
       NEW_EXPR_NODE(expr_node, divassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "/=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_modassignContext)) {
       NEW_EXPR_NODE(expr_node, modassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);


	   check_binary_arithmetic(env, ctx, expr_node, "%=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftleftassignContext)) {
       NEW_EXPR_NODE(expr_node, shiftleftassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "<<=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftrightassignContext)) {
       NEW_EXPR_NODE(expr_node, shiftrightassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, ">>=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_andassignContext)) {
       NEW_EXPR_NODE(expr_node, andassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "&=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_xorassignContext)) {
       NEW_EXPR_NODE(expr_node, xorassign);
	   INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "^=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_orassignContext)) {
       NEW_EXPR_NODE(expr_node, orassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "|=");
       check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->type = expr_node->operand1->type;
       expr_node->is_lvalue = true;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_commaContext)) {
       NEW_EXPR_NODE(expr_node, comma);
       INIT_POS(expr_node, ctx);

       if (e->argumentExpressionList())
       {
           expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());
       }
       else
       {
           throw;
       }

       expr_node->type = expr_node->arg_list->args.back()->type;
       expr_node->is_lvalue = expr_node->arg_list->args.back()->is_lvalue;

       out = expr_node;
   }
 
   INIT_POS(out, ctx);
 
   return out;

}

void 
antl4_parser_t::check_type_equality(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr1, expr_node_ptr_t expr2, const char* op_str)
{
    if (*expr1->type != *expr2->type)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: operator '%s' cannot be applied to expressions of different types '%s' and '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            op_str,
            expr1->type->to_str().c_str(),
            expr2->type->to_str().c_str());
    }
}

void 
antl4_parser_t::check_binary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, binary_expr_node_ptr_t  expr_node, const char* op_str)
{
    if (!is_arithmetic(expr_node->operand1->type->type_id) || !is_arithmetic(expr_node->operand2->type->type_id))
    {
        throw parse_exeption_t("%s:%zu:%zu: error: operator '%s' cannot be applied to expressions of type '%s' and '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            op_str,
            expr_node->operand1->type->to_str().c_str(),
            expr_node->operand2->type->to_str().c_str());
    }

	check_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, op_str);
}

void 
antl4_parser_t::check_unary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, unary_expr_node_ptr_t  expr_node, const char* op_str)
{

    if (!is_arithmetic(expr_node->operand->type->type_id))
    {
        throw parse_exeption_t("%s:%zu:%zu: error: operator '%s' cannot be applied to expressions of type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            op_str,
            expr_node->operand->type->to_str().c_str());
    }
    
}

void 
antl4_parser_t::check_lvalue(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr_node, const char* op_str)
{
    if (expr_node->is_lvalue == false)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: operator '%s' cannot be applied to rvalue expression of type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            op_str,
            expr_node->type->to_str().c_str());
    }
}

void 
antl4_parser_t::check_assignment(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t lhs, expr_node_ptr_t rhs)
{
    check_lvalue(env, ctx, lhs, "=");

    if (lhs->type != rhs->type)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: cannot assign expression of type '%s' to expression of type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            rhs->type->to_str().c_str(),
            lhs->type->to_str().c_str());
    }
}

void 
antl4_parser_t::check_pointer(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr_node, const char* op_str)
{
	if (expr_node->type->type_id != ALOE_TYPE_PTR)
	{
		throw parse_exeption_t("%s:%zu:%zu: error: operator '%s' cannot be applied to expression of non-pointer type '%s'",
			env->source_id.c_str(),
			ctx->getStart()->getLine(),
			ctx->getStart()->getStartIndex(),
			op_str,
			expr_node->type->to_str().c_str());
	}
}