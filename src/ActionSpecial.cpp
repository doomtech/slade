
/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "ActionSpecial.h"
#include "Parser.h"


/*******************************************************************
 * ACTIONSPECIAL CLASS FUNCTIONS
 *******************************************************************/

ActionSpecial::ActionSpecial(string name, string group) {
	// Init variables
	this->name = name;
	this->group = group;

	// Init args
	args[0].name = "Arg1";
	args[1].name = "Arg2";
	args[2].name = "Arg3";
	args[3].name = "Arg4";
	args[4].name = "Arg5";
}

void ActionSpecial::reset() {
	// Reset variables
	name = "Unknown";
	group = "";

	// Reset args
	for (unsigned a = 0; a < 5; a++) {
		args[a].name = S_FMT("Arg%d", a+1);
		args[a].type = ARGT_NUMBER;
		args[a].custom_flags.clear();
		args[a].custom_values.clear();
	}
}

void ActionSpecial::parse(ParseTreeNode* node) {
	// Go through all child nodes/values
	ParseTreeNode* child = NULL;
	for (unsigned a = 0; a < node->nChildren(); a++) {
		child = (ParseTreeNode*)node->getChild(a);
		string name = child->getName();
		int arg = -1;

		// Name
		if (S_CMPNOCASE(name, "name"))
			this->name = child->getStringValue();

		// Args
		else if (S_CMPNOCASE(name, "arg1"))
			arg = 0;
		else if (S_CMPNOCASE(name, "arg2"))
			arg = 1;
		else if (S_CMPNOCASE(name, "arg3"))
			arg = 2;
		else if (S_CMPNOCASE(name, "arg4"))
			arg = 3;
		else if (S_CMPNOCASE(name, "arg5"))
			arg = 4;

		// Tagged (group default override)
		else if (S_CMPNOCASE(name, "tagged"))
			this->tagged = child->getBoolValue();



		// Parse arg definition if it was one
		if (arg >= 0) {
			// Check for simple definition
			if (child->isLeaf())
				args[arg].name = child->getStringValue();
			else {
				// Extended arg definition

				// Name
				ParseTreeNode* val = (ParseTreeNode*)child->getChild("name");
				if (val) args[arg].name = val->getStringValue();

				// Type
				val = (ParseTreeNode*)child->getChild("type");
				string atype;
				if (val) atype = val->getStringValue();
				if (S_CMPNOCASE(atype, "yesno"))
					args[arg].type = ARGT_YESNO;
				else if (S_CMPNOCASE(atype, "noyes"))
					args[arg].type = ARGT_NOYES;
				else if (S_CMPNOCASE(atype, "angle"))
					args[arg].type = ARGT_ANGLE;
				else
					args[arg].type = ARGT_NUMBER;
			}
		}
	}

	/*
	// Name
	ParseTreeNode* val_name = (ParseTreeNode*)node->getChild("name");
	if (val_name)
		name = val_name->getStringValue();

	// Args
	for (unsigned arg = 0; arg < 5; arg++) {
		// Get arg value if it exists
		ParseTreeNode* val_arg = (ParseTreeNode*)node->getChild(S_FMT("arg%d", arg+1));
		if (val_arg) {
			// Check for simple definition
			if (val_arg->isLeaf())
				args[arg].name = val_arg->getStringValue();
			else {
				// Extended arg definition

				// Name
				ParseTreeNode* val = (ParseTreeNode*)val_arg->getChild("name");
				if (val) args[arg].name = val->getStringValue();

				// Type
				val = (ParseTreeNode*)val_arg->getChild("type");
				string atype;
				if (val) atype = val->getStringValue();
				if (S_CMPNOCASE(atype, "yesno"))
					args[arg].type = ARGT_YESNO;
				else if (S_CMPNOCASE(atype, "noyes"))
					args[arg].type = ARGT_NOYES;
				else if (S_CMPNOCASE(atype, "angle"))
					args[arg].type = ARGT_ANGLE;
				else
					args[arg].type = ARGT_NUMBER;
			}
		}
	}
	*/
}

string ActionSpecial::stringDesc() {
	// Init string
	string ret = S_FMT("\"%s\" in group \"%s\"", CHR(name), CHR(group));

	// Add tagged info
	if (tagged)
		ret += " (tagged)";
	else
		ret += " (not tagged)";

	// Add args
	ret += "\nArgs: ";
	for (unsigned a = 0; a < 5; a++) {
		ret += args[a].name + ": ";

		if (args[a].type == ARGT_NUMBER)
			ret += "Number";
		else if (args[a].type == ARGT_YESNO)
			ret += "Yes/No";
		else if (args[a].type == ARGT_NOYES)
			ret += "No/Yes";
		else if (args[a].type == ARGT_ANGLE)
			ret += "Angle";
		else
			ret += "Unknown Type";

		ret += ", ";
	}

	return ret;
}
