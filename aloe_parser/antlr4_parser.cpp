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


aloe::parse_exeption_t::parse_exeption_t(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
}

bool
antl4_parser_t::parse_from_stream(istream& stream, ast_ptr_t& ast, const string& source_id)
{
    bool success = true;

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
        prog->module_name = walk_identifier(env,ctx->moduleStatement()->identifier(),true,ID_MODULE);
    }

    for (auto& stmt : ctx->declarationStatementList()->declarationStatement())
    {
        try
        {
            if (stmt->varDeclaration() != nullptr)
            {
                prog->decl_statements.push_back(walk_var(env, stmt->varDeclaration()));
            }
            else if (stmt->funDeclaration() != nullptr)
            {
                prog->decl_statements.push_back(walk_func_declaration(env, stmt->funDeclaration()));
            }
            else if (stmt->objectDeclaration() != nullptr)
            {
                prog->decl_statements.push_back(walk_object_declaration(env, stmt->objectDeclaration()));
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


object_node_ptr_t 
antl4_parser_t::walk_object_declaration( environment_ptr_t env, aloeParser::ObjectDeclarationContext* ctx)
{
    environment_ptr_t new_env(new environment_t(env));
    object_node_ptr_t obj (new object_node_t());
    INIT_POS(obj, ctx);

    obj->id         = walk_identifier(env, ctx->identifier(), true, ID_TYPE);
    obj->inh_chain  = walk_chain_declaration(env, ctx->inheritanceChain());
    obj->fields     = walk_var_list(new_env, ctx->varList());
    

    env->register_id(obj->id, obj);
    
    return obj;
}

inh_chain_node_ptr_t
antl4_parser_t::walk_chain_declaration( environment_ptr_t env, aloeParser::InheritanceChainContext* ctx)
{
    inh_chain_node_ptr_t inh_chain(new inh_chain_node_t());
    INIT_POS(inh_chain, ctx);
    
    if (ctx)
    {
        for (auto& typeCtx : ctx->type())
        {
            type_node_ptr_t t = walk_type(env, typeCtx);

            if (t->syn_type != SYN_OBJECT)
            {
                throw parse_exeption_t("%s:%zu:%zu: error: wrong base type '%s' for inheritance", 
                    env->source_id.c_str(), 
                    ctx->getStart()->getLine(), 
                    ctx->getStart()->getStartIndex(), 
                    typeCtx->getText().c_str());
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
        auto obj = walk_object_declaration(env, ctx->objectDeclaration());
        type_node_ptr_t type_node(new type_node_t(SYN_OBJECT, obj));
        INIT_POS(type_node, ctx);
        return type_node;
    }
    else if (ctx->funDeclaration())
    {
        auto obj = walk_func_declaration(env, ctx->funDeclaration());
        type_node_ptr_t type_node(new type_node_t(SYN_OBJECT, obj));
        INIT_POS(type_node, ctx);
        return type_node;
    }
    else if (ctx->builtinType())
    {
        if (ctx->builtinType()->int_())
        {
            type_node_ptr_t type_node(new type_node_t(SYN_INT));
            INIT_POS(type_node, ctx);
            return type_node;
        }
        else if (ctx->builtinType()->void_())
        {
            type_node_ptr_t type_node(new type_node_t(SYN_VOID));
            INIT_POS(type_node, ctx);
            return type_node;
        }
        else if (ctx->builtinType()->char_())
        {
            type_node_ptr_t type_node(new type_node_t(SYN_CHAR));
            INIT_POS(type_node, ctx);
            return type_node;
        }
        else if (ctx->builtinType()->double_())
        {
            type_node_ptr_t type_node(new type_node_t(SYN_DOUBLE));
            INIT_POS(type_node, ctx);
            return type_node;
        }
        else if (ctx->builtinType()->opaque())
        {
            type_node_ptr_t type_node(new type_node_t(SYN_OPAQUE));
            INIT_POS(type_node, ctx);
            return type_node;
        }
        else
        {
            throw parse_exeption_t("%s:%zu:%zu: error: unknown type '%s'",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
				ctx->getText().c_str());
        }
    }
    else if (ctx->identifier())
    {
        identifier_node_ptr_t id_node = walk_identifier(env, ctx->identifier(), false, ID_TYPE);
        type_node_ptr_t tmp_type(new type_node_t(SYN_OBJECT, env->find_id(id_node)));
        tmp_type->id = id_node;

        INIT_POS(tmp_type, ctx);

        node_ptr_t def = env->find_id(id_node);
        if (!def)
        {
            throw parse_exeption_t("%s:%zu:%zu: error: unknown type '%s'",
                env->source_id.c_str(),
                ctx->getStart()->getLine(),
                ctx->getStart()->getStartIndex(),
                ctx->getText().c_str());
        }

        switch (def->type)
        {
        case OBJECT_NODE: {
            type_node_ptr_t out(new type_node_t(SYN_OBJECT, def));
            INIT_POS(out, ctx);
            return out;
        }
        case FUNCTION_NODE: {
            type_node_ptr_t out(new type_node_t(SYN_FUNCTION, def));
            INIT_POS(out, ctx);
            return out;
        }
        default:
        {
            throw parse_exeption_t("%s:%zu:%zu: error: identifer points to wrong type '%s'",
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
    
}

fun_node_ptr_t
antl4_parser_t::walk_func_declaration( environment_ptr_t env, aloeParser::FunDeclarationContext* ctx)
{
    fun_node_ptr_t fun_node = fun_node_ptr_t(new fun_node_t());
    INIT_POS(fun_node, ctx);

    
	fun_node->is_defined = ctx->expect() == nullptr;
    fun_node->id         = walk_identifier(env, ctx->identifier(), true, ID_VAR);
    fun_node->ret_type   = walk_type(env, ctx->funType()->type());

	env->register_id(fun_node->id, fun_node);

    environment_ptr_t new_env(new environment_t(env));
    fun_node->var_list = walk_var_list(new_env, ctx->funType()->varList());

	if (ctx->expect() && ctx->executionBlock())
    {
		throw parse_exeption_t("%s:%zu:%zu: error: function %s was declared as 'expect' but has body", 
            env->source_id.c_str(), 
            ctx->getStart()->getLine(), 
            ctx->getStart()->getStartIndex(), 
            fun_node->id->name.c_str());
    }

    if (!ctx->expect() && !ctx->executionBlock())
    {
        throw parse_exeption_t("%s:%zu:%zu: error: function %s was not declared as 'expect' but has no body", 
            env->source_id.c_str(), 
            ctx->getStart()->getLine(), 
            ctx->getStart()->getStartIndex(), 
            fun_node->id->name.c_str());
    }

    if (fun_node->is_defined)
    {
        fun_node->exec_block = walk_execution_block(new_env, ctx->executionBlock());
    }
    
   
    return fun_node;
}

execution_block_node_ptr_t
antl4_parser_t::walk_execution_block(environment_ptr_t env, aloeParser::ExecutionBlockContext* ctx)
{
    execution_block_node_ptr_t block_node = execution_block_node_ptr_t(new execution_block_node_t());
    INIT_POS(block_node, ctx);

    for (auto& exec_ctx : ctx->executionStatement())
    {
        if (exec_ctx->varDeclaration())
        {
            block_node->exec_statements.push_back(walk_var(env, exec_ctx->varDeclaration()));
        }
        else if (exec_ctx->funDeclaration())
        {
            block_node->exec_statements.push_back(walk_func_declaration(env, exec_ctx->funDeclaration()));
        }
        else if (exec_ctx->objectDeclaration())
        {
            block_node->exec_statements.push_back(walk_object_declaration(env, exec_ctx->objectDeclaration()));
        }
        else if (exec_ctx->expression())
        {
            block_node->exec_statements.push_back(walk_expression(env, exec_ctx->expression()));
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
    INIT_POS(id_node, ctx);

    id_node->name    = ctx->getText() ;
    id_node->id_type = expected_id_type;

    auto already_exists_node_ptr = env->find_id(id_node);
        
    if (id_declaration && already_exists_node_ptr)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: identifier %s was already defined", 
            env->source_id.c_str(), 
            ctx->getStart()->getLine(), 
            ctx->getStart()->getStartIndex(), 
            id_node->name.c_str());
    }
    else if (!id_declaration && !already_exists_node_ptr)
    {
        throw parse_exeption_t("%s:%zu:%zu: error: unknown identifier '%s'", 
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
		literal_node->lit_type = LIT_INT;
        literal_node->value = std::stoi(ctx->DigitSequence()->getText());
    }
    else if (ctx->StringLiteral().size() > 0)
    {
		literal_node->lit_type = LIT_STRING;
        string sf;
        for (auto& s :ctx->StringLiteral())
        {
            sf += s->getText();
        }
        literal_node->value = unescape(sf.substr(1, sf.size() - 2));
    }
    else if (ctx->CharacterConstant())
    {
		literal_node->lit_type = LIT_CHAR;
        literal_node->value = unescape(ctx->CharacterConstant()->getText())[0];
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

expr_node_ptr_t 
antl4_parser_t::walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx)
{

#define INSTANCE_OF(C) C* e = dynamic_cast<C*>(ctx)    

   if (INSTANCE_OF(aloeParser::Expr_identifierContext)) {
       NEW_EXPR_NODE(expr_node, identifier);
       INIT_POS(expr_node, ctx);

       expr_node->id = walk_identifier(env, e->identifier(), false,ID_VAR);
	   expr_node->id_def = env->find_id(expr_node->id);

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
       expr_node->id        = walk_identifier(env, e->identifier(), false, ID_VAR);

       return expr_node;

   }
   else if (INSTANCE_OF(aloeParser::Expr_arrowContext)) {

       NEW_EXPR_NODE(expr_node, arrow);
       INIT_POS(expr_node, ctx);

       expr_node->operand   = walk_expression(env, e->expression());
       expr_node->id        = walk_identifier(env, e->identifier(), false, ID_VAR);

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

