
#ifndef __SLADEMAP_H__
#define __SLADEMAP_H__

#include "MapLine.h"
#include "MapSide.h"
#include "MapSector.h"
#include "MapVertex.h"
#include "MapThing.h"
#include "Archive.h"
#include "PropertyList.h"

class ParseTreeNode;
class SLADEMap {
friend class MapEditor;
private:
	vector<MapLine*>	lines;
	vector<MapSide*>	sides;
	vector<MapSector*>	sectors;
	vector<MapVertex*>	vertices;
	vector<MapThing*>	things;
	string				udmf_namespace;
	PropertyList		udmf_props;

	// Map structure index status
	// These are true if the per-item index of that type is up-to-date
	// (used for faster item index lookup in certain cases)
	bool	i_lines;
	bool	i_sides;
	bool	i_sectors;
	bool	i_vertices;
	bool	i_things;

	// Doom format
	bool	addVertex(doomvertex_t& v);
	bool	addSide(doomside_t& s);
	bool	addLine(doomline_t& l);
	bool	addSector(doomsector_t& s);
	bool	addThing(doomthing_t& t);
	bool	readDoomVertexes(ArchiveEntry* entry);
	bool	readDoomSidedefs(ArchiveEntry* entry);
	bool	readDoomLinedefs(ArchiveEntry* entry);
	bool	readDoomSectors(ArchiveEntry* entry);
	bool	readDoomThings(ArchiveEntry* entry);

	// Hexen format
	bool	addLine(hexenline_t& l);
	bool	addThing(hexenthing_t& t);
	bool	readHexenLinedefs(ArchiveEntry* entry);
	bool	readHexenThings(ArchiveEntry* entry);

	// Doom 64 format
	bool	addVertex(doom64vertex_t& v){return true;}
	bool	addSide(doom64side_t& s){return true;}
	bool	addLine(doom64line_t& l){return true;}
	bool	addSector(doom64sector_t& s){return true;}
	bool	addThing(doom64thing_t& t){return true;}
	bool	readDoom64Vertexes(ArchiveEntry* entry);
	bool	readDoom64Sidedefs(ArchiveEntry* entry);
	bool	readDoom64Linedefs(ArchiveEntry* entry);
	bool	readDoom64Sectors(ArchiveEntry* entry);
	bool	readDoom64Things(ArchiveEntry* entry);

	// UDMF
	bool	addVertex(ParseTreeNode* def);
	bool	addSide(ParseTreeNode* def);
	bool	addLine(ParseTreeNode* def);
	bool	addSector(ParseTreeNode* def);
	bool	addThing(ParseTreeNode* def);

public:
	SLADEMap();
	~SLADEMap();

	MapVertex*	getVertex(unsigned index);
	MapSide*	getSide(unsigned index);
	MapLine*	getLine(unsigned index);
	MapSector*	getSector(unsigned index);
	MapThing*	getThing(unsigned index);
	size_t		nVertices() { return vertices.size(); }
	size_t		nLines() { return lines.size(); }
	size_t		nSides() { return sides.size(); }
	size_t		nSectors() { return sectors.size(); }
	size_t		nThings() { return things.size(); }

	// Map structure indices
	int		vertexIndex(MapVertex* v);
	int		sideIndex(MapSide* s);
	int		lineIndex(MapLine* l);
	int		sectorIndex(MapSector* s);
	int		thingIndex(MapThing* t);
	void	refreshIndices();

	bool	readMap(Archive::mapdesc_t map);

	void	clearMap();

	// Map loading
	bool	readDoomMap(Archive::mapdesc_t map);
	bool	readHexenMap(Archive::mapdesc_t map);
	bool	readDoom64Map(Archive::mapdesc_t map);
	bool	readUDMFMap(Archive::mapdesc_t map);

	// Geometry
	int			nearestVertex(double x, double y, double min = 64);
	int			nearestLine(double x, double y, double min = 64);
	int			nearestThing(double x, double y, double min = 64);
	vector<int>	nearestThingMulti(double x, double y);
	int			inSector(double x, double y);

	bool	lineInSector(MapLine* line, MapSector* sector);
	bool	getLinesOfSector(unsigned index, vector<MapLine*>& list);

	// Checks
	int		removeDetachedVertices();
};

#endif //__SLADEMAP_H__
