
#ifndef __MAP_PREVIEW_CANVAS_H__
#define __MAP_PREVIEW_CANVAS_H__

#include "OGLCanvas.h"
#include "Archive.h"

// Structs for basic map features
struct mep_vertex_t {
	double x;
	double y;
	mep_vertex_t(double x, double y) { this->x = x; this->y = y; }
};

struct mep_line_t {
	unsigned	v1;
	unsigned	v2;
	bool		twosided;
	bool		special;
	bool		macro;
	bool		segment;
	mep_line_t(unsigned v1, unsigned v2) { this->v1 = v1; this->v2 = v2; }
};

class MapPreviewCanvas : public OGLCanvas {
private:
	vector<mep_vertex_t>	verts;
	vector<mep_line_t>		lines;
	double					zoom;
	double					offset_x;
	double					offset_y;
	Archive*				temp_archive;

public:
	MapPreviewCanvas(wxWindow* parent);
	~MapPreviewCanvas();

	void addVertex(double x, double y);
	void addLine(unsigned v1, unsigned v2, bool twosided, bool special, bool macro = false);
	bool openMap(Archive::mapdesc_t map);
	void clearMap();
	void showMap();
	void draw();
	void createImage(ArchiveEntry& ae, int width, int height);
};

#endif//__MAP_PREVIEW_CANVAS_H__
