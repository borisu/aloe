#include "pch.h"
#include "base/defs.h"
#include "base/scope_guard.h"
#include "lang/aloe_exception.h"
#include "lang/aloe_type.h"
#include "lang/ast/ast.h"
#include "utils.h"
#include "antlr4_parser.h"
#include "antlr4_parser/parser.h"


using namespace aloe;
using namespace std;
using namespace antlr4;

static int object_id = 0;
static int anonymous_id_counter = 0;

#define INSTANCE_OF(C) C* e = dynamic_cast<C*>(ctx)    

#define RAISE_LOC(fmt, ...) \
    throw aloe_exception_t("%s:%zu:%zu: error: " fmt, \
        env->source().c_str(), \
        ctx->getStart()->getLine(), \
        ctx->getStart()->getStartIndex(), \
        ##__VA_ARGS__)

#define ASSERT_LOC(C, fmt, ...) \
    if (!(C)) { \
         RAISE_LOC(fmt, ##__VA_ARGS__); \
    }

#define INIT_POS(node, ctx) \
    node->line = (int)ctx->getStart()->getLine(); \
    node->pos  = (int)ctx->getStart()->getStartIndex();

#define INIT_END_POS(node, ctx) \
    node->line = (int)ctx->getStop()->getLine(); \
    node->pos  = (int)ctx->getStop()->getStartIndex();


parser_t_ptr
aloe::create_antlr4_parser()
{
    return parser_t_ptr(new antl4_parser_t());
}

antl4_parser_t::antl4_parser_t()
{
    syntax_error_occurred = false;
}

bool
antl4_parser_t::parse_from_stream(istream& stream, ast_t_ptr& ast, const string& source_id, node_type_e root_grammar)
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

        environment_t_ptr src_mod(new source_modifier_t(source_id));
		environment_t_ptr fun_mod(new fun_modifier_t(nullptr, src_mod));
		environment_t_ptr scp_mod(new scope_modifier_t(SCOPE_GLOBAL, fun_mod));
		environment_t_ptr env_mod(new environment_modifier_t(scp_mod));

        environment_t_ptr env = env_mod;


        switch (root_grammar)
        {
        case PROG_NODE:
        {
            ast->root = walk_prog(env, parser.prog());
            break;
        }
        case TYPE_NODE:
        {
            auto type_node   = newptr(type_node_t);
            type_node->type = walk_type(env, parser.atype());

            ast->root        = type_node;

            break;
        }
        default:
            RAISE("unknown root grammar type %d", root_grammar);
        }

        if (syntax_error_occurred)
        {
            RAISE("compilation failed: see previous errors for more details.");
		}
    }
    catch (aloe_exception_t& e)
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


prog_node_t_ptr
antl4_parser_t::walk_prog(environment_t_ptr env, aloeParser::ProgContext* ctx)
{
    prog_node_t_ptr out(new prog_node_t());
    INIT_POS(out, ctx);

	bool res = true;
   
    if (syntax_error_occurred)
    {
        throw aloe_exception_t("compilation failed: see previous errors for more details.");
	}

	if (ctx->moduleStatement())
    {
        out->module_name = walk_identifier(env,ctx->moduleStatement()->identifier());
    }

    for (auto& statement : ctx->topLevelStatement())
    {
        try
        {
			node_t_ptr statement_node;
            if (statement->varDeclaration())
            {
				statement_node = walk_var(env, statement->varDeclaration());
            }
            else if (statement->funDeclaration())
            {
				statement_node = walk_fun_declaration(env, statement->funDeclaration());
            }
			else if (statement->layoutDeclaration())
			{
                statement_node = walk_layout_declaration(env, statement->layoutDeclaration());
			}
            else if (statement->expectation())
            {
                walk_expectation(env, statement->expectation());
            }
			else
			{
				RAISE_LOC("unknown declaration statement '%s'", statement->getText().c_str());
			}
			out->statements.push_back(out);
            
        }
        catch (aloe_exception_t &e)
        {
            loginl("%s", e.what());
            res = false; // not failing immediately, giving chance for see other parsing errors;
        }
    }

    if (!res)
        throw aloe_exception_t("compilation failed: see previous errors for more details.");
    
    return out;
}

type_proxy_t_ptr
antl4_parser_t::walk_type( environment_t_ptr env, aloeParser::TypeContext* ctx)
{
    aloe_type_t_ptr type;
	INIT_POS(type, ctx);

    type_proxy_t_ptr out;

    if (INSTANCE_OF(aloeParser::Type_intContext)) 
    {
		type = newptr(aloe_type_t,ALOE_TYPE_INT);
    }
    else if (INSTANCE_OF(aloeParser::Type_charContext))
    {
		type = newptr(aloe_type_t,ALOE_TYPE_CHAR);
    }
    else if (INSTANCE_OF(aloeParser::Type_doubleContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_DOUBLE);
    }
    else if (INSTANCE_OF(aloeParser::Type_voidContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_VOID);
	}
    else if (INSTANCE_OF(aloeParser::Type_funContext))
    {
        out = walk_fun_type(env, e->funType());
    }
    else if (INSTANCE_OF(aloeParser::Type_groupedContext))
    {
		out = walk_type(env, e->atype());
    }
    else if (INSTANCE_OF(aloeParser::Type_pointerContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_PTR);
		type->ptr->pointee_type  = walk_type(env, e->atype());
    }
	else if (INSTANCE_OF(aloeParser::Type_arrayContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_ARRAY);
        type->arr->elem_type  = walk_type(env, e->atype());
		type->arr->size      = e->DigitSequence() ? stoul(e->DigitSequence()->getText()) : -1;
    }
    else if (INSTANCE_OF(aloeParser::Type_layoutContext))
    {
		out = walk_layout_declaration(env, e->layoutDeclaration())->type;
    }
    else if (INSTANCE_OF(aloeParser::Type_identifierContext))
    {
        auto id_node = walk_identifier(env, e->identifier());
		out = env->find_type(id_node);
        ASSERT_LOC(type, "undefined identifier '%s'", id_node->name.c_str());
    }
    else
    {
        RAISE_LOC("unknown type '%s'", ctx->getText().c_str());
    }

    if (!out)
    {
        out = newptr(type_proxy_t, type);
    }
     
    return out;
}

void 
antl4_parser_t::walk_expectation(environment_t_ptr env, aloeParser::ExpectationContext* ctx)
{
	if (ctx->expectFun())
	{
		walk_fun_expectation(env, ctx->expectFun());
	}
	else if (ctx->expectLayout())
	{
		walk_layout_expectation(env, ctx->expectLayout());
	}
	else
	{
		RAISE_LOC("unknown expectation '%s'", ctx->getText().c_str());
	}
}

void 
antl4_parser_t::walk_fun_expectation(environment_t_ptr env, aloeParser::ExpectFunContext* ctx)
{
	auto out = newptr(fun_node_t);
	INIT_POS(out, ctx);
	
    out->is_defined = false;
    out->idt        = walk_identifier(env, ctx->identifier());
    out->type       = walk_fun_type(env, ctx->funType());

    auto prev       = env->find_object(out->idt, true);
	if (prev)
	{
        ASSERT_LOC(prev->target->node_type_id == out->node_type_id, "identifier %s was already defined as a different kind at (%d,%d)", out->idt->name.c_str(), prev->target->line,prev->target->pos);
        auto prev_fun  = PCAST(fun_node_t, prev->target);
        ASSERT_LOC(*prev_fun->type->target == *out->type->target, "identifier %s was already defined with a different type at (%d,%d)", out->idt->name.c_str(), prev->target->line, prev->target->pos);
	}
    else
    {
        env->register_object(out->idt, out);
    }
	
    return;
}

void 
antl4_parser_t::walk_layout_expectation(environment_t_ptr env, aloeParser::ExpectLayoutContext* ctx)
{
    auto type = newptr(aloe_type_t, ALOE_TYPE_LAYOUT);
    INIT_POS(type, ctx);

	auto id = walk_identifier(env, ctx->identifier());
    type->lay->name =id->name;
	type->lay->is_incomplete = true;

    auto prev = env->find_type(id, true);
    if (prev)
    {
        ASSERT_LOC(*prev->target == *type, "identifier %s was already defined with different type at (%d,%d)", id->name.c_str(), prev->target->line, prev->target->pos);
    }
    else
    {
        env->register_type(id, type);
    }
}

layout_node_t_ptr
antl4_parser_t::walk_layout_declaration(environment_t_ptr env, aloeParser::LayoutDeclarationContext* ctx)
{
    auto out = newptr(layout_node_t);
    INIT_POS(out, ctx);

    auto type = newptr(aloe_type_t, ALOE_TYPE_LAYOUT);

    if (ctx->identifier())
    {
		auto id = walk_identifier(env, ctx->identifier());
        type->lay->name = id->name;
       
        auto prev = env->find_type(id, true);
        if (prev)
        {
            if (!prev->target->lay->is_incomplete)
            {
                RAISE_LOC("layout %s was already defined", out->type->target->lay->name.c_str());
            }
        }

        out->type =  env->register_type(id, out->type->target);
    }

    if (ctx->gtChain())
    {
        out->type->target->lay->gt_chain = walk_gt_chain_node(env, ctx->gtChain());
    }

	if (ctx->layoutMemberList())
	{
		out->type->target->lay->fields = walk_layout_member_list(env, ctx->layoutMemberList());
	}

    return out;
}

var_t_ptr
antl4_parser_t::walk_layout_member(environment_t_ptr env, aloeParser::LayoutMemberContext* ctx)
{
    auto out = newptr(var_t);
    INIT_POS(out, ctx);
   
    if (ctx->identifier()) {
        out->name = ctx->identifier()->getText();
    }

    out->type = walk_type(env, ctx->atype());

    return out;
}

var_set_t_ptr
antl4_parser_t::walk_layout_member_list(environment_t_ptr env, aloeParser::LayoutMemberListContext* ctx)
{
    auto out = newptr(var_set_t);
    INIT_POS(out, ctx);

    for (auto& member : ctx->layoutMember())
    {
        auto var = walk_layout_member(env, member);

        out->v.push_back(var);

        if (!var->name.empty())
        {
            if (out->m.count(var->name) != 0)
            {
                RAISE_LOC("layout member '%s' was already defined", var->name.c_str());
            }
            out->m[var->name] = var;
        }
       
    }
    return out;
}

gt_set_t_ptr
antl4_parser_t::walk_gt_chain_node(environment_t_ptr env, aloeParser::GtChainContext* ctx)
{
    auto out = newptr(gt_set_t);
    INIT_POS(out, ctx);
	
	for (auto& member : ctx->gtMember())
	{
        auto gt = walk_gt_chain_member(env, member);

        ASSERT(out->m.count(*gt->type->target) == 0, "same type appears twice in gt chain");
        
        out->m[*gt->type->target] = gt;
        out->v.push_back(gt);
	}

	return out;
}

gt_t_ptr
antl4_parser_t::walk_gt_chain_member(environment_t_ptr env, aloeParser::GtMemberContext* ctx)
{
    auto out = newptr(gt_t);
   
    if (ctx->identifier())
    {
        auto id     = walk_identifier(env, ctx->identifier());
        auto prev   = env->find_type(id);
        
        ASSERT (prev, "layout member '%s' was not defined", id->name.c_str());
        out->type = prev;
    }
    else if (ctx->layoutDeclaration())
    {
        auto layout_type    = walk_layout_declaration(env, ctx->layoutDeclaration());
        out->type           = layout_type->type;
    }
    else
    {
        RAISE_LOC("unknown gt chain member '%s'", ctx->getText().c_str());
    }

    return out;
}

type_proxy_t_ptr
antl4_parser_t::walk_fun_type(environment_t_ptr env, aloeParser::FunTypeContext* ctx)
{
    auto type = newptr(aloe_type_t, ALOE_TYPE_FUNCTION);
    INIT_POS(type, ctx);

    auto out = newptr(type_proxy_t, type);

    type->fun->ret_type  = walk_type(env, ctx->atype());

    environment_t_ptr new_env(new scope_modifier_t(SCOPE_FUN_ARGS, env));
    type->fun->params = walk_var_list(new_env, ctx->varList());
    
    
    return out;
}


fun_node_t_ptr
antl4_parser_t::walk_fun_declaration( environment_t_ptr env, aloeParser::FunDeclarationContext* ctx)
{
    auto out = newptr(fun_node_t);
    INIT_POS(out, ctx);
    
	out->is_defined  = true;
    out->idt         = walk_identifier(env, ctx->identifier());
    

    auto prev      = out->idt  ? env->find_object(out->idt) : nullptr;

    ASSERT_LOC(prev->target->node_type_id == out->node_type_id, "identifier %s was already defined as a different kind at (%d,%d)", out->idt->name.c_str(), prev->target->line, prev->target->pos);

    auto prev_fun       = prev ? PCAST(fun_node_t, prev->target) : nullptr;

    // check that function is defined twice
    if (prev && prev_fun->is_defined)
    {
        RAISE_LOC("function %s was already defined at (%d:%d)", out->idt->name.c_str(), prev_fun->line, prev_fun->pos);
    }

    environment_t_ptr fun_mod(new fun_modifier_t(out, env));
    environment_t_ptr scope_mod(new scope_modifier_t(SCOPE_FUNCTION, fun_mod));
	environment_t_ptr env_mod(new environment_modifier_t(scope_mod));
    
    environment_t_ptr new_env = env_mod;

    out->type = walk_fun_type(new_env, ctx->funType());

	// check that function is not defined with different type
    if (prev)
    {
        if (*prev_fun->type->target != *out->type->target)
        {
			RAISE_LOC("function %s was already declared with different type at (%d:%d)", out->idt->name.c_str(), prev_fun->line, prev_fun->pos);
        }

    }

    if (out->idt)
    {
        env->register_object(out->idt, out); // it will mark previous node as ignore
    }

    for (auto& exec_ctx : ctx->funLevelStatement())
    {
        if (exec_ctx->varDeclaration())
        {
            out->statements.push_back(walk_var(new_env, exec_ctx->varDeclaration()));
        }
        else if (exec_ctx->funDeclaration())
        {
            out->statements.push_back(walk_fun_declaration(new_env, exec_ctx->funDeclaration()));
        }
        else if (exec_ctx->expression())
        {
            out->statements.push_back(walk_expression(new_env, exec_ctx->expression()));
        }
        else if (exec_ctx->returnStatement())
        {
            out->statements.push_back(walk_return(new_env, exec_ctx->returnStatement()));
        }

    } // for 

	out->end_of_fun = marker_node_t_ptr(new marker_node_t());
	INIT_END_POS(out->end_of_fun, ctx);
   
    return out;
}

var_set_t_ptr
antl4_parser_t::walk_var_list( environment_t_ptr env, aloeParser::VarListContext* ctx)
{
    auto  out = newptr(var_set_t);
    INIT_POS(out, ctx);
    
    bool err = false;
    
    for (auto& varCtx : ctx->varDeclaration())
    {
        auto var_node = walk_var(env, varCtx);
        var_t_ptr var = var_t_ptr(new var_t());
        var->type = var_node->type;
        var->name  = var_node->id->name;

        if (!var->name.empty())
        {
            ASSERT(out->m.count(var->name) == 0, "same type appears twice in vars");
            out->m[var->name] = var;
        }
		
		out->v.push_back(var);
    };
    
    return out;
}

var_node_t_ptr 
antl4_parser_t::walk_var(environment_t_ptr env, aloeParser::VarDeclarationContext* ctx)
{
    var_node_t_ptr out  = var_node_t_ptr(new var_node_t());
    INIT_POS(out, ctx);

    out->id = walk_identifier(env, ctx->identifier());

    if (out->id)
    {
        auto prev_node = env->find_object(out->id,true);
        if (prev_node)
        {
			RAISE_LOC("var %s was already defined", out->id->name.c_str());
        }
    }
    else if (env->curr_scope() != SCOPE_FUN_ARGS)
    {
		RAISE_LOC("variable declaration must have an identifier in this scope");
    }
    
    out->type = walk_type(env, ctx->atype());
   
    if (ctx->expression())
    {
        if (env->curr_scope() == SCOPE_FUN_ARGS)
        {
			RAISE_LOC("variable initialization is not allowed in this context");
        }

        if (env->curr_scope() == SCOPE_GLOBAL && !dynamic_cast<aloeParser::Expr_literalContext*>(ctx->expression()))
        {
			RAISE_LOC("only literal expressions are allowed for global variable initialization");
        }

        out->initializer = walk_expression(env, ctx->expression());
		check_type_equality(env, ctx, out->initializer->type, out->type);

        
    }

    if (out->id)
    {
        env->register_object(out->id, out);
    }

    return out;
}

identifier_node_t_ptr  
antl4_parser_t::walk_identifier(environment_t_ptr env, aloeParser::IdentifierContext* ctx)
{
    if (!ctx)
        return identifier_node_t_ptr();

    identifier_node_t_ptr id_node = identifier_node_t_ptr(new identifier_node_t());
    INIT_POS(id_node, ctx);

    id_node->name           = ctx->getText() ;
    
    return id_node;
}

literal_node_t_ptr 
antl4_parser_t::walk_literal(environment_t_ptr env, aloeParser::LiteralContext* ctx)
{
    auto  out = newptr (literal_node_t);
    INIT_POS(out, ctx);

    if (ctx->DigitSequence())
    {
        out->lit_type_id = LIT_INT;
        out->value = std::stoi(ctx->DigitSequence()->getText());
		out->type  = newptr(type_proxy_t,newptr(aloe_type_t, ALOE_TYPE_INT));
    }
    else if (ctx->StringLiteral().size() > 0)
    {
		out->lit_type_id = LIT_STRING;
        string sf;
        for (auto& s :ctx->StringLiteral())
        {
            sf += s->getText();
        }
        out->value = unescape(sf.substr(1, sf.size() - 2));
        out->type = newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_ARRAY));
    	out->type->target->arr->elem_type = newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_CHAR));  

		out->type->target->arr->size = (int)std::get<string>(out->value).size() + 1; // +1 for null terminator
		
    }
    else if (ctx->CharacterConstant())
    {
		out->lit_type_id = LIT_CHAR;
        out->value = unescape(ctx->CharacterConstant()->getText())[0];
        out->type = newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_CHAR));
   	
    }
    else
    {
        RAISE_LOC("cannot parse literal %s", ctx->getText().c_str());
    }

    return out;
}

