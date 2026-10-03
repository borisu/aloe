#include "pch.h"
#include "base/defs.h"
#include "base/scope_guard.h"
#include "lang/aloe_exception.h"
#include "lang/aloe_type.h"
#include "lang/ast/ast.h"
#include "utils.h"
#include "antlr4_parser.h"
#include "antlr4_parser/parser.h"
#include "lang/ast/expression.h"


using namespace aloe;
using namespace std;
using namespace antlr4;

static int object_id = 0;
static int anonymous_id_counter = 0;

#define E_INSTANCE_OF(C) C* e = dynamic_cast<C*>(ctx)    

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
            type_node->p_type = walk_type(env, parser.type());

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

    for (auto& statement_ctx : ctx->topLevelStatement())
    {
        try
        {
			node_t_ptr statement_node;
            if (statement_ctx->varDeclaration())
            {
				statement_node = walk_var(env, statement_ctx->varDeclaration());
            }
            else if (statement_ctx->funDeclaration())
            {
				statement_node = walk_fun_declaration(env, statement_ctx->funDeclaration());
            }
			else if (statement_ctx->layoutDeclaration())
			{
                statement_node = walk_layout_declaration(env, statement_ctx->layoutDeclaration());
			}
            else if (statement_ctx->expectation())
            {
                walk_expectation(env, statement_ctx->expectation());
            }
			else
			{
				RAISE_LOC("unknown declaration statement '%s'", statement_ctx->getText().c_str());
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
    assert(ctx != nullptr);

    aloe_type_t_ptr type;
    type_proxy_t_ptr out;

    if (E_INSTANCE_OF(aloeParser::Type_intContext)) 
    {
		type = newptr(aloe_type_t,ALOE_TYPE_INT);
    }
    else if (E_INSTANCE_OF(aloeParser::Type_charContext))
    {
		type = newptr(aloe_type_t,ALOE_TYPE_CHAR);
    }
    else if (E_INSTANCE_OF(aloeParser::Type_doubleContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_DOUBLE);
    }
    else if (E_INSTANCE_OF(aloeParser::Type_voidContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_VOID);
	}
    else if (E_INSTANCE_OF(aloeParser::Type_funContext))
    {
        out = walk_fun_type(env, e->funType());
    }
    else if (E_INSTANCE_OF(aloeParser::Type_groupedContext))
    {
		out = walk_type(env, e->type());
    }
    else if (E_INSTANCE_OF(aloeParser::Type_pointerContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_PTR);
		type->ptr->pointee_type(walk_type(env, e->type()));
    }
	else if (E_INSTANCE_OF(aloeParser::Type_arrayContext))
    {
        type = newptr(aloe_type_t, ALOE_TYPE_ARRAY);
        type->arr->elem_type(walk_type(env, e->type()));
		type->arr->size  = e->DigitSequence() ? stoul(e->DigitSequence()->getText()) : -1;
    }
    else if (E_INSTANCE_OF(aloeParser::Type_layoutContext))
    {
		out = walk_layout_declaration(env, e->layoutDeclaration())->p_layout_type;
    }
    else if (E_INSTANCE_OF(aloeParser::Type_identifierContext))
    {
        auto id_node = walk_identifier(env, e->identifier());
		out = env->find_type(id_node);
        ASSERT_LOC(out, "undefined identifier '%s'", id_node->name.c_str());
    }
    else
    {
        RAISE_LOC("unknown type '%s'", ctx->getText().c_str());
    }

    if (!out)
    {
        out = newptr(type_proxy_t, type);
    }

    INIT_POS(out->target, ctx);
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

    out->fun_type(walk_fun_type(env, ctx->funType()));

    auto prev       = env->find_object(out->idt, true);
	if (prev)
	{
        ASSERT_LOC(prev->target->node_type_id == out->node_type_id, "identifier %s was already defined as a different kind at (%d,%d)", out->idt->name.c_str(), prev->target->line,prev->target->pos);

        auto prev_fun  = castptr(fun_node_t, prev->target);
        ASSERT_LOC(*prev_fun->fun_type() == *out->fun_type(), "identifier %s was already defined with a different type at (%d,%d)", out->idt->name.c_str(), prev->target->line, prev->target->pos);
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

	auto idt = walk_identifier(env, ctx->identifier());
    type->layout->name = idt->name;
	type->is_incomplete = true;

    auto prev = env->find_type(idt, true);
    if (prev)
    {
        ASSERT_LOC(*prev->target == *type, "identifier '%s' was already defined with different type at (%d,%d)", idt->name.c_str(), prev->target->line, prev->target->pos);
    }
    else
    {
        env->register_type(idt, type);
    }
}

layout_node_t_ptr
antl4_parser_t::walk_layout_declaration(environment_t_ptr env, aloeParser::LayoutDeclarationContext* ctx)
{
    auto out = newptr(layout_node_t);
    INIT_POS(out, ctx);

    auto type = newptr(aloe_type_t, ALOE_TYPE_LAYOUT);
    INIT_POS(type, ctx);

	type->layout = newptr(layout_info_t);
	type->is_incomplete = true;

    if (ctx->identifier())
    {
		auto idt = walk_identifier(env, ctx->identifier());
        type->layout->name = idt->name;
       
        auto prev = env->find_type(idt, true);

        ASSERT_LOC(!prev || prev->target->type_id == ALOE_TYPE_LAYOUT, "identifier '%s' was already defined as a different kind at (%d,%d)", idt->name.c_str(), prev->target->line, prev->target->pos);

        ASSERT_LOC (!prev || prev->target->is_incomplete, "layout '%s' was already defined at(%d,%d)", idt->name.c_str(), prev->target->line, prev->target->pos);
        
        out->layout_type(env->register_type(idt,type));
    }
    else
    {
        out->layout_type(newptr(type_proxy_t, type));
    }
    
    if (ctx->gtChain())
    {
        out->layout_type()->layout->gt_chain = walk_gt_chain_node(env, ctx->gtChain());
    }

	if (ctx->layoutFieldsList())
	{
		out->layout_type()->layout->fields = walk_layout_member_list(env, ctx->layoutFieldsList());
	}

    type->is_incomplete = false;

    return out;
}

var_t_ptr
antl4_parser_t::walk_layout_member(environment_t_ptr env, aloeParser::LayoutFieldContext* ctx)
{
    auto out = newptr(var_t);
    INIT_POS(out, ctx);
   
    if (ctx->identifier()) {
        out->name = ctx->identifier()->getText();
    }

    out->var_type(walk_type(env, ctx->type()));

    ASSERT(!out->var_type()->is_incomplete, "incomplete type '%s' cannot be used as layout field type.", out->var_type()->name().c_str());

    return out;
}

var_set_t_ptr
antl4_parser_t::walk_layout_member_list(environment_t_ptr env, aloeParser::LayoutFieldsListContext* ctx)
{
    auto out = newptr(var_set_t);
    INIT_POS(out, ctx);

    for (auto& member : ctx->layoutField())
    {
        auto var = walk_layout_member(env, member);

        out->v.push_back(var);

        if (!var->name.empty())
        {
            ASSERT_LOC(out->m.count(var->name)  == 0,"layout member '%s' was already defined at (%d,%d)", var->name.c_str(), out->m[var->name]->line, out->m[var->name]->pos);
            
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

        ASSERT(out->m.count(*gt->gt_type()) == 0, "same type '%s' appears twice in gt chain first at (%d,%d)", gt->gt_type()->name().c_str(), gt->gt_type()->line, gt->gt_type()->pos);

        ASSERT(!gt->gt_type()->is_incomplete, "incomplete type '%s' cannot be used in gt chain", gt->gt_type()->name().c_str());
        
        out->m[*gt->gt_type()] = gt;
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
        auto idt     = walk_identifier(env, ctx->identifier());
        auto prev   = env->find_type(idt);
        
        ASSERT (prev, "layout member '%s' was not defined", idt->name.c_str());
        out->gt_type(prev);
    }
    else if (ctx->layoutDeclaration())
    {
        auto layout_type  = walk_layout_declaration(env, ctx->layoutDeclaration());
        out->gt_type(layout_type->p_layout_type);
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

    type->fun->ret_type(walk_type(env, ctx->type()));

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

    ASSERT_LOC(!prev || prev->target->node_type_id == out->node_type_id, "identifier %s was already defined as a different kind at (%d,%d)", out->idt->name.c_str(), prev->target->line, prev->target->pos);

    auto prev_fun       = prev ? castptr(fun_node_t, prev->target) : nullptr;

    // check that function is not defined twice
    ASSERT_LOC(!prev_fun || !prev_fun->is_defined, "function %s was already defined at (%d:%d)", out->idt->name.c_str(), prev_fun->line, prev_fun->pos);
    
    environment_t_ptr fun_mod(new fun_modifier_t(out, env));
    environment_t_ptr scope_mod(new scope_modifier_t(SCOPE_FUNCTION, fun_mod));
	environment_t_ptr env_mod(new environment_modifier_t(scope_mod));
    
    environment_t_ptr new_env = env_mod;

    out->fun_type(walk_fun_type(new_env, ctx->funType()));

	// check that function is not defined with different type
    ASSERT_LOC( !prev || *prev_fun->fun_type() == *out->fun_type(), "function %s was already declared with different type at (%d:%d)", out->idt->name.c_str(), prev_fun->line, prev_fun->pos);
    
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
    auto out = newptr(var_set_t);
    INIT_POS(out, ctx);
    
    bool err = false;
    
    for (auto& varCtx : ctx->varDeclaration())
    {
        auto var_node = walk_var(env, varCtx);
        var_t_ptr var = var_t_ptr(new var_t());

        var->var_type(var_node->p_var_type);
        if (var_node->idt)
        {
            var->name = var_node->idt->name;
        }

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

    out->idt = walk_identifier(env, ctx->identifier());

    if (out->idt)
    {
        auto prev_node = env->find_object(out->idt,true);

		ASSERT_LOC(!prev_node,  "var %s was already defined", out->idt->name.c_str());
        
    }
    else if (env->curr_scope() != SCOPE_FUN_ARGS)
    {
		RAISE_LOC("variable declaration must have an identifier in this scope");
    }
    
    out->var_type(walk_type(env, ctx->type()));
   
    if (ctx->expression())
    {
        
		ASSERT_LOC(env->curr_scope() != SCOPE_FUN_ARGS, "variable initialization is not allowed in this context");
        
        ASSERT_LOC(env->curr_scope() != SCOPE_GLOBAL || dynamic_cast<aloeParser::Expr_literalContext*>(ctx->expression()),   "only literal expressions are allowed for global variable initialization");

        out->initializer = walk_expression(env, ctx->expression());
		check_type_equality(env, ctx, out->initializer->expr_type(), out->var_type());
    }

    if (out->idt)
    {
        env->register_object(out->idt, out);
    }

    return out;
}

identifier_node_t_ptr  
antl4_parser_t::walk_identifier(environment_t_ptr env, aloeParser::IdentifierContext* ctx)
{
    if (!ctx)
        return identifier_node_t_ptr();

    identifier_node_t_ptr idt_node = identifier_node_t_ptr(new identifier_node_t());
    INIT_POS(idt_node, ctx);

    idt_node->name           = ctx->getText();
    return idt_node;
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
		out->literal_type(newptr(type_proxy_t,newptr(aloe_type_t, ALOE_TYPE_INT)));
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
        out->literal_type(newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_ARRAY)));
		out->literal_type()->arr->elem_type(newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_CHAR)));
		out->literal_type()->arr->size = (int)std::get<string>(out->value).size() + 1; // +1 for null terminator
		
    }
    else if (ctx->CharacterConstant())
    {
		out->lit_type_id = LIT_CHAR;
        out->value = unescape(ctx->CharacterConstant()->getText())[0];
        out->literal_type(newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_CHAR)));
   	
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
    
	ASSERT_LOC(env->curr_fun(), "return statement is not allowed outside of function");
    

    return_node_t_ptr out(new return_node_t());
    INIT_POS(out, ctx);

    if (ctx->expression())
    {
        out->return_expr = walk_expression(env, ctx->expression());

        ASSERT_LOC(*out->return_expr->expr_type() == *env->curr_fun()->fun_type()->fun->ret_type(), "return expression type '%s' does not match function return type '%s'",
			out->return_expr->expr_type()->to_str().c_str(),
			env->curr_fun()->fun_type()->fun->ret_type()->to_str().c_str());
        
    } 
    else 
    {
        ASSERT_LOC(env->curr_fun()->fun_type()->fun->ret_type()->type_id == ALOE_TYPE_VOID, "function '%s' must return expression of type '%s'",
			env->curr_fun()->idt->name.c_str(),
			env->curr_fun()->fun_type()->fun->ret_type()->to_str().c_str());
    }

	return out;
}

