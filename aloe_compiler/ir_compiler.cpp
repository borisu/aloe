#include "pch.h"
#include "aloe\compiler.h"
#include "aloe\defs.h"
#include "compiler_exception.h"
#include "ir_compiler.h"
#include "fun_desc.h"

using namespace aloe;
using namespace llvm;
using namespace llvm::dwarf;



#define TYPE_NODE_KEY(N) type_cache_key_t(N->ref_count, N->def)
#define NODE_KEY(N) type_cache_key_t(0, N)

#define TYPE_KEY(T) di_cache_key_t(T->ref_count, T->irt)
#define DI_KEY(IR) di_cache_key_t(0, IR)

compiler_ptr_t
aloe::create_compiler()
{
    return compiler_ptr_t(new llvmir_compiler_t());
}
    
bool 
llvmir_compiler_t::compile(
    ast_ptr_t ast,
    ostream& out)
{
    LLVMContext ctx;
    Module module(ast->source_id, ctx);

    DIBuilder dib(module);
    std::filesystem::path pth(ast->source_id);
    DIFile* di_file = dib.createFile(pth.filename().string(), pth.parent_path().string());


    DICompileUnit* cu = dib.createCompileUnit(
        DW_LANG_C,   
        di_file,
        "aloe-frontend",    // producer
        false,              // isOptimized
        "",                 // flags
        0                   // runtime version
    );

    module.addModuleFlag(Module::Warning, "CodeView", 1);
    module.addModuleFlag(Module::Warning, "Dwarf Version", 4);
    module.addModuleFlag(Module::Warning, "Debug Info Version", DEBUG_METADATA_VERSION);

    IRBuilder<> ir(ctx);
    
    compiler_ctx_t compiler_ctx;

    compiler_ctx.llvm_ctx       = &ctx;
	compiler_ctx.llvm_ir        = &ir;
	compiler_ctx.llvm_module    = &module;
	compiler_ctx.llvm_di        = &dib;
    compiler_ctx.llvm_di_file   = di_file;

    compiler_ctx.ast = ast;

	bool res = false;
    try
    {
        walk_prog(&compiler_ctx, ast->prog);
		res = true;

        dib.finalize();

        // Print LLVM IR
        llvm::raw_os_ostream  llvmOs(out);
        module.print(llvmOs, nullptr);
    }
    catch (std::exception& e)
    {
        loginl("%s", e.what());
    }

    return res;
}

DIType* 
llvmir_compiler_t::get_dit(di_cache_key_t key, DIType* dit)
{
	auto ir = di_cache.find(key);
	if (ir != di_cache.end())
        return ir->second;

    di_cache[key] = dit;
    return dit;
	
}


type_ptr_t
llvmir_compiler_t::walk_built_in(compiler_ctx_t* ctx, builtin_node_ptr_t node)
{
    if (type_cache.find(type_cache_key_t{ 0, node }) != type_cache.end())
        return type_cache[type_cache_key_t{ 0, node }];

    type_ptr_t type(new type_t());

    switch (node->bit_type)
    {
    case BIT_INT:
    {
        type->irt = Type::getInt32Ty(*ctx->llvm_ctx);
        type->dit = get_dit(TYPE_KEY(type), ctx->llvm_di->createBasicType("int", 32, dwarf::DW_ATE_signed));

        break;
    }
    case BIT_CHAR:
    {
        type->irt = Type::getInt8Ty(*ctx->llvm_ctx);
        type->dit = get_dit(TYPE_KEY(type), ctx->llvm_di->createBasicType("char", 8, dwarf::DW_ATE_signed_char));
        break;
    }
    case BIT_VOID:
    case BIT_OPAQUE:
    {
        type->irt = Type::getVoidTy(*ctx->llvm_ctx);
        type->dit = get_dit(TYPE_KEY(type), nullptr);

        break;
    }
    case BIT_DOUBLE:
    {
        type->irt = Type::getDoubleTy(*ctx->llvm_ctx);
        type->dit = get_dit(TYPE_KEY(type), ctx->llvm_di->createBasicType("double",64,dwarf::DW_ATE_float));
        break;
    }
    default:
    {
        throw compiler_exeption_t("%s:%zu:%zu: error: unknown built int type '%d'",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->bit_type);
    }
    }

    return type;
}

type_ptr_t
llvmir_compiler_t::walk_type(compiler_ctx_t* ctx, type_node_ptr_t node)
{

    if (type_cache.find(TYPE_NODE_KEY(node)) != type_cache.end())
		return type_cache[TYPE_NODE_KEY(node)];

	type_ptr_t type(new type_t());

    switch (node->tt)
    {
    case TT_BUILTIN:
    {
        type = walk_built_in(ctx, static_pointer_cast<builtin_node_t>(node->def));
        break;
    }
    case TT_FUNCTION:
    {
        type = walk_func(ctx, static_pointer_cast<fun_node_t>(node->def));
        break;

    }
    case TT_OBJECT:
    case TT_UNKNOWN:
    default:
    {
		//TBD: implement user defined types
        throw;
    }
    }

	if (node->ref_count > 0)
    {
		type->irt = PointerType::getUnqual(*ctx->llvm_ctx); // collapse to pointer type
		for (int i = 1; i <= node->ref_count; i++)  
        {
			type->ref_count = i;
            type->dit = get_dit(TYPE_KEY(type), ctx->llvm_di->createPointerType(type->dit, 64)); // must preserve the type for each level of indirection for debug info
        }
	
    }

    return type;

}

