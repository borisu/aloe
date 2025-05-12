#include "pch.h"
#include "parser.h"

using namespace aloe;

TEST(parser_test, zone_test) {

	aloe::parser_t p;

	ASSERT_TRUE(p.parse_from_file("../../aloe_test/sources/module1.aloe", nullptr));
	
}