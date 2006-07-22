
#ifndef __COLOURS_H__
#define __COLOURS_H__

#include "structs.h"

struct coldef_t
{
	string	name;
	string	desc;
	rgba_t	&col;
};

void parse_rgba(Tokenizer *tz, rgba_t* col);
void load_colour_config(string filename);
void save_colour_config(string filename);
void load_colours(Tokenizer *tz);
void save_colours(FILE* fp);

#endif //__COLOURS_H__
