
#include "Main.h"
#include "WxStuff.h"
#include "UDMFProperty.h"
#include "Parser.h"

UDMFProperty::UDMFProperty() {
	has_default = false;
	flag = false;
	trigger = false;
	type = -1;
}

UDMFProperty::~UDMFProperty() {
}

void UDMFProperty::parse(ParseTreeNode* node, string group) {
	// Set group and property name
	this->group = group;
	this->property = node->getName();

	// Check for basic definition
	if (node->nChildren() == 0) {
		name = node->getStringValue();
		return;
	}

	// Otherwise, read node data
	for (unsigned a = 0; a < node->nChildren(); a++) {
		ParseTreeNode* prop = (ParseTreeNode*)node->getChild(a);

		// Property type
		if (S_CMPNOCASE(prop->getName(), "type")) {
			if (S_CMPNOCASE(prop->getStringValue(), "bool"))
				type = TYPE_BOOL;
			else if (S_CMPNOCASE(prop->getStringValue(), "int"))
				type = TYPE_INT;
			else if (S_CMPNOCASE(prop->getStringValue(), "float"))
				type = TYPE_FLOAT;
			else if (S_CMPNOCASE(prop->getStringValue(), "string"))
				type = TYPE_STRING;
			else if (S_CMPNOCASE(prop->getStringValue(), "colour"))
				type = TYPE_COLOUR;
		}

		// Property name
		else if (S_CMPNOCASE(prop->getName(), "name"))
			name = prop->getStringValue();

		// Default value
		else if (S_CMPNOCASE(prop->getName(), "default")) {
			switch (type) {
			case TYPE_BOOL: 	default_value = prop->getBoolValue(); break;
			case TYPE_INT:		default_value = prop->getIntValue(); break;
			case TYPE_FLOAT:	default_value = prop->getFloatValue(); break;
			case TYPE_STRING:	default_value = prop->getStringValue(); break;
			case TYPE_COLOUR:	default_value = prop->getStringValue(); break;
			default:			default_value = prop->getStringValue(); break;
			}

			has_default = true;
		}

		// Property is a flag
		else if (S_CMPNOCASE(prop->getName(), "flag"))
			flag = true;

		// Property is a SPAC trigger
		else if (S_CMPNOCASE(prop->getName(), "trigger"))
			trigger = true;

		// Possible values
		else if (S_CMPNOCASE(prop->getName(), "values")) {
			if (type == TYPE_BOOL) {
				for (unsigned b = 0; b < prop->nValues(); b++)
					values.push_back(prop->getBoolValue(b));
			}
			else if (type == TYPE_INT) {
				for (unsigned b = 0; b < prop->nValues(); b++)
					values.push_back(prop->getIntValue(b));
			}
			else if (type == TYPE_FLOAT) {
				for (unsigned b = 0; b < prop->nValues(); b++)
					values.push_back(prop->getFloatValue(b));
			}
			else {
				for (unsigned b = 0; b < prop->nValues(); b++)
					values.push_back(prop->getStringValue(b));
			}
		}
	}
}

string UDMFProperty::getStringRep() {
	string ret = S_FMT("Property \"%s\": name = \"%s\", group = \"%s\"", CHR(property), CHR(name), CHR(group));

	switch (type) {
	case TYPE_BOOL: ret += ", type = bool"; break;
	case TYPE_INT: ret += ", type = int"; break;
	case TYPE_FLOAT: ret += ", type = float"; break;
	case TYPE_STRING: ret += ", type = string"; break;
	case TYPE_COLOUR: ret += ", type = colour"; break;
	default: ret += ", ******unknown type********"; break;
	};

	if (has_default) {
		if (type == TYPE_BOOL) {
			if ((bool)default_value)
				ret += ", default = true";
			else
				ret += ", default = false";
		}
		else if (type == TYPE_INT)
			ret += S_FMT(", default = %d", (int)default_value);
		else if (type == TYPE_FLOAT)
			ret += S_FMT(", default = %1.2f", (double)default_value);
		else
			ret += S_FMT(", default = \"%s\"", default_value.getStringValue());
	}
	else
		ret += ", no valid default";

	if (flag)
		ret += ", is flag";
	if (trigger)
		ret += ", is trigger";

	if (values.size() > 0) {
		ret += "\nPossible values: ";
		for (unsigned a = 0; a < values.size(); a++) {
			ret += values[a].getStringValue();
			if (a < values.size() - 1)
				ret += ", ";
		}
	}

	return ret;
}