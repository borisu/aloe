#include "pch.h"
#include "xml_log.h"
#include "tinyxml2.h"

using namespace std;
using namespace aloe;
using namespace tinyxml2;

void 
xml_log_t::write_xml(ostream& os, ast_t* ast)
{
	// Create a new XML document
	XMLDocument doc;

	// Add XML declaration
	XMLDeclaration* decl = doc.NewDeclaration();
	doc.InsertFirstChild(decl);

	// 3. Add a root element
	XMLElement* root = doc.NewElement("ast");

	write_node(root, ast->nodes.front());

	doc.InsertEndChild(root);

	XMLPrinter p;

	doc.Print(&p);

	os << p.CStr();

}

void 
xml_log_t::write_object_node(XMLElement* el, object_node_t* node)
{
	auto key = el->InsertNewChildElement("object");
	write_identifier_node(key, node->id);
	write_inheritance_chain_node(key, node->chain);
}

void
xml_log_t::write_inheritance_chain_node(XMLElement* el, inheritance_chain_node_t* node)
{
	auto key = el->InsertNewChildElement("inheritance_chain");
}

void
xml_log_t::write_identifier_node(XMLElement* el, identifier_node_t* node)
{
	auto key = el->InsertNewChildElement("identifier");
	key->SetAttribute("name", node->name.c_str());
}


void 
xml_log_t::write_node(XMLElement* el, node_t* node)
{
	switch (node->node_type)
	{
	case OBJECT_NODE:
	{
		write_object_node(el, (object_node_t *)node);
		break;
	}
	case INHERITANCE_CHAIN_NODE:
	{
		write_inheritance_chain_node(el, (inheritance_chain_node_t*)node);
		break;
	}
	case IDENTIFIER_NODE:
	{
		write_identifier_node(el, (identifier_node_t*)node);
		break;
	}
	default :
	{

	}

	}
}
