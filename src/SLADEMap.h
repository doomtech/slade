
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
	PropertyList		udmf_props;

public:
	SLADEMap();
	~SLADEMap();

	bool	readMap(Archive* map_entries, uint8_t format);
	bool	readDoomMap(Archive* map_entries);
	bool	readHexenMap(Archive* map_entries);
	bool	readUDMFMap(ArchiveEntry* map_data);

	void	drawVertices();
	void	drawLines();
};

#endif //__SLADEMAP_H__
