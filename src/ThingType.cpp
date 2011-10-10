
#include "Main.h"
#include "ThingType.h"
#include "Parser.h"

ThingType::ThingType(string name) {
	// Init variables
	this->name = name;
	this->angled = true;
	this->hanging = false;
	this->colour = COL_WHITE;
	this->radius = 20;
	this->height = 16;
	
	// Init args
	args[0].name = "Arg1";
	args[1].name = "Arg2";
	args[2].name = "Arg3";
	args[3].name = "Arg4";
	args[4].name = "Arg5";
}

void ThingType::reset() {
	// Reset variables
	this->name = "Unknown";
	this->group = "";
	this->sprite = "";
	this->angled = true;
	this->hanging = false;
	this->colour = COL_WHITE;
	this->radius = 20;
	this->height = 16;
	
	// Reset args
	for (unsigned a = 0; a < 5; a++) {
		args[a].name = S_FMT("Arg%d", a+1);
		args[a].type = ARGT_NUMBER;
		args[a].custom_flags.clear();
		args[a].custom_values.clear();
	}
}

void ThingType::parse(ParseTreeNode* node) {
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

		// Sprite
		else if (S_CMPNOCASE(name, "sprite"))
			this->sprite = child->getStringValue();

		// Radius
		else if (S_CMPNOCASE(name, "radius"))
			this->radius = child->getIntValue();

		// Height
		else if (S_CMPNOCASE(name, "height"))
			this->height = child->getIntValue();

		// Colour
		else if (S_CMPNOCASE(name, "colour"))
			this->colour.set(child->getIntValue(0), child->getIntValue(1), child->getIntValue(2));

		// Show angle
		else if (S_CMPNOCASE(name, "angle"))
			this->angled = child->getBoolValue();

		// Hanging object
		else if (S_CMPNOCASE(name, "hanging"))
			this->hanging = child->getBoolValue();

		// Translation
		else if (S_CMPNOCASE(name, "translation"))
		{
			this->translation += "\"";
			size_t v = 0;
			do {
				this->translation += child->getStringValue(v++);
			} while ((v < child->nValues()) && ((this->translation += "\", \""), true));
			this->translation += "\"";
		}

		// Palette override
		else if (S_CMPNOCASE(name, "palette"))
			this->palette = child->getStringValue();
			


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
}

string ThingType::stringDesc() {
	// Init return string
	string ret = S_FMT("\"%s\" in group \"%s\", colour %d,%d,%d, radius %d", CHR(name), CHR(group), colour.r, colour.g, colour.b, radius);

	// Add any extra info
	if (!sprite.IsEmpty()) ret += S_FMT(", sprite \"%s\"", CHR(sprite));
	if (!angled) ret += ", angle hidden";
	if (hanging) ret += ", hanging";

	return ret;
}