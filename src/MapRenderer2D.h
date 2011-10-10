
#ifndef __MAP_RENDERER_2D__
#define __MAP_RENDERER_2D__

class SLADEMap;
class ThingType;
class GLTexture;
class MapRenderer2D {
private:
	SLADEMap*	map;
	GLTexture*	tex_last;

	// VBOs etc
	GLuint	vbo_vertices;
	GLuint	vbo_lines;
	GLuint	ibo_selection;

	// Display lists
	GLuint	list_vertices;
	GLuint	list_lines;

	// Visibility
	enum {
		VIS_LEFT	= 1,
		VIS_RIGHT	= 2,
		VIS_ABOVE	= 4,
		VIS_BELOW	= 8,
	};
	vector<uint8_t>	vis_v;
	vector<uint8_t>	vis_l;
	vector<uint8_t>	vis_t;

	// Structs
	struct glvert_t {
		GLfloat x, y;
		GLfloat r, g, b;
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

public:
	MapRenderer2D(SLADEMap* map);
	~MapRenderer2D();

	// Main drawing stuff
	void	renderVertices(float view_scale);
	void	renderVerticesVBO();
	void	renderVerticesImmediate();
	void	renderLines(bool show_direction);
	void	renderLinesVBO(bool show_direction);
	void	renderLinesImmediate(bool show_direction);
	void	renderRoundThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f);
	bool	renderSpriteThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f);
	void	renderSquareThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f);
	void	renderThings(double view_scale = 1, float alpha = 1.0f);
	void	renderThingsImmediate(double view_scale, float alpha);
	void	renderHilight(int item, int type, float fade, float view_scale);
	void	renderSelection(vector<int>& selection, int type, float view_scale);
	void	renderFlats(int type = 0);

	// VBOs
	void	updateVerticesVBO();
	void	updateLinesVBO(bool show_direction);

	void	updateVisibility(fpoint2_t view_tl, fpoint2_t view_br, double view_scale);
	void	forceUpdate(float view_scale);
};

#endif//__MAP_RENDERER_2D__
