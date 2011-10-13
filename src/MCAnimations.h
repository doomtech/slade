
#ifndef __MC_ANIMATIONS_H__
#define __MC_ANIMATIONS_H__

class MCAnimation {
protected:
	long	starttime;

public:
	MCAnimation(long start) { starttime = start; }
	~MCAnimation() {}

	virtual bool update(long time) { return false; }
	virtual void draw() {}
};

class MCASelboxFader : public MCAnimation {
private:
	fpoint2_t	tl;
	fpoint2_t	br;
	float		fade;

public:
	MCASelboxFader(long start, fpoint2_t tl, fpoint2_t br);
	~MCASelboxFader();

	bool update(long time);
	void draw();
};

class MCAThingSelection : public MCAnimation {
private:
	double	x;
	double	y;
	double	radius;
	bool	select;
	float	fade;

public:
	MCAThingSelection(long start, double x, double y, double radius, bool select = true);
	~MCAThingSelection();

	bool update(long time);
	void draw();
};

class MapLine;
class MCALineSelection : public MCAnimation {
private:
	vector<frect_t>	lines;
	vector<frect_t>	tabs;
	bool	select;
	float	fade;

public:
	MCALineSelection(long start, vector<MapLine*>& lines, bool select = true);
	~MCALineSelection();

	bool update(long time);
	void draw();
};

class MapVertex;
class MCAVertexSelection : public MCAnimation {
private:
	vector<fpoint2_t>	vertices;
	double				size;
	bool				select;
	float				fade;

public:
	MCAVertexSelection(long start, vector<MapVertex*>& verts, double size, bool select = true);
	~MCAVertexSelection();

	bool update(long time);
	void draw();
};

class Polygon2D;
class MCASectorSelection : public MCAnimation {
private:
	vector<Polygon2D*>	polygons;
	bool				select;
	float				fade;

public:
	MCASectorSelection(long start, vector<Polygon2D*>& polys, bool select = true);
	~MCASectorSelection();

	bool update(long time);
	void draw();
};

#endif//__MC_ANIMATIONS_H__
