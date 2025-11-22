#pragma once

#include <iostream>
#include "tinyxml2.h"
#include "aloe/parser.h"


using namespace std;
using namespace tinyxml2;

namespace aloe
{
	class xml_log_t
	{
	public:
		void write_xml(ostream& os, ast_t* ast);
	private:
		void write_node(XMLElement* el, node_t* node);
		void write_object_node(XMLElement* el, object_node_t* node);
		void write_inheritance_chain_node(XMLElement* el, inheritance_chain_node_t* node);
		void write_identifier_node(XMLElement* el, identifier_node_t* node);
	};

}


