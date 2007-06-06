
#ifndef __COPYPASTE_H__
#define __COPYPASTE_H__

#define	COPY_THINGS			1
#define COPY_ARCHITECTURE	2

#include "doom_map.h"

class Clipboard
{
private:
	BYTE				copy_type;
	vector<Thing*>		things;
	vector<Side*>		sides;
	vector<Line*>		lines;
	vector<Sector*>		sectors;
	vector<Vertex*>		verts;

public:
	Clipboard();
	~Clipboard();

	void Clear();
	void Copy();
	void Paste();
	void DrawPaste();
};

#endif
