
#ifndef __MAPVERTEX_H__
#define __MAPVERTEX_H__

#include "Tokenizer.h"

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

public:
	MapVertex(){}
	MapVertex(doomvertex_t v);
	~MapVertex(){}

	bool	parseUDMF(Tokenizer& tz);
};

#endif //__MAPVERTEX_H__
