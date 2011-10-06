
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

public:
	MapRenderer2D(SLADEMap* map);
	~MapRenderer2D();

	// Main drawing stuff
	void	renderVertices();
	void	renderVerticesVBO();
	void	renderVerticesImmediate();
	void	renderLines(bool show_direction);
	void	renderLinesVBO(bool show_direction);
	void	renderLinesImmediate(bool show_direction);
	void	renderRoundThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f);
	bool	renderSpriteThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f);
	void	renderSquareThing(double x, double y, double angle, ThingType* type, float alpha = 1.0f);
	void	renderThings(int mode, float alpha = 1.0f);
	void	renderThingsImmediate(int mode, float alpha);

	// VBOs
	void	updateVerticesVBO();
	void	updateLinesVBO(bool show_direction);

	void	updateVisibility(fpoint2_t view_tl, fpoint2_t view_br, double view_scale);
};

#endif//__MAP_RENDERER_2D__
