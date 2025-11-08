#pragma once
using namespace std;

namespace aloe
{
	enum statement_type_t
	{
		OBJECT_DECLARATION,
		VAR_DECLARATION,
		FUN_DECLARATION
	};

	struct object_declaration_t
	{

	};
	
	enum identifier_type_t
	{
		OBJECT,
		VAR,
		FUNCTION
	};

	struct identifier_t
	{
		identifier_type_t type;
		string fqn;
	};

	struct module_t
	{
		identifier_t id;
	};
	
	struct statement_t
	{
		statement_type_t type;
	};

	typedef std::list<statement_t> statements_list_t;

	struct ast_t
	{
		module_t module;
		statements_list_t statements;
	};
	
}