type_ptr_t
llvmir_compiler_t::walk_func(compiler_ctx_t* ctx, fun_node_ptr_t fun)
{
    if (type_cache.find(NODE_KEY(fun)) != type_cache.end())
        return type_cache[NODE_KEY(fun)];

    type_ptr_t rett = walk_type(ctx, fun->ret_type);

    SmallVector<Metadata*, 8>   di_sig;
    di_sig.push_back(rett->dit);
    
    std::vector<Type*>  args_irt; 
    for (auto p : fun->var_list->vars_m)
    {
        type_ptr_t argt = walk_type(ctx, p.second->type);
        args_irt.push_back(argt->irt);
        di_sig.push_back(argt->dit);
    };

    auto fun_irt = FunctionType::get(rett->irt, args_irt, false);

    DISubroutineType *dit = (DISubroutineType*) get_dit(
        DI_KEY(fun_irt),  
        ctx->llvm_di->createSubroutineType(ctx->llvm_di->getOrCreateTypeArray(di_sig)));

    Function* fun_ir =
        Function::Create(fun_irt,
            Function::ExternalLinkage, fun->id->name, ctx->llvm_module);

    DISubprogram* sp = ctx->llvm_di->createFunction(
        ctx->llvm_di_file,        // Function scope.  
        fun->id->name,            // Function name.
        fun->id->name,            // Mangled function name.
        ctx->llvm_di_file,        // File where this variable is defined.
        fun->line,                // Line number.
        dit, // fnType
        fun->line,                 // scope line
        DINode::FlagZero,
        DISubprogram::SPFlagDefinition
     );

    fun_ir->setSubprogram(sp);

    type_ptr_t type(new type_t());

    type->def = fun;
    type->irt = fun_irt;
    type->dit = dit;
   
    if (!fun->is_defined)
        return type;
    
    BasicBlock* ir_entry = BasicBlock::Create(*ctx->llvm_ctx, fun->id->name, fun_ir);

    ctx->llvm_ir->SetInsertPoint(ir_entry);

    ctx->llvm_ir->CreateRet(ConstantInt::get(Type::getInt32Ty(*ctx->llvm_ctx), 0));
  
    for (auto& statement : fun->exec_block->exec_statements)
    {
        walk_exec_statement(ctx, statement);
    }

    bool is_broken = llvm::verifyFunction(*fun_ir);

    if (is_broken) {
        throw compiler_exeption_t("%s:%zu:%zu: error: generated IR for function '%s' is broken", 
            ctx->ast->source_id.c_str(), 
            fun->line, 
            fun->pos, 
			fun->id->name.c_str());
    }
   
    return type;
		
}

void 
llvmir_compiler_t::walk_prog(compiler_ctx_t* ctx, prog_node_ptr_t prog)
{
    for (auto& decl : prog->decl_statements)
    {
        switch (decl->type)
        {
        case FUNCTION_NODE:
            walk_func(ctx, static_pointer_cast<fun_node_t>(decl));
			break;
        }
    }

}

value_ptr_t 
llvmir_compiler_t::walk_identifier(compiler_ctx_t* ctx, identifier_expr_node_ptr_t node)
{
    return nullptr;
}

void 
llvmir_compiler_t::walk_exec_statement(compiler_ctx_t* ctx, node_ptr_t node)
{
    switch (node->type)
    {
        case EXPRESSION_NODE:
        {
			walk_expression(ctx, static_pointer_cast<expr_node_t>(node));
            break;
		}
    default:
        break;
    }
}

value_ptr_t
llvmir_compiler_t::walk_expression(compiler_ctx_t* ctx, expr_node_ptr_t node)
{
	value_ptr_t val(new value_t());
    switch (node->op)
    {
    case expr_literal:
    {
        val = walk_expr_literal(ctx, PCAST(literal_expr_node_t,node));
        break;
    }
    case expr_funcall:
    {
        val = walk_fun_call(ctx, PCAST(funcall_expr_node_t,node));
        break;
    }
    case expr_identifier:
    {
		val = walk_identifier(ctx, PCAST(identifier_expr_node_t, node));
        break;
    }
    default:
        break;
    }

    return val;
}



value_ptr_t
llvmir_compiler_t::walk_fun_call(compiler_ctx_t* ctx, funcall_expr_node_ptr_t node)
{

	/*Value* callee = walk_expression(ctx, node->function);

    funcPtr = builder.CreateBitCast(funcPtr, fooPtrType);
	

    ctx->llvm_ir->CreateCall(fooFunc, {
    ConstantInt::get(intTy, 10),
    ConstantInt::get(intTy, 20),
    x
        });;*/

    return nullptr;
}

value_ptr_t
llvmir_compiler_t::walk_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node)
{
	return walk_literal(ctx, node->literal);
}

value_ptr_t
llvmir_compiler_t::walk_literal(compiler_ctx_t* ctx, literal_node_ptr_t node)
{
	value_ptr_t val(new value_t());

    //val->type = ctx->type_cache->get_type(node);

    switch (node->lit_type)
    {
    case LIT_INT:
    {
	    val->ir_value   =  ConstantInt::get(val->type->irt, std::get<int>(node->value));
	    break;
    }
    case LIT_STRING:
    {
        val->ir_value = ctx->llvm_ir->CreateGlobalString(std::get<string>(node->value));
        break;
    }
    case LIT_CHAR:
    {
        val->ir_value = ConstantInt::get(val->type->irt, std::get<char>(node->value));
        break;
    }
    default:
    {
        throw compiler_exeption_t("%s:%zu:%zu: error: unsupported literal type %d", ctx->ast->source_id.c_str(), node->line, node->pos, node->line);
    }
    }

    return val;
        
}