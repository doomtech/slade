
#ifndef __MAP_RENDERER_3D_H__
#define __MAP_RENDERER_3D_H__

class SLADEMap;
class GLTexture;
class ThingType;
class MapThing;
class MapSector;
class Polygon2D;
class MapRenderer3D {
private:
	SLADEMap*	map;
	bool		udmf_zdoom;
	bool		fullbright;
	bool		fog;
	int			last_light;
	GLTexture*	tex_last;
	unsigned	n_quads;
	unsigned	n_flats;
	int			flat_last;

	// Visibility
	vector<float>	dist_sectors;

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
		TRANS	= 0x01,

		// Quad/flat flags
		SKY		= 0x02,

		// Quad flags
		MIDTEX	= 0x04,

		// Flat flags
		CEIL	= 0x04,

		// Thing flags
		ICON	= 0x02,
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
		float		alpha;

		quad_3d_t() {
			colour.set(255, 255, 255, 255, 0);
			texture = NULL;
			flags = 0;
		}
	};
	struct line_3d_t {
		vector<quad_3d_t>	quads;
		long				updated_time;
		bool				visible;

		line_3d_t() { updated_time = 0; visible = true; }
	};
	struct thing_3d_t {
		uint8_t		flags;
		ThingType*	type;
		MapSector*	sector;
		float		z;
		GLTexture*	sprite;
		long		updated_time;

		thing_3d_t() {
			flags = 0;
			type = NULL;
			sector = NULL;
			z = 0;
			updated_time = 0;
		}
	};
	struct flat_3d_t {
		uint8_t		flags;
		uint8_t		light;
		rgba_t		colour;
		GLTexture*	texture;
		plane_t		plane;
		float		alpha;
		MapSector*	sector;
		long		updated_time;

		flat_3d_t() {
			light = 255;
			texture = NULL;
			updated_time = 0;
			flags = 0;
			alpha = 1.0f;
			sector = NULL;
		}
	};

	// Map Structures
	vector<line_3d_t>	lines;
	quad_3d_t**			quads;
	vector<thing_3d_t>	things;
	vector<flat_3d_t>	floors;
	vector<flat_3d_t>	ceilings;
	flat_3d_t**			flats;

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
	void	clearData();
	void	buildSkyCircle();

	// Camera
	void	cameraMove(double distance, bool z = true);
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
	void	renderSkySlice(float top, float bottom, float atop, float abottom, float size, float tx = 0.125f, float ty = 2.0f);
	void	renderSky();

	// Flats
	void	updateFlatTexCoords(unsigned index, bool floor);
	void	updateSector(unsigned index);
	void	renderFlat(flat_3d_t* flat);
	void	renderFlats();
	//void	renderFlatsVBO();

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
	void	checkVisibleQuads();
	void	checkVisibleFlats();
};

#endif//__MAP_RENDERER_3D_H__
