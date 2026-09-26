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

parser_ptr_t
aloe::create_antlr4_parser()
{
    return parser_ptr_t(new antl4_parser_t());
}

antl4_parser_t::antl4_parser_t()
{
    syntax_error_occurred = false;
}

bool
antl4_parser_t::parse_from_stream(istream& stream, ast_ptr_t& ast, const string& source_id, node_type_e root_grammar)
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

        environment_ptr_t src_mod(new source_modifier_t(source_id));
		environment_ptr_t fun_mod(new fun_modifier_t(nullptr, src_mod));
		environment_ptr_t scp_mod(new scope_modifier_t(SCOPE_GLOBAL, fun_mod));
		environment_ptr_t env_mod(new environment_modifier_t(scp_mod));

        environment_ptr_t env = env_mod;


        switch (root_grammar)
        {
        case PROG_NODE:
        {
            ast->root = walk_prog(env, parser.prog());
            break;
        }
        case TYPE_NODE:
        {
            ast->root = walk_type(env, parser.type());
            break;
        }
        default:
            throw aloe_exception_t("unknown root grammar type %d", root_grammar);
        }

        if (syntax_error_occurred)
        {
            throw aloe_exception_t("compilation failed: see previous errors for more details.");
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
        throw aloe_exception_t("compilation failed: see previous errors for more details.");
	}

	if (ctx->moduleStatement())
    {
        prog->module_name = walk_identifier(env,ctx->moduleStatement()->identifier(), ID_MODULE,false);
    }

    for (auto& stmt : ctx->topLevelStatement())
    {
        try
        {
			node_ptr_t stmt_node;
            if (stmt->varDeclaration())
            {
				stmt_node = walk_var(env, stmt->varDeclaration());
            }
            else if (stmt->funDeclaration())
            {
				stmt_node = walk_fun_declaration(env, stmt->funDeclaration());
            }
			else if (stmt->layoutDeclaration())
			{
                stmt_node = walk_layout_declaration(env, stmt->layoutDeclaration());
			}
            else if (stmt->expectation())
            {
                walk_expectation(env, stmt->expectation());
            }
			else
			{
				RAISE_LOC("unknown declaration statement '%s'", stmt->getText().c_str());
			}
			prog->statements.push_back(prog);
            
        }
        catch (aloe_exception_t &e)
        {
            loginl("%s", e.what());
            res = false; // not failing immediately, giving chance for see other parsing errors;
        }
    }

    if (!res)
        throw aloe_exception_t("compilation failed: see previous errors for more details.");
    
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
		out->ptr_type_node = walk_type(env, e->type());
		out->type->ptr_type = out->ptr_type_node->type;
    }
	else if (INSTANCE_OF(aloeParser::Type_arrayContext))
    {
		out->arr_type_node = walk_type(env, e->type());
        out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_ARRAY));
		out->type->arr_type = out->arr_type_node->type;
		out->type->arr_size = e->DigitSequence() ? stoul(e->DigitSequence()->getText()) : -1;
    }
    else if (INSTANCE_OF(aloeParser::Type_layoutContext))
    {
		auto layout_node = walk_layout_declaration(env, e->layoutDeclaration());
		out->type = layout_node->type;
    }
    else
    {
        RAISE_LOC("unknown type '%s'", ctx->getText().c_str());
    }
  
    return out;
}


void 
antl4_parser_t::walk_expectation(environment_ptr_t env, aloeParser::ExpectationContext* ctx)
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
antl4_parser_t::walk_fun_expectation(environment_ptr_t env, aloeParser::ExpectFunContext* ctx)
{
	fun_node_ptr_t out = fun_node_ptr_t(new fun_node_t());
	INIT_POS(out, ctx);
	
    out->is_defined = false;

    out->id         = walk_identifier(env, ctx->identifier(), ID_OBJ, false);

    out->type_node = walk_fun_type(env, ctx->funType());
    out->type = out->type_node->type;
	
	auto prev_node = env->find_id(out->id, true);
	if (prev_node)
	{
        auto prev_fun  = PCAST(fun_node_t,prev_node->target);
        if (*prev_fun->type != *out->type)
            RAISE_LOC("function %s was already defined with different type at(%d,%d)", out->id->name.c_str(), prev_fun->line, prev_fun->pos);

        return;
	}
    else
    {
        env->register_id(out->id, out);
    }
	
    return;
}


