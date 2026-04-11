#include "pch.h"
#include "antlr4_parser.h"
#include "parse_exception.h"
#include "utils.h"
#include "aloe/defs.h"

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

antl4_parser_t::antl4_parser_t()
{
    syntax_error_occurred = false;

    INT_NODE.reset(new builtin_node_t(BIT_INT));
    CHAR_NODE.reset(new builtin_node_t(BIT_CHAR));
    DOUBLE_NODE.reset(new builtin_node_t(BIT_DOUBLE));
    OPAQUE_NODE.reset(new builtin_node_t(BIT_OPAQUE));
    VOID_NODE.reset(new builtin_node_t(BIT_VOID));

	INT_NODE_BRIDGE.reset(new bridge_t(INT_NODE));
	CHAR_NODE_BRIDGE.reset(new bridge_t(CHAR_NODE));
	DOUBLE_NODE_BRIDGE.reset(new bridge_t(DOUBLE_NODE));
	OPAQUE_NODE_BRIDGE.reset(new bridge_t(OPAQUE_NODE));
	VOID_NODE_BRIDGE.reset(new bridge_t(VOID_NODE));
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
    node->line = (int) ctx->getStart()->getLine(); \
    node->pos  = (int)ctx->getStart()->getStartIndex();

prog_node_ptr_t
antl4_parser_t::walk_prog(environment_ptr_t env, aloeParser::ProgContext* ctx)
{
    prog_node_ptr_t prog(new prog_node_t());
    INIT_POS(prog, ctx);

	bool res = true;

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
                prog->decl_statements.push_back(walk_var(env, stmt->varDeclaration()));
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


base_type_ptr_t
antl4_parser_t::walk_base_type(environment_ptr_t env, aloeParser::BaseTypeContext* ctx)
{
    base_type_ptr_t out;

    if (ctx->funType())
    {
        auto fun_type = walk_fun_type(env, ctx->funType());
        out.reset(new base_type_t(TT_FUNCTION, bridge_ptr_t(new bridge_t(fun_type))));
    }
    else if (ctx->builtinType())
    {
        auto builtin_type_node = walk_built_in_type(env, ctx->builtinType());
        out.reset(new base_type_t(TT_BUILTIN, bridge_ptr_t(new bridge_t(builtin_type_node))));
    }
    else if (ctx->identifier())
    {
        identifier_node_ptr_t id_node = walk_identifier(env, ctx->identifier(), ID_TYPE,true);
        bridge_ptr_t bridge = env->find_id(id_node);
        
        switch (bridge->target->node_type_id)
        {
        case FUN_TYPE_NODE: {
            out.reset(new base_type_t(TT_FUNCTION, bridge));
            break;
        }
        default:
        {
            throw parse_exeption_t("%s:%zu:%zu: error: identifier points to wrong type '%s'",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                ctx->getText().c_str());
        }
        }
    }
    else
    {
        throw parse_exeption_t("%s:%zu:%zu: error: uknown type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            ctx->getText().c_str());
    };

    INIT_POS(out, ctx);
    return out;
}

type_node_ptr_t
antl4_parser_t::walk_type( environment_ptr_t env, aloeParser::TypeContext* ctx, int ref_count)
{
    if (ctx->baseType())
    {
        base_type_ptr_t base  = walk_base_type(env, ctx->baseType());

        type_node_ptr_t out(new type_node_t(ref_count));
		out->base_type = base;

        return out;
    }
    else if (ctx->type())
    {
		return walk_type(env, ctx->type(), ref_count + 1);
    }
    else
    {
        throw parse_exeption_t("%s:%zu:%zu: error: error parsing type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
			ctx->getText().c_str());
    }
}


builtin_node_ptr_t 
antl4_parser_t::walk_built_in_type(environment_ptr_t env, aloeParser::BuiltinTypeContext* ctx)
{
	builtin_node_ptr_t bit_node;

    if (ctx->int_())
    {
        bit_node = INT_NODE;
    }
    else if (ctx->void_())
    {
        bit_node = VOID_NODE;
    }
    else if (ctx->char_())
    {
        bit_node = CHAR_NODE;
    }
    else if (ctx->double_())
    {
        bit_node = DOUBLE_NODE;
    }
    else if (ctx->opaque())
    {
        bit_node = OPAQUE_NODE;
    }
    else
    {
        throw parse_exeption_t("%s:%zu:%zu: error: unknown type '%s'",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            ctx->getText().c_str());
    }

    INIT_POS(bit_node, ctx);
    return bit_node;
}

fun_type_node_ptr_t
antl4_parser_t::walk_fun_type(environment_ptr_t env, aloeParser::FunTypeContext* ctx)
{
    fun_type_node_ptr_t fun_type(new fun_type_node_t());
    INIT_POS(fun_type, ctx);
    fun_type->ret_type = walk_type(env, ctx->type());
    fun_type->var_list = walk_var_list(env, ctx->varList());
	return fun_type;
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
    fun_node->id         = walk_identifier(env, ctx->identifier(), ID_NONTYPE,false);

    auto prev_node      = env->find_id(fun_node->id);
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
    fun_node->fun_type = walk_fun_type(new_env, ctx->funType());

	// check that function is not defined with different type
    if (prev_node)
    {
        if (*(prev_fun->fun_type) != *(fun_node->fun_type))
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
        fun_node->exec_block = walk_execution_block(new_env, ctx->executionBlock());
    }
   
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
            block_node->exec_statements.push_back(walk_var(env, exec_ctx->varDeclaration()));
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
antl4_parser_t::walk_var_list( environment_ptr_t env, aloeParser::VarListContext* ctx)
{
    var_list_node_ptr_t var_list(new var_node_list_t());
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
  
    var_node_ptr_t var_node  = var_node_ptr_t(new var_node_t());
    INIT_POS(var_node, ctx);

    var_node->id = walk_identifier(env, ctx->identifier(), ID_NONTYPE, false);

    auto prev_node = env->find_id(var_node->id);
    if (prev_node)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: var %s was already defined",
            env->source_id.c_str(),
            ctx->getStart()->getLine(),
            ctx->getStart()->getStartIndex(),
            var_node->id->name.c_str());
    }

    var_node->type = walk_type(env, ctx->type());

    if (ctx->literal())
    {
        var_node->initializer = walk_literal(env, ctx->literal());
        if (*var_node->initializer->type != *var_node->type)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: cannot initialize variable of type '%s' with literal of type '%s'",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                ctx->type()->getText().c_str(),
				ctx->literal()->getText().c_str());
        }
    }
   
    env->register_id(var_node->id, var_node);

    return var_node;
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
		literal_node->type->base_type->node_type_id = BASE_TYPE_NODE;
		literal_node->type->base_type->type_type_id = TT_BUILTIN;
        literal_node->type->base_type->ast_def = INT_NODE_BRIDGE;
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
        literal_node->type->base_type->node_type_id = BASE_TYPE_NODE;
        literal_node->type->base_type->ast_def = CHAR_NODE_BRIDGE;
        literal_node->type->base_type->type_type_id = TT_BUILTIN;
		literal_node->type->ref_count = 1; // string literal is char pointer
    }
    else if (ctx->CharacterConstant())
    {
		literal_node->lit_type_id = LIT_CHAR;
        literal_node->value = unescape(ctx->CharacterConstant()->getText())[0];
		literal_node->type->base_type->node_type_id = BASE_TYPE_NODE;
        literal_node->type->base_type->type_type_id = TT_BUILTIN;
        literal_node->type->base_type->ast_def = CHAR_NODE_BRIDGE;
    }
    else if (ctx->pointerToInt())
    {
        literal_node->lit_type_id = LIT_POINTER_INT;
        literal_node->value = std::stoi(ctx->pointerToInt()->DigitSequence()->getText());
        literal_node->type->base_type->node_type_id = BASE_TYPE_NODE;
        literal_node->type->base_type->type_type_id = TT_BUILTIN;
        literal_node->type->base_type->ast_def = INT_NODE_BRIDGE;
		literal_node->type->ref_count = ctx->pointerToInt()->pl_pfx.size(); // pointer literal is int pointer

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
    return_node_ptr_t return_node(new return_node_t());
    INIT_POS(return_node, ctx);
    if (ctx->expression())
    {
        return_node->return_expr = walk_expression(env, ctx->expression());
    }
	return return_node;
}

