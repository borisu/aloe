#include "pch.h"
#include "parser.h"

using namespace aloe;

TEST(parser_test, object_test) {

	aloe::parser_t p;

	ASSERT_TRUE(p.parse_from_file("../../aloe_test/sources/objects.aloe", nullptr));
	
}