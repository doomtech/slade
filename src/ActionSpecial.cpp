
/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "ActionSpecial.h"


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