arglist_node_t_ptr 
antl4_parser_t::walk_arg_list(environment_t_ptr env, aloeParser::ArgumentExpressionListContext* ctx)
{
    arglist_node_t_ptr out(new arglist_node_t());
    INIT_POS(out, ctx);

    for (auto& exprCtx : ctx->expression())
    {
        out->args.push_back(walk_expression(env, exprCtx));
    }
    
    return out;
}

return_node_t_ptr  
antl4_parser_t::walk_return(environment_t_ptr env, aloeParser::ReturnStatementContext* ctx)
{
    if (!env->curr_fun())
    {
		RAISE_LOC("return statement is not allowed outside of function");
    }

    return_node_t_ptr out(new return_node_t());
    INIT_POS(out, ctx);

    if (ctx->expression())
    {
        out->return_expr = walk_expression(env, ctx->expression());

        if (*out->return_expr->type->target != *env->curr_fun()->type->target->fun->ret_type->target)
        {
			RAISE_LOC("return expression type '%s' does not match function return type '%s'",
				out->return_expr->type->target->fun->ret_type->target->to_str().c_str(),
				env->curr_fun()->type->target->fun->ret_type->target->to_str().c_str());
        }
    } 
    else if (env->curr_fun()->type->target->fun->ret_type->target->type_id != ALOE_TYPE_VOID) 
    {
		RAISE_LOC("function '%s' must return expression of type '%s'",
			env->curr_fun()->idt->name.c_str(),
			env->curr_fun()->type->target->fun->ret_type->target->to_str().c_str());
    }

	return out;
}