void 
antl4_parser_t::walk_layout_expectation(environment_ptr_t env, aloeParser::ExpectLayoutContext* ctx)
{
    layout_node_ptr_t out = layout_node_ptr_t(new layout_node_t());
    INIT_POS(out, ctx);

    out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_LAYOUT));
    out->id = walk_identifier(env, ctx->identifier(), ID_TYPE, false);
    out->type->lot_name = out->id->name;

    auto prev_node = env->find_id(out->id, true);

    if (prev_node)
    {
        return;
    }
    else
    {
        env->register_id(out->id, out);
    }
}

layout_node_ptr_t 
antl4_parser_t::walk_layout_declaration(environment_ptr_t env, aloeParser::LayoutDeclarationContext* ctx)
{
    layout_node_ptr_t out = layout_node_ptr_t(new layout_node_t());
    INIT_POS(out, ctx);

    out->type = aloe_type_ptr_t(new aloe_type_t(ALOE_TYPE_LAYOUT));

    if (ctx->identifier())
    {
        out->id = walk_identifier(env, ctx->identifier(), ID_TYPE, false);
		out->type->lot_name = out->id->name;
    }

    if (ctx->gtChain())
    {
        out->gt_chain = walk_gt_chain_node(env, ctx->gtChain());
    }

	if (ctx->layoutMemberList())
	{
		out->member_list = walk_layout_member_list(env, ctx->layoutMemberList());
	}

    if (out->id)
    {
        auto prev_node = env->find_id(out->id, true);
        if (prev_node)
        {
            RAISE_LOC("layout %s was already defined", out->id->name.c_str());
        }

        env->register_id(out->id, out);

        out->type->lot_name = out->id->name;

    };

	

    return out;
}

layout_member_ptr_t 
antl4_parser_t::walk_layout_member(environment_ptr_t env, aloeParser::LayoutMemberContext* ctx)
{
	layout_member_ptr_t out = layout_member_ptr_t(new layout_member_node_t());
    INIT_POS(out, ctx);

    if (ctx->identifier()) {
        out->name = ctx->identifier()->getText();
    }
	out->type_node = walk_type(env, ctx->type());
	out->type = out->type_node->type;

    return out;
}

gt_chain_node_ptr_t 
antl4_parser_t::walk_gt_chain_node(environment_ptr_t env, aloeParser::GtChainContext* ctx)
{
	gt_chain_node_ptr_t out = gt_chain_node_ptr_t(new gt_chain_node_t());
	INIT_POS(out, ctx);

	for (auto& member : ctx->gtMember())
	{
		out->members.push_back(walk_gt_chain_member(env, member));
	}

	return out;
}

gt_chain_member_ptr_t
antl4_parser_t::walk_gt_chain_member(environment_ptr_t env, aloeParser::GtMemberContext* ctx)
{
    gt_chain_member_ptr_t out = gt_chain_member_ptr_t(new gt_chain_member_t());
    INIT_POS(out, ctx);

    if (ctx->identifier())
    {
        auto id = walk_identifier(env, ctx->identifier(), ID_TYPE, true);

        auto b = env->find_id(id);

        if (!b && !b->target)
            RAISE_LOC("layout member '%s' was not defined", id->name.c_str());

        out->id = id;
        out->layout = PCAST(layout_node_t, b->target);
        
    }
    else if (ctx->layoutDeclaration())
    {
        auto layout_node = walk_layout_declaration(env, ctx->layoutDeclaration());

        out->layout = layout_node;
    }
    else
    {
        RAISE_LOC("unknown gt chain member '%s'", ctx->getText().c_str());
    }

    return out;
}

