
#ifndef __MAP_RENDERER_3D_H__
#define __MAP_RENDERER_3D_H__

class SLADEMap;
class MapRenderer3D {
private:
	SLADEMap* map;

	// Camera
	fpoint3_t	cam_position;
	fpoint2_t	cam_direction;
	double		cam_pitch;
	double		cam_angle;
	fpoint3_t	cam_dir3d;
	fpoint3_t	cam_strafe;

public:
	MapRenderer3D(SLADEMap* map = NULL);
	~MapRenderer3D();

	// Camera
	void	cameraMove(double distance);
	void	cameraTurn(double angle);
	void	cameraMoveUp(double distance);
	void	cameraStrafe(double distance);
	void	cameraPitch(double amount);
	void	cameraUpdateVectors();

	// Rendering
	void	setupView(int width, int height);
	void	renderFlats();
	void	renderWalls();
};

#endif//__MAP_RENDERER_3D_H__
