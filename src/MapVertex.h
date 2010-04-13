
#ifndef __MAPVERTEX_H__
#define __MAPVERTEX_H__

#include "Tokenizer.h"
#include "PropertyList.h"

class MapLine;

struct doomvertex_t
{
	short x;
	short y;
};

class MapVertex {
private:
	double	x;
	double	y;

	vector<MapLine*>	connected_lines;

	PropertyList	udmf_props;

public:
	MapVertex(){}
	MapVertex(doomvertex_t v);
	~MapVertex(){}

	double	xPos() { return x; }
	double	yPos() { return y; }

	bool	parseUDMF(Tokenizer& tz);
};

#endif //__MAPVERTEX_H__
