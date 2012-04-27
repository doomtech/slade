
#ifndef __MAP_RENDERER_3D_H__
#define __MAP_RENDERER_3D_H__

class SLADEMap;
class GLTexture;
class MapRenderer3D {
private:
	SLADEMap*	map;
	bool		udmf_zdoom;
	bool		fullbright;
	bool		fog;

	// Visibility
	vector<double>	dist_sectors;
	vector<double>	dist_lines;
	vector<int>		thing_sectors;

	// Camera
	fpoint3_t	cam_position;
	fpoint2_t	cam_direction;
	double		cam_pitch;
	double		cam_angle;
	fpoint3_t	cam_dir3d;
	fpoint3_t	cam_strafe;

	// Structs
	struct gl_vertex_t {
		float x, y, z;
		float tx, ty;
	};
	struct quad_3d_t {
		gl_vertex_t	points[4];
		rgba_t		colour;
		uint8_t		light;
		GLTexture*	texture;

		quad_3d_t() {
			colour.set(255, 255, 255, 255, 0);
			texture = NULL;
		}
	};
	struct line_3d_t {
		bool				calculated;
		vector<quad_3d_t>	quads;

		line_3d_t() { calculated = false; }
	};

	// Map Structures
	vector<line_3d_t>	lines;

public:
	MapRenderer3D(SLADEMap* map = NULL);
	~MapRenderer3D();

	bool	fullbrightEnabled() { return fullbright; }
	bool	fogEnabled() { return fog; }
	void	enableFullbright(bool enable = true) { fullbright = enable; }
	void	enableFog(bool enable = true) { fog = enable; }

	bool	init();
	void	refresh();

	// Camera
	void	cameraMove(double distance);
	void	cameraTurn(double angle);
	void	cameraMoveUp(double distance);
	void	cameraStrafe(double distance);
	void	cameraPitch(double amount);
	void	cameraUpdateVectors();

	// -- Rendering --
	void	setupView(int width, int height);
	void	setLight(rgba_t& colour, uint8_t light, float alpha = 1.0f);
	void	renderMap();

	// Flats
	void	renderFlats();

	// Walls
	void	setupQuad(quad_3d_t* quad, double x1, double y1, double x2, double y2, double top, double bottom);
	void	setupQuadTexCoords(quad_3d_t* quad, int length, double left, double top, bool pegbottom = false, double sx = 1, double sy = 1);
	void	updateLine(unsigned index);
	void	renderQuad(quad_3d_t* quad, float alpha = 1.0f);
	void	renderWalls();

	// Things
	void	renderThings();

	// Visibility checking
	void	quickVisDiscard();
	float	calcDistFade(double distance, double max = -1);
};

#endif//__MAP_RENDERER_3D_H__
