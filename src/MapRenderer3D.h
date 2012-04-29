
#ifndef __MAP_RENDERER_3D_H__
#define __MAP_RENDERER_3D_H__

class SLADEMap;
class GLTexture;
class ThingType;
class MapThing;
class MapSector;
class MapRenderer3D {
private:
	SLADEMap*	map;
	bool		udmf_zdoom;
	bool		fullbright;
	bool		fog;

	// Visibility
	vector<double>	dist_sectors;
	vector<double>	dist_lines;

	// Camera
	fpoint3_t	cam_position;
	fpoint2_t	cam_direction;
	double		cam_pitch;
	double		cam_angle;
	fpoint3_t	cam_dir3d;
	fpoint3_t	cam_strafe;
	double		gravity;

	// Structs
	enum {
		// Common flags
		CALCULATED	= 0x01,
		
		// Thing flags
		ICON		= 0x02,

		// Quad flags
		TRANS		= 0x02,
		SKY			= 0x04,
	};
	struct gl_vertex_t {
		float x, y, z;
		float tx, ty;
	};
	struct quad_3d_t {
		gl_vertex_t	points[4];
		rgba_t		colour;
		uint8_t		light;
		GLTexture*	texture;
		uint8_t		flags;

		quad_3d_t() {
			colour.set(255, 255, 255, 255, 0);
			texture = NULL;
			flags = 0;
		}
	};
	struct line_3d_t {
		uint8_t				flags;
		vector<quad_3d_t>	quads;

		line_3d_t() { flags = 0; }
	};
	struct thing_3d_t {
		uint8_t		flags;
		ThingType*	type;
		MapSector*	sector;
		float		z;
		GLTexture*	sprite;

		thing_3d_t() {
			flags = 0;
			type = NULL;
			sector = NULL;
			z = 0;
		}
	};

	// Map Structures
	vector<line_3d_t>	lines;
	vector<thing_3d_t>	things;

	// VBOs
	GLuint	vbo_floors;
	GLuint	vbo_ceilings;
	GLuint	vbo_walls;

	// Sky
	struct gl_vertex_ex_t {
		float x, y, z;
		float tx, ty;
		float alpha;
	};
	string		skytex1;
	string		skytex2;
	rgba_t		skycol_top;
	rgba_t		skycol_bottom;
	fpoint2_t	sky_circle[32];

public:
	MapRenderer3D(SLADEMap* map = NULL);
	~MapRenderer3D();

	bool	fullbrightEnabled() { return fullbright; }
	bool	fogEnabled() { return fog; }
	void	enableFullbright(bool enable = true) { fullbright = enable; }
	void	enableFog(bool enable = true) { fog = enable; }

	bool	init();
	void	refresh();
	void	buildSkyCircle();

	// Camera
	void	cameraMove(double distance);
	void	cameraTurn(double angle);
	void	cameraMoveUp(double distance);
	void	cameraStrafe(double distance);
	void	cameraPitch(double amount);
	void	cameraUpdateVectors();
	void	cameraSet(fpoint3_t position, fpoint2_t direction);
	void	cameraApplyGravity(double mult);

	// -- Rendering --
	void	setupView(int width, int height);
	void	setLight(rgba_t& colour, uint8_t light, float alpha = 1.0f);
	void	renderMap();
	void	renderSkySlice(float top, float bottom, float atop, float abottom, float size);
	void	renderSky();

	// Flats
	void	updateSectorVBO(unsigned index);
	void	renderFlats();
	void	renderFlatsVBO();

	// Walls
	void	setupQuad(quad_3d_t* quad, double x1, double y1, double x2, double y2, double top, double bottom);
	void	setupQuadTexCoords(quad_3d_t* quad, int length, double left, double top, bool pegbottom = false, double sx = 1, double sy = 1);
	void	updateLine(unsigned index);
	void	renderQuad(quad_3d_t* quad, float alpha = 1.0f);
	void	renderWalls();

	// Things
	void	updateThing(unsigned index, MapThing* thing);
	void	renderThings();

	// VBO stuff
	void	updateFlatsVBO();
	void	updateWallsVBO();

	// Visibility checking
	void	quickVisDiscard();
	float	calcDistFade(double distance, double max = -1);
};

#endif//__MAP_RENDERER_3D_H__
