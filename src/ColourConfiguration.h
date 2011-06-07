
#ifndef __COLOUR_CONFIGURATION_H__
#define __COLOUR_CONFIGURATION_H__

struct cc_col_t {
	bool	exists;
	bool	custom;
	string	name;
	string	group;
	rgba_t	colour;
	cc_col_t() { exists = false; custom = false; }
};

namespace ColourConfiguration {
	rgba_t	getColour(string name);

	bool	readConfiguration(MemChunk& mc);
	bool	writeConfigration(MemChunk& mc);
	bool	init();
}

#endif//__COLOUR_CONFIGURATION_H__
