#include "pch.h"
#include "aloe/parser.h"

using namespace aloe;

TEST(parser_test, object_test) {

	aloe::parser_t* p = create_parser();

	ast_t* ast = nullptr;
	
	ASSERT_TRUE(p->parse_from_string(
R"(

object A {}
object B {}
object C > B > A {}


)", &ast));

	release_parser(p);
	
}