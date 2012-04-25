
#include "Main.h"
#include "MapRenderer3D.h"
#include "SLADEMap.h"
#include "MathStuff.h"

MapRenderer3D::MapRenderer3D(SLADEMap* map) {
	// Init variables
	this->map = map;

	// Init camera
	cam_position.set(0,0,64);
	cam_direction.set(0,1);
	cam_pitch = 0;
	cameraUpdateVectors();
}

MapRenderer3D::~MapRenderer3D() {
}

void MapRenderer3D::cameraMove(double distance) {
	// Move along direction vector
	cam_position.x += cam_dir3d.x * distance;
	cam_position.y += cam_dir3d.y * distance;
	cam_position.z += cam_dir3d.z * distance;
}

void MapRenderer3D::cameraTurn(double angle) {
	// Find rotated view point
	fpoint2_t cp2d(cam_position.x, cam_position.y);
	fpoint2_t nd = MathStuff::rotatePoint(cp2d, cp2d + cam_direction, angle);

	// Update direction
	cam_direction.x = nd.x - cam_position.x;
	cam_direction.y = nd.y - cam_position.y;

	// Update vectors
	cameraUpdateVectors();
}

void MapRenderer3D::cameraMoveUp(double distance) {
	cam_position.z += distance;
}

void MapRenderer3D::cameraStrafe(double distance) {
	// Move along strafe vector
	cam_position.x += cam_strafe.x * distance;
	cam_position.y += cam_strafe.y * distance;
}

void MapRenderer3D::cameraPitch(double amount) {
	// Pitch camera
	cam_pitch += amount;

	// Clamp
	double rad90 = PI*0.5;
	if (cam_pitch > rad90)
		cam_pitch = rad90;
	if (cam_pitch < -rad90)
		cam_pitch = -rad90;

	// Update vectors
	cameraUpdateVectors();
}

void MapRenderer3D::cameraUpdateVectors() {
	// Calculate strafe vector
	cam_strafe = fpoint3_t(cam_direction.x, cam_direction.y, 0).cross(fpoint3_t(0, 0, 1));
	cam_strafe = cam_strafe.normalize();

	// Calculate 3d direction vector
	cam_dir3d = MathStuff::rotateVector3D(fpoint3_t(cam_direction.x, cam_direction.y, 0), cam_strafe, cam_pitch);
	cam_dir3d = cam_dir3d.normalize();
}

void MapRenderer3D::setupView(int width, int height) {
	// Setup projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float aspect = (float)width / (float)height;
	gluPerspective(60.0f, aspect, 0.01f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Calculate up vector
	fpoint3_t up = cam_strafe.cross(cam_dir3d).normalize();
	
	// Setup camera view
	gluLookAt(cam_position.x, cam_position.y, cam_position.z,
				cam_position.x+cam_dir3d.x, cam_position.y+cam_dir3d.y, cam_position.z+cam_dir3d.z,
				up.x, up.y, up.z);
}

void MapRenderer3D::renderFlats() {
	// Check for map
	if (!map)
		return;

	// Testing
	COL_WHITE.set_gl();
	glBegin(GL_LINES);
	for (unsigned a = 0; a < map->nLines(); a++) {
		MapLine* line = map->getLine(a);
		glVertex3d(line->x1(), line->y1(), 0);
		glVertex3d(line->x2(), line->y2(), 0);
	}
	glEnd();
}

void MapRenderer3D::renderWalls() {
}