expr_node_t_ptr 
antl4_parser_t::walk_expression(environment_t_ptr env, aloeParser::ExpressionContext* ctx)
{
   expr_node_t_ptr out;

   if (INSTANCE_OF(aloeParser::Expr_identifierContext)) {
       NEW_EXPR_NODE(expr_node, identifier);
     
       expr_node->id = walk_identifier(env, e->identifier());
	   expr_node->ref = env->find_object(expr_node->id);
       
       switch (expr_node->ref->target->node_type_id)
       {
       case VAR_NODE:
       {
           expr_node->type = PCAST(var_node_t, expr_node->ref->target)->type    ;
           expr_node->is_lvalue = true;
           break;
       }
       case FUNCTION_NODE:
       {
           expr_node->type = PCAST(fun_node_t, expr_node->ref->target)->type;
           break;
       }
       default:
       {
		   RAISE_LOC("identifier '%s' is not a variable or function", expr_node->id->name.c_str());
       }
       }
       
       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_literalContext)) {
       NEW_EXPR_NODE(expr_node, literal);
       INIT_POS(expr_node, ctx);

       expr_node->literal   = walk_literal(env, e->literal());
	   expr_node->type = expr_node->literal->type;
      
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

       check_is_lvalue(env, ctx, expr_node->operand, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");
       

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_sfxminminContext)) {

       NEW_EXPR_NODE(expr_node, sfxminmin);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;
       expr_node->is_lvalue = true;
      
       check_is_lvalue(env, ctx, expr_node->operand, "--");
       check_unary_arithmetic(env, ctx, expr_node, "--");
   
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_funcallContext)) {
       NEW_EXPR_NODE(expr_node, funcall);
       INIT_POS(expr_node, ctx);

	   expr_node->fun_expr = walk_expression(env, e->expression());
       if (expr_node->fun_expr->type->target->type_id != ALOE_TYPE_FUNCTION)
       {
           RAISE_LOC("expression '%s' is not of a function type", ctx->getText().c_str());
       }

       auto fun_node_type = expr_node->fun_expr->type;

	   expr_node->type = fun_node_type->target->fun->ret_type;
	   expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());

       if (expr_node->arg_list->args.size() != fun_node_type->target->fun->params->v.size())
       {
           RAISE_LOC("function call expects %zu arguments but %zu were provided",
               fun_node_type->target->fun->params->v.size(),
               fun_node_type->target->fun->params->v.size());
       }
       
       for (int i=0; i < expr_node->arg_list->args.size(); i++)
       {
		   auto arg     = expr_node->arg_list->args[i];
		   auto param   = fun_node_type->target->fun->params->v[i];
		   if (*arg->type->target != *param->type->target)
           {
			   RAISE_LOC("function call expects argument %d of type '%s' but argument of type '%s' was provided",
				   i + 1,
				   param->name.c_str(),
				   arg->type->target->to_str().c_str());
           }
       }

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_indexContext)) {
	   NEW_EXPR_NODE(expr_node, index);
       INIT_POS(expr_node, ctx);

	   expr_node->operand1 = walk_expression(env, e->expression(0));
       expr_node->operand2 = walk_expression(env, e->expression(1));

	   expr_node->type = expr_node->operand1->type->target->arr->elem_type;
       expr_node->is_lvalue = true;
       out = expr_node;

       
   }
   else if (INSTANCE_OF(aloeParser::Expr_dotContext)) {

       NEW_EXPR_NODE(expr_node, dot);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier());

	   throw;  // dot operator not implemented yet

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_arrowContext)) {
       NEW_EXPR_NODE(expr_node, arrow);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier());

	   throw;  // TBD: pointer dereference not implemented yet

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preplusplusContext)) {
       NEW_EXPR_NODE(expr_node, preplusplus);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;
       
       
       check_is_lvalue(env, ctx, expr_node->operand, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preminminContext)) {
       NEW_EXPR_NODE(expr_node, preminmin);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;


       check_is_lvalue(env, ctx, expr_node->operand, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_plusContext)) {
       NEW_EXPR_NODE(expr_node, plus);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_minContext)) {
       NEW_EXPR_NODE(expr_node, min);
       INIT_POS(expr_node, ctx);

       
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_notContext)) {
       NEW_EXPR_NODE(expr_node, not);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "!");

       out = expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_bwsnotContext)) {
       NEW_EXPR_NODE(expr_node, bwsnot);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->type = expr_node->operand->type;

       check_unary_arithmetic(env, ctx, expr_node, "!");
       
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_castContext)) {
       NEW_EXPR_NODE(expr_node, cast);
       INIT_POS(expr_node, ctx);

       expr_node->type = walk_type(env, e->atype());
       expr_node->operand   = walk_expression(env, e->expression());

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_derefContext)) {
       NEW_EXPR_NODE(expr_node, deref);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
	   check_is_pointer(env, ctx, expr_node->operand, "@");
       expr_node->type = expr_node->operand->type->target->ptr->pointee_type;

       expr_node->is_lvalue = true;
       
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addressofContext)) {
       NEW_EXPR_NODE(expr_node, addressof);
       INIT_POS(expr_node, ctx);
       
       expr_node->operand = walk_expression(env, e->expression());
       check_is_lvalue(env, ctx, expr_node->operand, "^");

       
       expr_node->type = newptr(type_proxy_t,newptr (aloe_type_t,ALOE_TYPE_PTR));
	   expr_node->type->target->ptr->pointee_type->target = expr_node->operand->type->target;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeofexprContext)) {
       NEW_EXPR_NODE(expr_node, sizeofexpr);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
	   expr_node->type = newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_INT)); // sizeof operator always returns int

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeoftypeContext)) {
       NEW_EXPR_NODE(expr_node, sizeoftype);
       INIT_POS(expr_node, ctx);

       expr_node->type = walk_type(env, e->atype());
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_multContext)) {
        NEW_EXPR_NODE(expr_node, mult);
        INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "*");
       check_binary_arithmetic(env,  ctx, expr_node, "*");

       out = expr_node;
       

   }
   else if (INSTANCE_OF(aloeParser::Expr_divContext)) {
       NEW_EXPR_NODE(expr_node, div);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       
       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "/");
       check_binary_arithmetic(env, ctx, expr_node, "/");

       out = expr_node;
       

   }
   else if (INSTANCE_OF(aloeParser::Expr_modContext)) {
       NEW_EXPR_NODE(expr_node, mod);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       
       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "%");
       check_binary_arithmetic(env, ctx, expr_node, "%");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addContext)) {
       NEW_EXPR_NODE(expr_node, add);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "+");
       check_binary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_subContext)) {
       NEW_EXPR_NODE(expr_node, sub);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);
       
       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "-");
       check_binary_arithmetic(env, ctx, expr_node, "-");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftleftContext)) {
       NEW_EXPR_NODE(expr_node, shiftleft);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<<");
       check_binary_arithmetic(env, ctx, expr_node, "<<");

       out = expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftrightContext)) {
       NEW_EXPR_NODE(expr_node, shiftright);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">>");
       check_binary_arithmetic(env, ctx, expr_node, ">>");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lessContext)) {
       NEW_EXPR_NODE(expr_node, less);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<");
       check_binary_arithmetic(env, ctx, expr_node, "<");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lesseeqContext)) {
       NEW_EXPR_NODE(expr_node, lesseeq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<=");
       check_binary_arithmetic(env, ctx, expr_node, "<=");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreContext)) {
       NEW_EXPR_NODE(expr_node, more);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">");
       check_binary_arithmetic(env, ctx, expr_node, ">");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreeqContext)) {
       NEW_EXPR_NODE(expr_node, moreeq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">=");
       check_binary_arithmetic(env, ctx, expr_node, ">=");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicaleqContext)) {
       NEW_EXPR_NODE(expr_node, logicaleq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "==");
       check_binary_arithmetic(env, ctx, expr_node, "==");

       out = expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_noteqContext)) {
       NEW_EXPR_NODE(expr_node, noteq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "!=");
       check_binary_arithmetic(env, ctx, expr_node, "!=");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_andContext)) {
       NEW_EXPR_NODE(expr_node, and);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "&");
       check_binary_arithmetic(env, ctx, expr_node, "&");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_xorContext)) {
       NEW_EXPR_NODE(expr_node, xor);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "^");
       check_binary_arithmetic(env, ctx, expr_node, "^");

       out = expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_orContext)) {
       NEW_EXPR_NODE(expr_node, or);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "|");
       check_binary_arithmetic(env, ctx, expr_node, "|");
       
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalandContext)) {
       NEW_EXPR_NODE(expr_node, logicaland);
	   INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "&&");
       check_binary_arithmetic(env, ctx, expr_node, "&&");

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalorContext)) {
       NEW_EXPR_NODE(expr_node, logicalor);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->type = expr_node->operand1->type;

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "||");
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

       check_expr_type_equality(env, ctx, expr_node->true_expr, expr_node->false_expr, "?");
       if (!is_arithmetic(expr_node->condition->type->target->type_id))
       {
           RAISE_LOC("operator '%s' cannot be applied to conditional expressions of type '%s'", 
               "?", 
               expr_node->condition->type->target->to_str().c_str());
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
antl4_parser_t::check_type_equality(environment_t_ptr env, antlr4::ParserRuleContext* ctx, type_proxy_t_ptr type1, type_proxy_t_ptr  type2)
{
    if (*type1->target != *type2->target)
    {
        RAISE_LOC("type mismatch: '%s' and '%s'", type1->target->to_str().c_str(), type2->target->to_str().c_str());
    };
}

void 
antl4_parser_t::check_expr_type_equality(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr1, expr_node_t_ptr expr2, const char* op_str)
{
    if (*expr1->type->target != *expr2->type->target)
    {
        RAISE_LOC("operator '%s' cannot be applied to expressions of different types '%s' and '%s'",
            op_str,
            expr1->type->target->to_str().c_str(),
            expr2->type->target->to_str().c_str());
    };
}

void 
antl4_parser_t::check_binary_arithmetic(environment_t_ptr env, aloeParser::ExpressionContext* ctx, binary_expr_node_t_ptr  expr_node, const char* op_str)
{
    if (!is_arithmetic(expr_node->operand1->type->target->type_id) || !is_arithmetic(expr_node->operand2->type->target->type_id))
    {
		RAISE_LOC("operator '%s' cannot be applied to expressions of type '%s' and '%s'",
			op_str,
			expr_node->operand1->type->target->to_str().c_str(),
			expr_node->operand2->type->target->to_str().c_str());
    }

	check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, op_str);
}

