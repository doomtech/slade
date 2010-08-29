
#ifndef __MAP_ENTRY_PANEL_H__
#define __MAP_ENTRY_PANEL_H__

#include "EntryPanel.h"
#include "OGLCanvas.h"

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
	mep_line_t(unsigned v1, unsigned v2) { this->v1 = v1; this->v2 = v2; }
};

class MEPCanvas : public OGLCanvas {
private:
	vector<mep_vertex_t>	verts;
	vector<mep_line_t>		lines;
	double					zoom;
	double					offset_x;
	double					offset_y;

public:
	MEPCanvas(wxWindow* parent);
	~MEPCanvas();

	void addVertex(double x, double y);
	void addLine(unsigned v1, unsigned v2, bool twosided, bool special);
	void clearMap();
	void showMap();
	void draw();
};

class MapEntryPanel : public EntryPanel {
private:
	MEPCanvas*	map_canvas;

public:
	MapEntryPanel(wxWindow* parent);
	~MapEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
};

#endif//__MAP_ENTRY_PANEL_H__
