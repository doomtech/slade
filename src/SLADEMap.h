
#ifndef __SLADEMAP_H__
#define __SLADEMAP_H__

#include "MapLine.h"
#include "MapSide.h"
#include "MapSector.h"
#include "MapVertex.h"
#include "MapThing.h"
#include "Archive.h"
#include "PropertyList.h"


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
	void	drawVertices();
	void	drawLines();

	// Doom format
	bool	readDoomMap(Archive* map_entries);
	bool	readDoomVertexes(ArchiveEntry* entry);
	bool	readDoomSidedefs(ArchiveEntry* entry);
	bool	readDoomLinedefs(ArchiveEntry* entry);
	bool	readDoomSectors(ArchiveEntry* entry);
	bool	readDoomThings(ArchiveEntry* entry);

	// Hexen format
	bool	readHexenMap(Archive* map_entries);
	bool	readHexenLinedefs(ArchiveEntry* entry);
	bool	readHexenThings(ArchiveEntry* entry);

	// Doom 64 format
	bool	readDoom64Map(Archive* map_entries);
	bool	readDoom64Vertexes(ArchiveEntry* entry);
	bool	readDoom64Sidedefs(ArchiveEntry* entry);
	bool	readDoom64Linedefs(ArchiveEntry* entry);
	bool	readDoom64Sectors(ArchiveEntry* entry);
	bool	readDoom64Things(ArchiveEntry* entry);

	// UDMF
	bool	readUDMFMap(ArchiveEntry* map_data);

};

#endif //__SLADEMAP_H__
