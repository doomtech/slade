
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

// Those are actually fixed_t
struct doom64vertex_t
{
	int32_t x;
	int32_t y;
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
	MapVertex(doom64vertex_t v);
	~MapVertex(){}

	double	xPos() { return x; }
	double	yPos() { return y; }

	bool	parseUDMF(Tokenizer& tz);
};

#endif //__MAPVERTEX_H__
