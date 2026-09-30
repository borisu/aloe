#pragma once
#include <string>

using namespace std;

namespace aloe
{
	struct context_t
	{
		context_t() :line(-1), pos(-1) {}
		size_t line;
		size_t pos;
	};
}
