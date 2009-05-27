
#ifndef __SLADEMAP_H__
#define __SLADEMAP_H__

#include "MapLine.h"
#include "MapSide.h"
#include "MapSector.h"
#include "MapVertex.h"
#include "MapThing.h"
#include "Archive.h"


class SLADEMap {
private:
	vector<MapLine*>	lines;
	vector<MapSide*>	sides;
	vector<MapSector*>	sectors;
	vector<MapVertex*>	vertices;
	vector<MapThing*>	things;

public:
	SLADEMap();
	~SLADEMap();

	bool	readMap(Archive map_entries);
};

#endif //__SLADEMAP_H__