layout_member_list_ptr_t
antl4_parser_t::walk_layout_member_list(environment_ptr_t env, aloeParser::LayoutMemberListContext* ctx)
{
    layout_member_list_ptr_t out(new layout_member_list_node_t());
    INIT_POS(out, ctx);

    for (auto& member : ctx->layoutMember())
    {
        auto m = walk_layout_member(env, member);
        out->members_v.push_back(m);
        if (!m->name.empty())
        {
			if (out->members_m.find(m->name) != out->members_m.end())
			{
				RAISE_LOC("layout member '%s' was already defined", m->name.c_str());
			}
            out->members_m[m->name] = m;
        }
	
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

    environment_ptr_t new_env(new scope_modifier_t(SCOPE_FUN_ARGS, env));
    out->fun_params_node = walk_var_list(new_env, ctx->varList());
    for (auto& var : out->fun_params_node->vars_v)
    {
        out->type->fun_param_types.push_back(var.second->type);
    }

    return out;
}


fun_node_ptr_t
antl4_parser_t::walk_fun_declaration( environment_ptr_t env, aloeParser::FunDeclarationContext* ctx)
{
    fun_node_ptr_t out = fun_node_ptr_t(new fun_node_t());
    INIT_POS(out, ctx);
    
	out->is_defined = true;
    out->id         = walk_identifier(env, ctx->identifier(), ID_OBJ,false);
    

    auto prev_node      = out->id  ? env->find_id(out->id) : nullptr;
    auto prev_fun       = prev_node ? PCAST(fun_node_t, prev_node->target) : nullptr;

    // check that function is defined twice
    if (prev_node && prev_fun->is_defined)
    {
        RAISE_LOC("function %s was already defined at (%d:%d)", out->id->name.c_str(), prev_fun->line, prev_fun->pos);
    }

    environment_ptr_t fun_mod(new fun_modifier_t(out, env));
    environment_ptr_t scope_mod(new scope_modifier_t(SCOPE_FUNCTION, fun_mod));
	environment_ptr_t env_mod(new environment_modifier_t(scope_mod));
    
    environment_ptr_t new_env = env_mod;

    out->type_node = walk_fun_type(new_env, ctx->funType());
	out->type = out->type_node->type;

	// check that function is not defined with different type
    if (prev_node)
    {
        if (*prev_fun->type_node->type != *out->type_node->type)
        {
			RAISE_LOC("function %s was already declared with different type at (%d:%d)", out->id->name.c_str(), prev_fun->line, prev_fun->pos);
        }

    }

    env->register_id(out->id, out); // it will mark previous node as ignore
   
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

	out->end_of_fun = marker_node_ptr_t(new marker_node_t());
	INIT_END_POS(out->end_of_fun, ctx);
   
    return out;
}


var_list_node_ptr_t
antl4_parser_t::walk_var_list( environment_ptr_t env, aloeParser::VarListContext* ctx)
{
    var_list_node_ptr_t var_list(new var_list_node_t());
    INIT_POS(var_list, ctx);
    
    bool err = false;
    
    for (auto& varCtx : ctx->varDeclaration())
    {
        auto var_ptr = walk_var(env, varCtx);
		var_list->vars_m[var_ptr->id] = var_ptr;
		var_list->vars_v.push_back(var_id_t(var_ptr->id, var_ptr));
    };
    
    return var_list;
}

var_node_ptr_t 
antl4_parser_t::walk_var(environment_ptr_t env, aloeParser::VarDeclarationContext* ctx)
{
  
    var_node_ptr_t out  = var_node_ptr_t(new var_node_t());
    INIT_POS(out, ctx);

    out->id = walk_identifier(env, ctx->identifier(), ID_OBJ, false);

    if (out->id)
    {
        auto prev_node = env->find_id(out->id,true);
        if (prev_node)
        {
			RAISE_LOC("var %s was already defined", out->id->name.c_str());
        }
    }
    else if (env->curr_scope() != SCOPE_FUN_ARGS)
    {
		RAISE_LOC("variable declaration must have an identifier in this scope");
    }
    
    out->type_node = walk_type(env, ctx->type());
	out->type = out->type_node->type;
   
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
		RAISE_LOC("identifier '%s' was not defined", id_node->name.c_str());
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
		literal_node->type  = make_shared<aloe_type_t>(ALOE_TYPE_ARRAY);
		literal_node->type->arr_type = make_shared<aloe_type_t>(ALOE_TYPE_CHAR);
		literal_node->type->arr_size = (int)std::get<string>(literal_node->value).size() + 1; // +1 for null terminator
		
    }
    else if (ctx->CharacterConstant())
    {
		literal_node->lit_type_id = LIT_CHAR;
        literal_node->value = unescape(ctx->CharacterConstant()->getText())[0];
        literal_node->type  = make_shared<aloe_type_t>(ALOE_TYPE_CHAR);
		
    }
    else
    {
        RAISE_LOC("cannot parse literal %s", ctx->getText().c_str());
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
    if (!env->curr_fun())
    {
		RAISE_LOC("return statement is not allowed outside of function");
    }

    return_node_ptr_t return_node(new return_node_t());
    INIT_POS(return_node, ctx);

    if (ctx->expression())
    {
        return_node->return_expr = walk_expression(env, ctx->expression());

        if (*return_node->return_expr->type != *env->curr_fun()->type->fun_ret_type)
        {
			RAISE_LOC("return expression type '%s' does not match function return type '%s'",
				return_node->return_expr->type->to_str().c_str(),
				env->curr_fun()->type->fun_ret_type->to_str().c_str());
        }
    } 
    else if (env->curr_fun()->type->fun_ret_type->type_id != ALOE_TYPE_VOID) 
    {
		RAISE_LOC("function '%s' must return expression of type '%s'",
			env->curr_fun()->id->name.c_str(),
			env->curr_fun()->type->fun_ret_type->to_str().c_str());
    }

	return return_node;
}

expr_node_ptr_t 
antl4_parser_t::walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx)
{
   expr_node_ptr_t out;

   if (INSTANCE_OF(aloeParser::Expr_identifierContext)) {
       NEW_EXPR_NODE(expr_node, identifier);
     
       expr_node->id = walk_identifier(env, e->identifier(),ID_OBJ,true);
	   expr_node->bn = env->find_id(expr_node->id);
       
       switch (expr_node->bn->target->node_type_id)
       {
       case VAR_NODE:
       {
           expr_node->type = PCAST(var_node_t, expr_node->bn->target)->type_node->type    ;
           expr_node->is_lvalue = true;
           break;
       }
       case FUNCTION_NODE:
       {
           expr_node->type = PCAST(fun_node_t, expr_node->bn->target)->type_node->type;
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
       if (expr_node->fun_expr->type->type_id != ALOE_TYPE_FUNCTION)
       {
           RAISE_LOC("expression '%s' is not of a function type", ctx->getText().c_str());
       }

       auto fun_node_type = expr_node->fun_expr->type;

	   expr_node->type = fun_node_type->fun_ret_type;
	   expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());

       if (expr_node->arg_list->args.size() != fun_node_type->fun_param_types.size())
       {
           RAISE_LOC("function call expects %zu arguments but %zu were provided",
               fun_node_type->fun_param_types.size(),
               fun_node_type->fun_param_types.size());
       }
       
       for (int i=0; i < expr_node->arg_list->args.size(); i++)
       {
		   auto arg     = expr_node->arg_list->args[i];
		   auto param   = fun_node_type->fun_param_types[i];
		   if (*arg->type != *param)
           {
			   RAISE_LOC("function call expects argument %d of type '%s' but argument of type '%s' was provided",
				   i + 1,
				   param->to_str().c_str(),
				   arg->type->to_str().c_str());
           }
       }

       out = expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_indexContext)) {
	   NEW_EXPR_NODE(expr_node, index);
       INIT_POS(expr_node, ctx);

	   expr_node->operand1 = walk_expression(env, e->expression(0));
       expr_node->operand2 = walk_expression(env, e->expression(1));

	   expr_node->type = expr_node->operand1->type->arr_type;
       expr_node->is_lvalue = true;
       out = expr_node;

       
   }
   else if (INSTANCE_OF(aloeParser::Expr_dotContext)) {

       NEW_EXPR_NODE(expr_node, dot);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), ID_OBJ,true);

	   throw;  // dot operator not implemented yet

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_arrowContext)) {
       NEW_EXPR_NODE(expr_node, arrow);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), ID_OBJ,true);

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

       expr_node->type_node = walk_type(env, e->type());
       expr_node->type = expr_node->type_node->type;
       expr_node->operand   = walk_expression(env, e->expression());

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_derefContext)) {
       NEW_EXPR_NODE(expr_node, deref);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
	   check_is_pointer(env, ctx, expr_node->operand, "@");
       expr_node->type = expr_node->operand->type->ptr_type;

       expr_node->is_lvalue = true;
       
       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addressofContext)) {
       NEW_EXPR_NODE(expr_node, addressof);
       INIT_POS(expr_node, ctx);
       
       expr_node->operand = walk_expression(env, e->expression());
       check_is_lvalue(env, ctx, expr_node->operand, "^");

       
       expr_node->type = make_shared<aloe_type_t>(ALOE_TYPE_PTR);
	   expr_node->type->ptr_type = expr_node->operand->type;

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeofexprContext)) {
       NEW_EXPR_NODE(expr_node, sizeofexpr);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
	   expr_node->type = make_shared<aloe_type_t>(ALOE_TYPE_INT); // sizeof operator always returns int

       out = expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeoftypeContext)) {
       NEW_EXPR_NODE(expr_node, sizeoftype);
       INIT_POS(expr_node, ctx);

       expr_node->type_node = walk_type(env, e->type());
	   expr_node->type = expr_node->type_node->type;   

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
       if (!is_arithmetic(expr_node->condition->type->type_id))
       {
           RAISE_LOC("operator '%s' cannot be applied to conditional expressions of type '%s'", 
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
antl4_parser_t::check_type_equality(environment_ptr_t env, antlr4::ParserRuleContext* ctx, aloe_type_ptr_t type1, aloe_type_ptr_t  type2)
{
    if (*type1 != *type2)
    {
        RAISE_LOC("type mismatch: '%s' and '%s'", type1->to_str().c_str(), type2->to_str().c_str());
    };
}

void 
antl4_parser_t::check_expr_type_equality(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr1, expr_node_ptr_t expr2, const char* op_str)
{
    if (*expr1->type != *expr2->type)
    {
        RAISE_LOC("operator '%s' cannot be applied to expressions of different types '%s' and '%s'",
            op_str,
            expr1->type->to_str().c_str(),
            expr2->type->to_str().c_str());
    };
}

void 
antl4_parser_t::check_binary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, binary_expr_node_ptr_t  expr_node, const char* op_str)
{
    if (!is_arithmetic(expr_node->operand1->type->type_id) || !is_arithmetic(expr_node->operand2->type->type_id))
    {
		RAISE_LOC("operator '%s' cannot be applied to expressions of type '%s' and '%s'",
			op_str,
			expr_node->operand1->type->to_str().c_str(),
			expr_node->operand2->type->to_str().c_str());
    }

	check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, op_str);
}

void 
antl4_parser_t::check_unary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, unary_expr_node_ptr_t  expr_node, const char* op_str)
{

    if (!is_arithmetic(expr_node->operand->type->type_id))
    {
		RAISE_LOC("operator '%s' cannot be applied to expression of type '%s'",
			op_str,
			expr_node->operand->type->to_str().c_str());
    }
    
}

void 
antl4_parser_t::check_is_lvalue(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr_node, const char* op_str)
{
    if (expr_node->is_lvalue == false)
    {
		RAISE_LOC("operator '%s' cannot be applied to rvalue expression of type '%s'",
			op_str,
			expr_node->type->to_str().c_str());
    }
}

void 
antl4_parser_t::check_assignment(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t lhs, expr_node_ptr_t rhs)
{
    check_is_lvalue(env, ctx, lhs, "=");

    if (*lhs->type != *rhs->type)
    {
		RAISE_LOC("operator '=' cannot be applied to expressions of type '%s' and '%s'",
			lhs->type->to_str().c_str(),
			rhs->type->to_str().c_str());
    }
}

void 
antl4_parser_t::check_is_pointer(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr_node, const char* op_str)
{
	if (expr_node->type->type_id != ALOE_TYPE_PTR)
	{
		RAISE_LOC("operator '%s' cannot be applied to expression of non-pointer type '%s'",
			op_str,
			expr_node->type->to_str().c_str());
	}
}
