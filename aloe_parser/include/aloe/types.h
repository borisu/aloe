#pragma once
#include <string>
#include <memory>

using namespace std;

namespace aloe
{
	enum type_e
	{
		UNKNOWN,
		INT,
		DOUBLE,
		CHAR,
		OBJECT,
		FUNCTION
	};

	struct type_t
	{
		type_t(type_e type_type, string name):
			ref_count(0),
			type_type(type_type),
			name(name)
		{

		};

		size_t     ref_count;
		type_e	   type_type;
		string	   name;
	};

	typedef shared_ptr<type_t> type_ptr_t;

}
