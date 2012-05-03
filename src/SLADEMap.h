
#ifndef __SLADEMAP_H__
#define __SLADEMAP_H__

#include "MapLine.h"
#include "MapSide.h"
#include "MapSector.h"
#include "MapVertex.h"
#include "MapThing.h"
#include "Archive.h"
#include "PropertyList.h"

// Line texture ids
enum {
	TEX_FRONT_MIDDLE	= 0x01,
	TEX_FRONT_UPPER		= 0x02,
	TEX_FRONT_LOWER		= 0x04,
	TEX_BACK_MIDDLE		= 0x08,
	TEX_BACK_UPPER		= 0x10,
	TEX_BACK_LOWER		= 0x20,
};

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
	bool				position_frac;
	string				name;

	// The last time the map geometry was updated
	long	geometry_updated;

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

	bool	writeDoomVertexes(ArchiveEntry* entry);
	bool	writeDoomSidedefs(ArchiveEntry* entry);
	bool	writeDoomLinedefs(ArchiveEntry* entry);
	bool	writeDoomSectors(ArchiveEntry* entry);
	bool	writeDoomThings(ArchiveEntry* entry);

	// Hexen format
	bool	addLine(hexenline_t& l);
	bool	addThing(hexenthing_t& t);

	bool	readHexenLinedefs(ArchiveEntry* entry);
	bool	readHexenThings(ArchiveEntry* entry);

	bool	writeHexenLinedefs(ArchiveEntry* entry);
	bool	writeHexenThings(ArchiveEntry* entry);

	// Doom 64 format
	bool	addVertex(doom64vertex_t& v);
	bool	addSide(doom64side_t& s);
	bool	addLine(doom64line_t& l);
	bool	addSector(doom64sector_t& s);
	bool	addThing(doom64thing_t& t);

	bool	readDoom64Vertexes(ArchiveEntry* entry);
	bool	readDoom64Sidedefs(ArchiveEntry* entry);
	bool	readDoom64Linedefs(ArchiveEntry* entry);
	bool	readDoom64Sectors(ArchiveEntry* entry);
	bool	readDoom64Things(ArchiveEntry* entry);

	bool	writeDoom64Vertexes(ArchiveEntry* entry);
	bool	writeDoom64Sidedefs(ArchiveEntry* entry);
	bool	writeDoom64Linedefs(ArchiveEntry* entry);
	bool	writeDoom64Sectors(ArchiveEntry* entry);
	bool	writeDoom64Things(ArchiveEntry* entry);

	// UDMF
	bool	addVertex(ParseTreeNode* def);
	bool	addSide(ParseTreeNode* def);
	bool	addLine(ParseTreeNode* def);
	bool	addSector(ParseTreeNode* def);
	bool	addThing(ParseTreeNode* def);

public:
	SLADEMap();
	~SLADEMap();

	// Map entry ordering
	enum {
		THINGS = 0,
		LINEDEFS,
		SIDEDEFS,
		VERTEXES,
		SECTORS
	};

	string		mapName() { return name; }
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
	long		geometryUpdated() { return geometry_updated; }

	// Map structure indices
	int		vertexIndex(MapVertex* v);
	int		sideIndex(MapSide* s);
	int		lineIndex(MapLine* l);
	int		sectorIndex(MapSector* s);
	int		thingIndex(MapThing* t);
	int		objectIndex(MapObject* o);
	void	refreshIndices();

	bool	readMap(Archive::mapdesc_t map);

	void	clearMap();

	// Map loading
	bool	readDoomMap(Archive::mapdesc_t map);
	bool	readHexenMap(Archive::mapdesc_t map);
	bool	readDoom64Map(Archive::mapdesc_t map);
	bool	readUDMFMap(Archive::mapdesc_t map);

	// Map saving
	bool	writeDoomMap(vector<ArchiveEntry*>& map_entries);
	bool	writeHexenMap(vector<ArchiveEntry*>& map_entries);
	bool	writeDoom64Map(vector<ArchiveEntry*>& map_entries);
	bool	writeUDMFMap(ArchiveEntry* textmap);

	// Item removal
	bool	removeVertex(MapVertex* vertex);
	bool	removeVertex(unsigned index);
	bool	removeLine(MapLine* line);
	bool	removeLine(unsigned index);
	bool	removeSide(MapSide* side);
	bool	removeSide(unsigned index);
	bool	removeSector(MapSector* sector);
	bool	removeSector(unsigned index);
	bool	removeThing(MapThing* thing);
	bool	removeThing(unsigned index);

	// Geometry
	double				fastDistanceToLine(double x, double y, unsigned line, double mindist = 64);
	int					nearestVertex(double x, double y, double min = 64);
	int					nearestLine(double x, double y, double min = 64);
	int					nearestThing(double x, double y, double min = 64);
	vector<int>			nearestThingMulti(double x, double y);
	int					inSector(double x, double y);
	bbox_t				getMapBBox();
	MapVertex*			vertexAt(double x, double y);
	vector<fpoint2_t>	cutLines(double x1, double y1, double x2, double y2);
	MapVertex*			lineCrossVertex(double x1, double y1, double x2, double y2);
	double				distanceToSector(double x, double y, unsigned index, double maxdist = -1);

	bool	lineInSector(MapLine* line, MapSector* sector);
	bool	getLinesOfSector(unsigned index, vector<MapLine*>& list);
	bool	getLinesOfSector(MapSector* sector, vector<MapLine*>& list);
	bool	getVerticesOfSector(unsigned index, vector<MapVertex*>& list);
	int		lineNeedsTexture(unsigned index);

	// Tags/Ids
	void	getSectorsByTag(int tag, vector<MapSector*>& list);
	void	getThingsById(int id, vector<MapThing*>& list);
	void	getLinesById(int id, vector<MapLine*>& list);
	void	getThingsByIdInSectorTag(int id, int tag, vector<MapThing*>& list);

	// Info
	rgba_t	getSectorColour(MapSector* sector, int where = 0, bool fullbright = false);
	uint8_t	getSectorLight(MapSector* sector, int where = 0);
	string	getAdjacentLineTexture(MapVertex* vertex, int tex_part = 255);

	// Creation
	MapVertex*	createVertex(double x, double y, double split_dist = -1);
	MapLine*	createLine(double x1, double y1, double x2, double y2, double split_dist = -1);
	MapLine*	createLine(MapVertex* vertex1, MapVertex* vertex2);
	MapThing*	createThing(double x, double y);
	MapSector*	createSector();
	MapSide*	createSide(MapSector* sector);

	// Editing
	void		moveVertex(unsigned vertex, double nx, double ny);
	void		mergeVertices(unsigned vertex1, unsigned vertex2);
	MapVertex*	mergeVerticesPoint(double x, double y);
	void		splitLine(unsigned line, unsigned vertex);
	void		moveThing(unsigned thing, double nx, double ny);
	void		thingSetAnglePoint(unsigned thing, fpoint2_t point);
	void		splitLinesAt(MapVertex* vertex, double split_dist = 0);
	bool		setLineSector(unsigned line, unsigned sector, bool front = true);
	void		clearLineUnneededTextures(unsigned index);

	// Checks
	int		removeDetachedVertices();
	int		removeZeroLengthLines();
};

#endif //__SLADEMAP_H__