expr_node_ptr_t 
antl4_parser_t::walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx)
{

#define INSTANCE_OF(C) C* e = dynamic_cast<C*>(ctx)    

   if (INSTANCE_OF(aloeParser::Expr_identifierContext)) {
       NEW_EXPR_NODE(expr_node, identifier);
       INIT_POS(expr_node, ctx);

       expr_node->id = walk_identifier(env, e->identifier(),ID_NONTYPE,true);
	   expr_node->ast_def = env->find_id(expr_node->id);

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_literalContext)) {
       NEW_EXPR_NODE(expr_node, literal);
       INIT_POS(expr_node, ctx);

       expr_node->literal  = walk_literal(env, e->literal());

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_bracketedContext)) {
      
       return walk_expression(env, e->expression());
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_sfxplusplusContext)) {
       NEW_EXPR_NODE(expr_node, sfxplusplus);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_sfxminminContext)) {

       NEW_EXPR_NODE(expr_node, sfxminmin);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_funcallContext)) {
       NEW_EXPR_NODE(expr_node, funcall);
       INIT_POS(expr_node, ctx);

	   expr_node->function = walk_expression(env, e->expression());

       if (e->argumentExpressionList())
       {
		   expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());
       }

       return expr_node;
       
   }
   else if (INSTANCE_OF(aloeParser::Expr_indexContext)) {
	   NEW_EXPR_NODE(expr_node, index);
       INIT_POS(expr_node, ctx);

	   expr_node->operand1 = walk_expression(env, e->expression(0));
       expr_node->operand2 = walk_expression(env, e->expression(0));

       return expr_node;
   }
   else if (INSTANCE_OF(aloeParser::Expr_dotContext)) {

       NEW_EXPR_NODE(expr_node, dot);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), ID_NONTYPE,true);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_arrowContext)) {

       NEW_EXPR_NODE(expr_node, arrow);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), ID_NONTYPE,true);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preplusplusContext)) {
       NEW_EXPR_NODE(expr_node, preplusplus);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_preminminContext)) {
       NEW_EXPR_NODE(expr_node, preminmin);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_plusContext)) {
       NEW_EXPR_NODE(expr_node, plus);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_minContext)) {
       NEW_EXPR_NODE(expr_node, min);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_notContext)) {
       NEW_EXPR_NODE(expr_node, not);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_bwsnotContext)) {
       NEW_EXPR_NODE(expr_node, bwsnot);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_castContext)) {
       NEW_EXPR_NODE(expr_node, cast);
       INIT_POS(expr_node, ctx);

       expr_node->type_node = walk_type(env, e->type());
       expr_node->operand   = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_derefContext)) {
       NEW_EXPR_NODE(expr_node, deref);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addressofContext)) {
       NEW_EXPR_NODE(expr_node, addressof);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeofexprContext)) {
       NEW_EXPR_NODE(expr_node, sizeofexpr);
       INIT_POS(expr_node, ctx);

       expr_node->operand = walk_expression(env, e->expression());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_sizeoftypeContext)) {
       NEW_EXPR_NODE(expr_node, sizeoftype);
       INIT_POS(expr_node, ctx);

       expr_node->type_node = walk_type(env, e->type());

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_multContext)) {
        NEW_EXPR_NODE(expr_node, mult);
        INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_divContext)) {
       NEW_EXPR_NODE(expr_node, div);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_modContext)) {
       NEW_EXPR_NODE(expr_node, mod);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addContext)) {
       NEW_EXPR_NODE(expr_node, add);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_subContext)) {
       NEW_EXPR_NODE(expr_node, sub);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftleftContext)) {
       NEW_EXPR_NODE(expr_node, shiftleft);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftrightContext)) {
       NEW_EXPR_NODE(expr_node, shiftright);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lessContext)) {
       NEW_EXPR_NODE(expr_node, less);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_lesseeqContext)) {
       NEW_EXPR_NODE(expr_node, lesseeq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreContext)) {
       NEW_EXPR_NODE(expr_node, more);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_moreeqContext)) {
       NEW_EXPR_NODE(expr_node, moreeq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicaleqContext)) {
       NEW_EXPR_NODE(expr_node, logicaleq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_noteqContext)) {
       NEW_EXPR_NODE(expr_node, noteq);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_andContext)) {
       NEW_EXPR_NODE(expr_node, and);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_xorContext)) {
       NEW_EXPR_NODE(expr_node, xor);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_orContext)) {
       NEW_EXPR_NODE(expr_node, or);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalandContext)) {
       NEW_EXPR_NODE(expr_node, logicaland);
	   INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_logicalorContext)) {
       NEW_EXPR_NODE(expr_node, logicalor);
       INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_ternaryContext)) {
       NEW_EXPR_NODE(expr_node, ternary);
       INIT_POS(expr_node, ctx);

       expr_node->condition  = walk_expression(env, e->expression()[0]);
       expr_node->true_expr  = walk_expression(env, e->expression()[1]);
       expr_node->false_expr = walk_expression(env, e->expression()[2]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_assignContext)) {
       NEW_EXPR_NODE(expr_node, assign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_addassignContext)) {
       NEW_EXPR_NODE(expr_node, addassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_subassignContext)) {
       NEW_EXPR_NODE(expr_node, subassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_multassignContext)) {
       NEW_EXPR_NODE(expr_node, multassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_divassignContext)) {
       NEW_EXPR_NODE(expr_node, divassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_modassignContext)) {
       NEW_EXPR_NODE(expr_node, modassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftleftassignContext)) {
       NEW_EXPR_NODE(expr_node, shiftleftassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;


   }
   else if (INSTANCE_OF(aloeParser::Expr_shiftrightassignContext)) {
       NEW_EXPR_NODE(expr_node, shiftrightassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_andassignContext)) {
       NEW_EXPR_NODE(expr_node, andassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_xorassignContext)) {
       NEW_EXPR_NODE(expr_node, xorassign);
	   INIT_POS(expr_node, ctx);


       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_orassignContext)) {
       NEW_EXPR_NODE(expr_node, orassign);
       INIT_POS(expr_node, ctx);

       expr_node->operand1 = walk_expression(env, e->expression()[0]);
       expr_node->operand2 = walk_expression(env, e->expression()[1]);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_commaContext)) {
       NEW_EXPR_NODE(expr_node, comma);
       INIT_POS(expr_node, ctx);

       if (e->argumentExpressionList())
       {
           expr_node->arg_list = walk_arg_list(env, e->argumentExpressionList());
       }

       return expr_node;

   }
 
   return nullptr;

#undef INSTANCE_OF
}