void 
antl4_parser_t::check_unary_arithmetic(environment_t_ptr env, aloeParser::ExpressionContext* ctx, unary_expr_node_t_ptr  expr_node, const char* op_str)
{

    if (!is_arithmetic(expr_node->operand->type->target->type_id))
    {
		RAISE_LOC("operator '%s' cannot be applied to expression of type '%s'",
			op_str,
			expr_node->operand->type->target->to_str().c_str());
    }
    
}

void 
antl4_parser_t::check_is_lvalue(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr_node, const char* op_str)
{
    if (expr_node->is_lvalue == false)
    {
		RAISE_LOC("operator '%s' cannot be applied to rvalue expression of type '%s'",
			op_str,
			expr_node->type->target->to_str().c_str());
    }
}

void 
antl4_parser_t::check_assignment(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr lhs, expr_node_t_ptr rhs)
{
    check_is_lvalue(env, ctx, lhs, "=");

    if (*lhs->type->target != *rhs->type->target)
    {
		RAISE_LOC("operator '=' cannot be applied to expressions of type '%s' and '%s'",
			lhs->type->target->to_str().c_str(),
			rhs->type->target->to_str().c_str());
    }
}

void 
antl4_parser_t::check_is_pointer(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr_node, const char* op_str)
{
	if (expr_node->type->target->type_id != ALOE_TYPE_PTR)
	{
		RAISE_LOC("operator '%s' cannot be applied to expression of non-pointer type '%s'",
			op_str,
			expr_node->type->target->to_str().c_str());
	}
}


