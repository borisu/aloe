#include "pch.h"
#include "aloe/utils.h"
#include "xml_log.h"

using namespace std;
using namespace aloe;


void 
aloe::print_ast(ast_t* tree)
{
	xml_log_t xml;
	xml.write_xml(std::cerr,tree);
}