expr_node_t_ptr 
antl4_parser_t::walk_expression(environment_t_ptr env, aloeParser::ExpressionContext* ctx)
{
   expr_node_t_ptr out;

   if (E_INSTANCE_OF(aloeParser::Expr_identifierContext)) {
       auto  expr_node = newptr(identifier_expr_node_t);
     
       expr_node->idt = walk_identifier(env, e->identifier());
	   expr_node->ref(env->find_object(expr_node->idt));
       
       switch (expr_node->ref()->node_type_id)
       {
       case VAR_NODE:
       {
           expr_node->p_expr_type = castptr(var_node_t, expr_node->ref())->p_var_type    ;
           expr_node->is_lvalue = true;
           break;
       }
       case FUNCTION_NODE:
       {
           expr_node->p_expr_type = castptr(fun_node_t, expr_node->ref())->p_fun_type;
           break;
       }
       default:
       {
		   RAISE_LOC("identifier '%s' is not a variable or function", expr_node->idt->name.c_str());
       }
       }
       
       out = expr_node;
       
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_literalContext)) {
	   auto expr_node = newptr(literal_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->literal   = walk_literal(env, e->literal());
	   expr_node->p_expr_type = expr_node->literal->p_literal_type;
      
       out = expr_node;
       
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_bracketedContext)) {
      
       out = walk_expression(env, e->expression());
       
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_sfxplusplusContext)) {
       auto expr_node = newptr(sfxplusplus_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->p_expr_type = expr_node->operand->p_expr_type;
       expr_node->is_lvalue = true;

       check_is_lvalue(env, ctx, expr_node->operand, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");
       

       out = expr_node;
       
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_sfxminminContext)) {
       auto expr_node = newptr(sfxminmin_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->p_expr_type = expr_node->operand->p_expr_type;
       expr_node->is_lvalue = true;
      
       check_is_lvalue(env, ctx, expr_node->operand, "--");
       check_unary_arithmetic(env, ctx, expr_node, "--");
   
       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_funcallContext)) {
       auto expr_node = newptr(funcall_expr_node_t);
       INIT_POS(expr_node, ctx);

	   expr_node->fun_expr = walk_expression(env, e->expression());
       
       ASSERT_LOC(expr_node->fun_expr->expr_type()->type_id == ALOE_TYPE_FUNCTION, "expression '%s' is not of a function type", ctx->getText().c_str());
       

       auto fun_node_type = expr_node->fun_expr->p_expr_type;

	   expr_node->p_expr_type = fun_node_type->target->fun->p_ret_type;
	   expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());

       
       ASSERT_LOC(expr_node->arg_list->args.size() == fun_node_type->target->fun->params->v.size(), "function call expects %zu arguments but %zu were provided",
            fun_node_type->target->fun->params->v.size(),
            fun_node_type->target->fun->params->v.size());
       
       
       for (int i=0; i < expr_node->arg_list->args.size(); i++)
       {
		   auto arg     = expr_node->arg_list->args[i];
		   auto param   = fun_node_type->target->fun->params->v[i];
		   
		   ASSERT_LOC(*arg->expr_type() == *param->var_type(), "function call expects argument %d of type '%s' but argument of type '%s' was provided",
			    i + 1,
			    param->name.c_str(),
			    arg->expr_type()->to_str().c_str());
           
       }

       out = expr_node;
       
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_indexContext)) {
	   auto expr_node = newptr(index_expr_node_t);
       INIT_POS(expr_node, ctx);

	   expr_node->operand1 = walk_expression(env, e->expression(0));
       expr_node->operand2 = walk_expression(env, e->expression(1));

	   expr_node->p_expr_type = expr_node->operand1->expr_type()->arr->p_elem_type;
       expr_node->is_lvalue = true;
       out = expr_node;

       
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_dotContext)) {

       auto expr_node = newptr(dot_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->idt       = walk_identifier(env, e->identifier());

	   throw;  // dot operator not implemented yet

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_arrowContext)) {
       auto expr_node = newptr(arrow_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->idt       = walk_identifier(env, e->identifier());

	   throw;  // TBD: pointer dereference not implemented yet

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_preplusplusContext)) {
       auto expr_node = newptr(preplusplus_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->p_expr_type = expr_node->operand->p_expr_type;
       
       
       check_is_lvalue(env, ctx, expr_node->operand, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_preminminContext)) {
       auto expr_node = newptr(preminmin_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->p_expr_type = expr_node->operand->p_expr_type;


       check_is_lvalue(env, ctx, expr_node->operand, "++");
       check_unary_arithmetic(env, ctx, expr_node, "++");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_plusContext)) {
       auto expr_node = newptr(plus_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->p_expr_type = expr_node->operand->p_expr_type;

       check_unary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_minContext)) {
       auto expr_node = newptr(min_expr_node_t);
       INIT_POS(expr_node, ctx);

       
       expr_node->p_expr_type = expr_node->operand->p_expr_type;

       check_unary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;
       
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_notContext)) {
       auto expr_node = newptr(not_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->p_expr_type = expr_node->operand->p_expr_type;

       check_unary_arithmetic(env, ctx, expr_node, "!");

       out = expr_node;
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_bwsnotContext)) {
       auto expr_node = newptr(bwsnot_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
       expr_node->p_expr_type = expr_node->operand->p_expr_type;

       check_unary_arithmetic(env, ctx, expr_node, "!");
       
       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_castContext)) {
       auto expr_node = newptr(cast_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->p_expr_type = walk_type(env, e->type());
       expr_node->operand   = walk_expression(env, e->expression());

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_derefContext)) {
       auto expr_node = newptr(deref_expr_node_t);
       INIT_POS(expr_node, ctx);
       

       expr_node->operand = walk_expression(env, e->expression());
	   check_is_pointer(env, ctx, expr_node->operand, "@");
       expr_node->p_expr_type = expr_node->operand->expr_type()->ptr->p_ptr_type;

       expr_node->is_lvalue = true;
       
       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_addressofContext)) {
       auto expr_node = newptr(addressof_expr_node_t);
       INIT_POS(expr_node, ctx);
       
       expr_node->operand = walk_expression(env, e->expression());
       check_is_lvalue(env, ctx, expr_node->operand, "^");

       
       expr_node->p_expr_type = newptr(type_proxy_t,newptr (aloe_type_t,ALOE_TYPE_PTR));
	   expr_node->expr_type()->ptr->pointee_type(expr_node->operand->p_expr_type);

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_sizeofexprContext)) {
       auto expr_node = newptr(sizeofexpr_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());
	   expr_node->p_expr_type = newptr(type_proxy_t, newptr(aloe_type_t, ALOE_TYPE_INT)); // sizeof operator always returns int

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_sizeoftypeContext)) {
       auto expr_node = newptr(sizeoftype_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->p_expr_type = walk_type(env, e->type());
       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_multContext)) {
        auto expr_node = newptr(mult_expr_node_t);
        INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "*");
       check_binary_arithmetic(env,  ctx, expr_node, "*");

       out = expr_node;
       

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_divContext)) {
       auto expr_node = newptr(div_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       
       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "/");
       check_binary_arithmetic(env, ctx, expr_node, "/");

       out = expr_node;
       

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_modContext)) {
       auto expr_node = newptr(mod_expr_node_t);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       
       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "%");
       check_binary_arithmetic(env, ctx, expr_node, "%");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_addContext)) {
       auto expr_node = newptr(add_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "+");
       check_binary_arithmetic(env, ctx, expr_node, "+");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_subContext)) {
       auto expr_node = newptr(sub_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);
       
       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "-");
       check_binary_arithmetic(env, ctx, expr_node, "-");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_shiftleftContext)) {
       auto expr_node = newptr(shiftleft_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<<");
       check_binary_arithmetic(env, ctx, expr_node, "<<");

       out = expr_node;


   }
   else if (E_INSTANCE_OF(aloeParser::Expr_shiftrightContext)) {
       auto expr_node = newptr(shiftright_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">>");
       check_binary_arithmetic(env, ctx, expr_node, ">>");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_lessContext)) {
       auto expr_node = newptr(less_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<");
       check_binary_arithmetic(env, ctx, expr_node, "<");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_lesseeqContext)) {
       auto expr_node = newptr(lesseeq_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "<=");
       check_binary_arithmetic(env, ctx, expr_node, "<=");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_moreContext)) {
       auto expr_node = newptr(more_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">");
       check_binary_arithmetic(env, ctx, expr_node, ">");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_moreeqContext)) {
       auto expr_node = newptr(moreeq_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, ">=");
       check_binary_arithmetic(env, ctx, expr_node, ">=");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_logicaleqContext)) {
       auto expr_node = newptr(logicaleq_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "==");
       check_binary_arithmetic(env, ctx, expr_node, "==");

       out = expr_node;


   }
   else if (E_INSTANCE_OF(aloeParser::Expr_noteqContext)) {
       auto expr_node = newptr(noteq_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "!=");
       check_binary_arithmetic(env, ctx, expr_node, "!=");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_andContext)) {
       auto expr_node = newptr(and_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "&");
       check_binary_arithmetic(env, ctx, expr_node, "&");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_xorContext)) {
       auto expr_node = newptr(xor_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "^");
       check_binary_arithmetic(env, ctx, expr_node, "^");

       out = expr_node;
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_orContext)) {
       auto expr_node = newptr(or_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "|");
       check_binary_arithmetic(env, ctx, expr_node, "|");
       
       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_logicalandContext)) {
       auto expr_node = newptr(logicaland_expr_node_t);
	   INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "&&");
       check_binary_arithmetic(env, ctx, expr_node, "&&");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_logicalorContext)) {
       auto expr_node = newptr(logicalor_expr_node_t);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       expr_node->expr_type(expr_node->operand1->p_expr_type);

       check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, "||");
       check_binary_arithmetic(env, ctx, expr_node, "||");

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_ternaryContext)) {
       auto expr_node = newptr(ternary_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->condition  = walk_expression(env, e->expression()[0]);
       expr_node->true_expr  = walk_expression(env, e->expression()[1]);
       expr_node->false_expr = walk_expression(env, e->expression()[2]);

       expr_node->is_lvalue = expr_node->true_expr->is_lvalue && expr_node->false_expr->is_lvalue;
       expr_node->p_expr_type = expr_node->true_expr->p_expr_type;

       check_expr_type_equality(env, ctx, expr_node->true_expr, expr_node->false_expr, "?");
       
       ASSERT_LOC(is_arithmetic(expr_node->condition->expr_type()->type_id), "operator '%s' cannot be applied to conditional expressions of type '%s'",
            "?", 
            expr_node->condition->expr_type()->to_str().c_str());
       

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_assignContext)) {
       auto expr_node = newptr(assign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);
       
       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;


   }
   else if (E_INSTANCE_OF(aloeParser::Expr_addassignContext)) {
       auto expr_node = newptr(addassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "+=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
              
       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_subassignContext)) {
       auto expr_node = newptr(subassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "-=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_multassignContext)) {
       auto expr_node = newptr(multassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "*=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_divassignContext)) {
       auto expr_node = newptr(divassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "/=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_modassignContext)) {
       auto expr_node = newptr(modassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);


	   check_binary_arithmetic(env, ctx, expr_node, "%=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_shiftleftassignContext)) {
       auto expr_node = newptr(shiftleftassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "<<=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_shiftrightassignContext)) {
       auto expr_node = newptr(shiftrightassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, ">>=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_andassignContext)) {
       auto expr_node = newptr(andassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "&=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_xorassignContext)) {
       auto expr_node = newptr(xorassign_expr_node_t);
	   INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "^=");
	   check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;
   }
   else if (E_INSTANCE_OF(aloeParser::Expr_orassignContext)) {
       auto expr_node = newptr(orassign_expr_node_t);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

	   check_binary_arithmetic(env, ctx, expr_node, "|=");
       check_assignment(env, ctx, expr_node->operand1, expr_node->operand2);

       expr_node->expr_type(expr_node->operand1->p_expr_type);
       

       out = expr_node;

   }
   else if (E_INSTANCE_OF(aloeParser::Expr_commaContext)) {
       auto expr_node = newptr(comma_expr_node_t);
       INIT_POS(expr_node, ctx);

       if (e->argumentExpressionList())
       {
           expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());
       }
       else
       {
           throw;
       }

       expr_node->p_expr_type = expr_node->arg_list->args.back()->p_expr_type;
       expr_node->is_lvalue = expr_node->arg_list->args.back()->is_lvalue;

       out = expr_node;
   }
 
   INIT_POS(out, ctx);
 
   return out;

}

void
antl4_parser_t::check_type_equality(environment_t_ptr env, antlr4::ParserRuleContext* ctx, aloe_type_t_ptr type1, aloe_type_t_ptr  type2)
{
    ASSERT_LOC(
        *type1 == *type2, 
        "type mismatch: '%s' and '%s'", 
        type1->to_str().c_str(), 
        type2->to_str().c_str());
}

void 
antl4_parser_t::check_expr_type_equality(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr1, expr_node_t_ptr expr2, const char* op_str)
{
    ASSERT_LOC(
        *expr1->expr_type() == *expr2->expr_type(), 
        "operator '%s' cannot be applied to expressions of different types '%s' and '%s'",
        op_str,
        expr1->expr_type()->to_str().c_str(),
        expr2->expr_type()->to_str().c_str());
}

void 
antl4_parser_t::check_binary_arithmetic(environment_t_ptr env, aloeParser::ExpressionContext* ctx, binary_expr_node_t_ptr  expr_node, const char* op_str)
{
    ASSERT_LOC(
        is_arithmetic(expr_node->operand1->expr_type()->type_id) &&  is_arithmetic(expr_node->operand2->expr_type()->type_id), 
        "operator '%s' cannot be applied to expressions of type '%s' and '%s'",
		op_str,
		expr_node->operand1->expr_type()->to_str().c_str(),
		expr_node->operand2->expr_type()->to_str().c_str());

	check_expr_type_equality(env, ctx, expr_node->operand1, expr_node->operand2, op_str);
}

void 
antl4_parser_t::check_unary_arithmetic(environment_t_ptr env, aloeParser::ExpressionContext* ctx, unary_expr_node_t_ptr  expr_node, const char* op_str)
{
    ASSERT_LOC(
        is_arithmetic(expr_node->operand->expr_type()->type_id), 
        "operator '%s' cannot be applied to expression of type '%s'",
		op_str,
		expr_node->operand->expr_type()->to_str().c_str());
}

void 
antl4_parser_t::check_is_lvalue(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr_node, const char* op_str)
{
    ASSERT_LOC(
        expr_node->is_lvalue == true, 
        "operator '%s' cannot be applied to rvalue expression of type '%s'", op_str, expr_node->expr_type()->to_str().c_str());

}

void 
antl4_parser_t::check_assignment(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr lhs, expr_node_t_ptr rhs)
{
    check_is_lvalue(env, ctx, lhs, "=");

    ASSERT_LOC(
        *lhs->expr_type() == *rhs->expr_type(), 
        "operator '=' cannot be applied to expressions of type '%s' and '%s'",
		lhs->expr_type()->to_str().c_str(),
		rhs->expr_type()->to_str().c_str());
}

void 
antl4_parser_t::check_is_pointer(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr_node, const char* op_str)
{
    ASSERT_LOC(
        expr_node->expr_type()->type_id == ALOE_TYPE_PTR, "operator '%s' cannot be applied to expression of non-pointer type '%s'",
		op_str,
		expr_node->expr_type()->to_str().c_str());
}


