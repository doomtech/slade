
#ifndef __SECTOR_BUILDER_H__
#define __SECTOR_BUILDER_H__

// Forward declarations
class MapLine;
class MapVertex;
class MapSector;
class SLADEMap;

class SectorBuilder {
private:
	struct edge_t {
		MapLine*	line;
		bool		front;

		edge_t(MapLine* line = NULL, bool front = true) {
			this->line = line;
			this->front = front;
		}
	};

	vector<bool>	vertex_valid;
	SLADEMap*		map;
	vector<edge_t>	sector_edges;
	string			error;

	// Current outline
	vector<edge_t>	o_edges;
	bool			o_clockwise;
	bbox_t			o_bbox;
	MapVertex*		vertex_right;

public:
	SectorBuilder();
	~SectorBuilder();

	string		getError() { return error; }
	unsigned	nEdges() { return sector_edges.size(); }
	MapLine*	getEdgeLine(unsigned index);
	bool		edgeIsFront(unsigned index);

	edge_t		nextEdge(edge_t edge);
	bool		traceOutline(MapLine* line, bool front = true);
	int			nearestEdge(double x, double y);
	bool		pointWithinOutline(double x, double y);
	void		discardOutsideVertices();
	edge_t		findOuterEdge();
	edge_t		findInnerEdge();
	MapSector*	findCopySector();
	MapSector*	findExistingSector();

	bool	traceSector(SLADEMap* map, MapLine* line, bool front = true);
	void	createSector(MapSector* sector_copy = NULL);

	// Testing
	void	drawResult();
};

#endif//__SECTOR_BUILDER_H__
