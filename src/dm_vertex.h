
#ifndef __DM_VERTEX_H__
#define __DM_VERTEX_H__

#include "structs.h"

class DoomMap;

struct doomvertex_t
{
	short x;
	short y;
};

class Vertex
{
private:
	DoomMap	*parent;

	int		x, y;
	WORD	ref_count;
	props_t	ex_props;

	int		index;

public:
	Vertex(DoomMap *parent = NULL);
	Vertex(int x, int y, DoomMap *parent);
	Vertex(doomvertex_t v, DoomMap *parent);
	~Vertex();

	int x_pos() { return x; }
	int y_pos() { return y; }
	int refs()	{ return ref_count; }

	int		get_index() { return index; }
	void	set_index(int i) { index = i; }

	void	move(int xd, int yd);
	void	set_pos(int x, int y) { this->x = x; this->y = y; }
	void	set_parent(DoomMap* parent) { this->parent = parent; }

	void add_ref();
	bool rem_ref();
	void set_refs(int n) { ref_count = n; }
	void update_refs();

	point2_t pos();

	string ex_prop(string p);

	void draw(BYTE style = 0);

	doomvertex_t	to_doomformat();
};

#endif
