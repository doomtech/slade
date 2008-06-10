
#ifndef __SLADEMAP_H__
#define __SLADEMAP_H__

#include "MapLine.h"
#include "MapSide.h"
#include "MapSector.h"
#include "MapVertex.h"
#include "MapThing.h"

class SLADEMap
{
private:
	vector<MapLine*>	lines;
	vector<MapSide*>	sides;
	vector<MapSector*>	sectors;
	vector<MapVertex*>	vertices;
	vector<MapThing*>	things;

public:
	SLADEMap();
	~SLADEMap();
};

#endif //__SLADEMAP_H__