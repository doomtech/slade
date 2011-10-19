
#ifndef __MAP_RENDERER_2D__
#define __MAP_RENDERER_2D__

class SLADEMap;
class ThingType;
class GLTexture;
class MapSector;
class MapLine;
class MapThing;
class MapRenderer2D {
private:
	SLADEMap*	map;
	GLTexture*	tex_last;

	// VBOs etc
	GLuint	vbo_vertices;
	GLuint	vbo_lines;
	GLuint	vbo_flats;

	// Display lists
	GLuint	list_vertices;
	GLuint	list_lines;

	// Visibility
	enum {
		VIS_LEFT	= 1,
		VIS_RIGHT	= 2,
		VIS_ABOVE	= 4,
		VIS_BELOW	= 8,
		VIS_SMALL	= 16,
	};
	vector<uint8_t>	vis_v;
	vector<uint8_t>	vis_l;
	vector<uint8_t>	vis_t;
	vector<uint8_t>	vis_s;

	// Structs
	struct glvert_t {
		GLfloat x, y;
		GLfloat r, g, b, a;
	};
	struct glline_t {
		glvert_t v1, v2;	// The line itself
		glvert_t dv1, dv2;	// Direction tab
	};

	// Other
	bool	lines_dirs;
	int		n_vertices;
	int		n_lines;
	int		n_things;
	double	view_scale;
	double	view_scale_inv;

public:
	MapRenderer2D(SLADEMap* map);
	~MapRenderer2D();

	// Main drawing stuff

	// Vertices
	void	renderVertices(float alpha = 1.0f);
	void	renderVerticesVBO();
	void	renderVerticesImmediate();
	void	renderVertexHilight(int index, float fade);
	void	renderVertexSelection(vector<int>& selection);

	// Lines
	void	renderLines(bool show_direction);
	void	renderLinesVBO(bool show_direction);
	void	renderLinesImmediate(bool show_direction);
	void	renderLineHilight(int index, float fade);
	void	renderLineSelection(vector<int>& selection);
	void	renderTaggedLines(vector<MapLine*>& lines, float fade);

	// Things
	void	renderRoundThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f);
	bool	renderSpriteThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f, bool fitradius = false);
	void	renderSquareThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f, bool showang = true);
	void	renderThings(float alpha = 1.0f);
	void	renderThingsImmediate(float alpha);
	void	renderThingHilight(int index, float fade);
	void	renderThingSelection(vector<int>& selection);
	void	renderTaggedThings(vector<MapThing*>& things, float fade);

	// Flats (sectors)
	void	renderFlats(int type = 0);
	void	renderFlatsImmediate(int type);
	void	renderFlatsVBO(int type);
	void	renderFlatHilight(int index, float fade);
	void	renderFlatSelection(vector<int>& selection);
	void	renderTaggedFlats(vector<MapSector*>& sectors, float fade);


	// VBOs
	void	updateVerticesVBO();
	void	updateLinesVBO(bool show_direction);
	void	updateFlatsVBO();

	// Misc
	void	setScale(double scale) { view_scale = scale; view_scale_inv = 1.0 / scale; }
	void	updateVisibility(fpoint2_t view_tl, fpoint2_t view_br, double view_scale);
	void	forceUpdate(float view_scale);
	double	scaledRadius(int radius);
};

#endif//__MAP_RENDERER_2D__
