#pragma once

#include <variant>
#include <string>
#include <list>
#include <map>

using namespace std;

namespace aloe
{
	enum entity_ref_type_e
	{
		REF_UNKNOWN,
		REF_BY_ID,
		REF_IN_PLACE
	};

	enum node_type_e
	{
		UNKNOWN_NODE,
		OBJECT_NODE,
		IDENTIFIER_NODE,
		INHERITANCE_CHAIN_NODE
	};

	struct identifier_node_t;
	struct object_node_t;
	struct inheritance_chain_node_t;

	struct node_t
	{
		node_t() { node_type = UNKNOWN_NODE; }

		node_type_e node_type;

		virtual bool add_identifier_node(identifier_node_t* n)               { return false; };
		virtual bool add_object_node(object_node_t* n)                       { return false; };
		virtual bool add_inheritance_chain_node(inheritance_chain_node_t* n) { return false; };
		virtual bool mark_pointer(bool ptr)                                  { return false; };
	};

	typedef std::list<node_t*>  node_list_t;

	struct identifier_node_t : public node_t
	{
		identifier_node_t() { node_type = IDENTIFIER_NODE; }
		string name;
		string fqn;
	};

	struct object_node_t : public node_t
	{
		object_node_t() { node_type = OBJECT_NODE; id = nullptr; chain = nullptr; }
		virtual bool add_identifier_node(identifier_node_t* n) override { id = n;  return true; };
		virtual bool add_inheritance_chain_node(inheritance_chain_node_t* n) override { chain = n;  return true; };
		identifier_node_t* id;
		inheritance_chain_node_t* chain;
	};

	struct object_ref_t
	{
		object_ref_t() { ref_type = REF_UNKNOWN; u = { 0 }; is_virtual = false; }
		entity_ref_type_e ref_type;
		bool is_virtual;
		union entity {
			object_node_t* object;
			identifier_node_t* id;
		} u;
	};

	typedef std::list<object_ref_t>  object_ref_list_t;

	struct inheritance_chain_node_t : public node_t
	{
		inheritance_chain_node_t() { node_type = INHERITANCE_CHAIN_NODE; is_virtual = false; }

		bool is_virtual;

		virtual bool mark_pointer(bool ptr) { is_virtual = ptr; return true; }

		virtual bool add_identifier_node(identifier_node_t* n) override
		{
			object_ref_t e; 
			e.ref_type = REF_BY_ID;
			e.is_virtual = is_virtual;
			e.u.id = n;
			obj_entities.push_back(e);

			return true;
		}
		virtual bool add_object_node(object_node_t* n) override
		{
			object_ref_t e;
			e.ref_type = REF_IN_PLACE;
			e.is_virtual = is_virtual;
			e.u.object = n;
			obj_entities.push_back(e);

			return true;
		}

		object_ref_list_t obj_entities;
	};
	
	struct ast_t : public node_t
	{
		virtual bool add_object_node(object_node_t* n) { nodes.push_back(n); return true; };
		node_list_t nodes;
	};
	
}
