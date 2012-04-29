
#include "Main.h"
#include "WxStuff.h"
#include "MapRenderer3D.h"
#include "SLADEMap.h"
#include "MathStuff.h"
#include "MapEditorWindow.h"

CVAR(Float, render_max_dist, -1, CVAR_SAVE)
CVAR(Float, render_max_thing_dist, 3000, CVAR_SAVE)
CVAR(Int, render_thing_icon_size, 16, CVAR_SAVE)
CVAR(Bool, render_fog_quality, true, CVAR_SAVE)
CVAR(Bool, render_max_dist_adaptive, true, CVAR_SAVE)
CVAR(Int, render_adaptive_ms, 15, CVAR_SAVE)

MapRenderer3D::MapRenderer3D(SLADEMap* map) {
	// Init variables
	this->map = map;
	this->fog = true;
	this->fullbright = false;
	this->gravity = 0.5;
	this->vbo_ceilings = 0;
	this->vbo_floors = 0;
	this->vbo_walls = 0;
	this->skytex1 = "SKY1";

	// Build skybox circle
	buildSkyCircle();

	// Init other
	init();
}

MapRenderer3D::~MapRenderer3D() {
}

bool MapRenderer3D::init() {
	// Check to enable zdoom udmf extensions
	if (S_CMPNOCASE(theGameConfiguration->udmfNamespace(), "zdoom") && theGameConfiguration->getMapFormat() == MAP_UDMF)
		udmf_zdoom = true;

	// Init camera
	bbox_t bbox = map->getMapBBox();
	cam_position.set(bbox.min.x + ((bbox.max.x - bbox.min.x)*0.5), bbox.min.y + ((bbox.max.y - bbox.min.y)*0.5), 64);
	cam_direction.set(0,1);
	cam_pitch = 0;
	cameraUpdateVectors();

	refresh();

	return true;
}

void MapRenderer3D::refresh() {
	// Clear any existing map data
	lines.clear();
	things.clear();
	dist_lines.clear();
	dist_sectors.clear();

	// Clear VBOs
	if (vbo_floors != 0) {
		glDeleteBuffers(1, &vbo_floors);
		glDeleteBuffers(1, &vbo_ceilings);
		vbo_floors = vbo_ceilings = 0;
	}

	// Set sky texture
	gc_mapinfo_t minf = theGameConfiguration->mapInfo(map->mapName());
	skytex1 = minf.sky1;
	skytex2 = minf.sky2;
	skycol_top.a = 0;
	//wxLogMessage("sky1: %s, sky2: %s", CHR(skytex1), CHR(skytex2));
}

void MapRenderer3D::buildSkyCircle() {
	double rot = 0;
	for (unsigned a = 0; a < 32; a++) {
		sky_circle[a].set(sin(rot), -cos(rot));
		rot -= (3.1415926535897932384626433832795 * 2) / 32.0;
	}

	/*
	// Generate circular coordinates (as quads)
	double rot = 0;
	double srot, crot;
	float tc_x = 0.0f;
	float fade_size = 0.8f;
	float half_fade_size = fade_size * 0.5f;
	for (int a = 0; a < 32; a++) {
		srot = sin(rot);
		crot = cos(rot);
		int c = a*4;

		// Left of quad
		sky_circle_top[c].x = srot;
		sky_circle_top[c].y = crot;
		sky_circle_top[c].tx = tc_x;
		sky_circle_mid[c].x = srot;
		sky_circle_mid[c].y = crot;
		sky_circle_mid[c].tx = tc_x;
		sky_circle_bottom[c].x = srot;
		sky_circle_bottom[c].y = crot;
		sky_circle_bottom[c].tx = tc_x;
		sky_circle_top[c+1].x = srot;
		sky_circle_top[c+1].y = crot;
		sky_circle_top[c+1].tx = tc_x;
		sky_circle_mid[c+1].x = srot;
		sky_circle_mid[c+1].y = crot;
		sky_circle_mid[c+1].tx = tc_x;
		sky_circle_bottom[c+1].x = srot;
		sky_circle_bottom[c+1].y = crot;
		sky_circle_bottom[c+1].tx = tc_x;

		// Rotate to next point on circle
		rot -= (3.1415926535897932384626433832795 * 2) / 32.0;
		srot = sin(rot);
		crot = cos(rot);
		tc_x += 0.125f;

		// Right of quad
		sky_circle_top[c+2].x = srot;
		sky_circle_top[c+2].y = crot;
		sky_circle_top[c+2].tx = tc_x;
		sky_circle_mid[c+2].x = srot;
		sky_circle_mid[c+2].y = crot;
		sky_circle_mid[c+2].tx = tc_x;
		sky_circle_bottom[c+2].x = srot;
		sky_circle_bottom[c+2].y = crot;
		sky_circle_bottom[c+2].tx = tc_x;
		sky_circle_top[c+3].x = srot;
		sky_circle_top[c+3].y = crot;
		sky_circle_top[c+3].tx = tc_x;
		sky_circle_mid[c+3].x = srot;
		sky_circle_mid[c+3].y = crot;
		sky_circle_mid[c+3].tx = tc_x;
		sky_circle_bottom[c+3].x = srot;
		sky_circle_bottom[c+3].y = crot;
		sky_circle_bottom[c+3].tx = tc_x;

		// Top of quad
		sky_circle_top[c].z = 1.0f;
		sky_circle_top[c].alpha = 0.0f;
		sky_circle_top[c].ty = 0.0f;
		sky_circle_top[c+3].z = 1.0f;
		sky_circle_top[c+3].alpha = 0.0f;
		sky_circle_top[c+3].ty = 0.0f;
		sky_circle_mid[c].z = 1.0f-fade_size;
		sky_circle_mid[c].alpha = 1.0f;
		sky_circle_mid[c].ty = fade_size;
		sky_circle_mid[c+3].z = 1.0f-fade_size;
		sky_circle_mid[c+3].alpha = 1.0f;
		sky_circle_mid[c+3].ty = fade_size;
		sky_circle_bottom[c].z = -1.0f+fade_size;
		sky_circle_bottom[c].alpha = 1.0f;
		sky_circle_bottom[c].ty = 2.0f-fade_size;
		sky_circle_bottom[c+3].z = -1.0f+fade_size;
		sky_circle_bottom[c+3].alpha = 1.0f;
		sky_circle_bottom[c+3].ty = 2.0f-fade_size;

		// Bottom of quad
		sky_circle_top[c+1].z = 0.0f;//1.0f-fade_size;
		sky_circle_top[c+1].alpha = 1.0f;
		sky_circle_top[c+1].ty = fade_size;
		sky_circle_top[c+2].z = 0.0f;//1.0f-fade_size;
		sky_circle_top[c+2].alpha = 1.0f;
		sky_circle_top[c+2].ty = fade_size;
		sky_circle_mid[c+1].z = -1.0f+fade_size;
		sky_circle_mid[c+1].alpha = 1.0f;
		sky_circle_mid[c+1].ty = 2.0f-fade_size;
		sky_circle_mid[c+2].z = -1.0f+fade_size;
		sky_circle_mid[c+2].alpha = 1.0f;
		sky_circle_mid[c+2].ty = 2.0f-fade_size;
		sky_circle_bottom[c+1].z = -1.0f;
		sky_circle_bottom[c+1].alpha = 0.0f;
		sky_circle_bottom[c+1].ty = 2.0f;
		sky_circle_bottom[c+2].z = -1.0f;
		sky_circle_bottom[c+2].alpha = 0.0f;
		sky_circle_bottom[c+2].ty = 2.0f;
	}
	*/
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

void MapRenderer3D::cameraSet(fpoint3_t position, fpoint2_t direction) {
	// Set camera position/direction
	cam_position = position;
	cam_direction = direction;
	cam_pitch = 0;

	// Update camera vectors
	cameraUpdateVectors();
}

void MapRenderer3D::cameraApplyGravity(double mult) {
	// Get current sector
	int sector = map->inSector(cam_position.x, cam_position.y);
	if (sector < 0)
		return;

	// Get target height
	int fheight = map->getSector(sector)->intProperty("heightfloor") + 40;
	int cheight = map->getSector(sector)->intProperty("heightceiling");
	if (fheight > cheight - 4)
		fheight = cheight - 4;

	if (cam_position.z > fheight) {
		double diff = cam_position.z - fheight;
		cam_position.z -= (diff*0.3*mult);
		if (cam_position.z < fheight)
			cam_position.z = fheight;
	}

	else if (cam_position.z < fheight) {
		double diff = fheight - cam_position.z;
		cam_position.z += (diff*0.5*mult);
		if (cam_position.z > fheight)
			cam_position.z = fheight;
	}
}

void MapRenderer3D::setupView(int width, int height) {
	// Setup projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float aspect = (float)width / (float)height;
	float max = render_max_dist * 1.5f;
	if (max < 100) max = 20000;
	gluPerspective(60.0f, aspect, 0.5f, max);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Calculate up vector
	fpoint3_t up = cam_strafe.cross(cam_dir3d).normalize();
	
	// Setup camera view
	gluLookAt(cam_position.x, cam_position.y, cam_position.z,
				cam_position.x+cam_dir3d.x, cam_position.y+cam_dir3d.y, cam_position.z+cam_dir3d.z,
				up.x, up.y, up.z);
}

void MapRenderer3D::setLight(rgba_t& colour, uint8_t light, float alpha) {
	// Fullbright
	if (fullbright || light == 255) {
		glDisable(GL_FOG);
		colour.ampf(1.0f, 1.0f, 1.0f, alpha).set_gl(false);
		return;
	}

	// Set fog level
	if (fog) {
		glEnable(GL_FOG);
		float lm = light/200.0f;
		glFogf(GL_FOG_END, (lm * lm * 3000.0f));
	}
	else
		glDisable(GL_FOG);

	// If we have a non-coloured light, darken it a bit to
	// closer resemble the software renderer light level
	float mult = (float)light / 255.0f;
	mult *= (mult * 1.3f);
	glColor4f(colour.fr()*mult, colour.fg()*mult, colour.fb()*mult, colour.fa()*alpha);
}

void MapRenderer3D::renderMap() {
	// Setup GL stuff
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
	glAlphaFunc(GL_GREATER, 0.8f);

	// Setup fog
	GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 0.6f};
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 2.0f);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 3000.0f);
	if (render_fog_quality)
		glHint(GL_FOG_HINT, GL_NICEST);
	else
		glHint(GL_FOG_HINT, GL_FASTEST);

	// Quick distance vis check
	sf::Clock clock;
	quickVisDiscard();
	
	// Render sky
	renderSky();

	// Render walls
	renderWalls();
	
	// Render flats
	renderFlats();

	// Render things
	renderThings();

	// Check elapsed time
	if (render_max_dist_adaptive) {
#if SFML_VERSION_MAJOR < 2	// SFML 1.6: uppercase G, returns time in seconds as a float
		long ms = clock.GetElapsedTime() * 1000;
#else						// SFML 2.0: lowercase G, returns a Time object
		long ms = clock.getElapsedTime().asMilliseconds();
#endif
		if (ms > render_adaptive_ms) {
			render_max_dist = render_max_dist - 100;
			if (render_max_dist < 1000)
				render_max_dist = 1000;
		}
		else if (ms < render_adaptive_ms-5) {
			render_max_dist = render_max_dist + 100;
			if (render_max_dist > 20000)
				render_max_dist = 20000;
		}
	}
	
	// Cleanup gl state
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
}

void MapRenderer3D::renderSkySlice(float top, float bottom, float atop, float abottom, float size, float tx, float ty) {
	float tc_x = 0.0f;
	float tc_y1 = (-top + 1.0f) * (ty * 0.5f);
	float tc_y2 = (-bottom + 1.0f) * (ty * 0.5f);

	glBegin(GL_QUADS);

	// Go through circular points
	for (unsigned a = 0; a < 31; a++) {
		// Top
		glColor4f(1.0f, 1.0f, 1.0f, atop);
		glTexCoord2f(tc_x+tx, tc_y1);	glVertex3f(cam_position.x + (sky_circle[a+1].x * size), cam_position.y - (sky_circle[a+1].y * size), cam_position.z + (top * size));
		glTexCoord2f(tc_x, tc_y1);		glVertex3f(cam_position.x + (sky_circle[a].x * size), cam_position.y - (sky_circle[a].y * size), cam_position.z + (top * size));

		// Bottom
		glColor4f(1.0f, 1.0f, 1.0f, abottom);
		glTexCoord2f(tc_x, tc_y2);		glVertex3f(cam_position.x + (sky_circle[a].x * size), cam_position.y - (sky_circle[a].y * size), cam_position.z + (bottom * size));
		glTexCoord2f(tc_x+tx, tc_y2);	glVertex3f(cam_position.x + (sky_circle[a+1].x * size), cam_position.y - (sky_circle[a+1].y * size), cam_position.z + (bottom * size));

		tc_x += tx;
	}

	// Link last point -> first
	// Top
	glColor4f(1.0f, 1.0f, 1.0f, atop);
	glTexCoord2f(tc_x+tx, tc_y1);	glVertex3f(cam_position.x + (sky_circle[0].x * size), cam_position.y - (sky_circle[0].y * size), cam_position.z + (top * size));
	glTexCoord2f(tc_x, tc_y1);		glVertex3f(cam_position.x + (sky_circle[31].x * size), cam_position.y - (sky_circle[31].y * size), cam_position.z + (top * size));

	// Bottom
	glColor4f(1.0f, 1.0f, 1.0f, abottom);
	glTexCoord2f(tc_x, tc_y2);		glVertex3f(cam_position.x + (sky_circle[31].x * size), cam_position.y - (sky_circle[31].y * size), cam_position.z + (bottom * size));
	glTexCoord2f(tc_x+tx, tc_y2);	glVertex3f(cam_position.x + (sky_circle[0].x * size), cam_position.y - (sky_circle[0].y * size), cam_position.z + (bottom * size));

	glEnd();
}

void MapRenderer3D::renderSky() {
	COL_WHITE.set_gl();
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_2D);
	GLTexture* sky = NULL;

	// Center skybox a bit below the camera view
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -10.0f);

	// Get sky texture
	if (!skytex2.IsEmpty())
		sky = theMapEditor->textureManager().getTexture(skytex2);
	else
		sky = theMapEditor->textureManager().getTexture(skytex1);
	if (sky) {
		// Bind texture
		sky->bind();

		// Get average colour if needed
		if (skycol_top.a == 0) {
			int theight = sky->getHeight() * 0.4;
			skycol_top = sky->averageColour(rect_t(0, 0, sky->getWidth(), theight));
			skycol_bottom = sky->averageColour(rect_t(0, sky->getHeight() - theight, sky->getWidth(), sky->getHeight()));
		}

		// Render top cap
		float size = 64.0f;
		glDisable(GL_TEXTURE_2D);
		skycol_top.set_gl(false);
		glBegin(GL_QUADS);
		glVertex3f(cam_position.x-(size*10), cam_position.y-(size*10), cam_position.z+size);
		glVertex3f(cam_position.x-(size*10), cam_position.y+(size*10), cam_position.z+size);
		glVertex3f(cam_position.x+(size*10), cam_position.y+(size*10), cam_position.z+size);
		glVertex3f(cam_position.x+(size*10), cam_position.y-(size*10), cam_position.z+size);
		glEnd();

		// Render bottom cap
		skycol_bottom.set_gl(false);
		glBegin(GL_QUADS);
		glVertex3f(cam_position.x-(size*10), cam_position.y-(size*10), cam_position.z-size);
		glVertex3f(cam_position.x-(size*10), cam_position.y+(size*10), cam_position.z-size);
		glVertex3f(cam_position.x+(size*10), cam_position.y+(size*10), cam_position.z-size);
		glVertex3f(cam_position.x+(size*10), cam_position.y-(size*10), cam_position.z-size);
		glEnd();

		// Render skybox sides
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_TEXTURE_2D);

		// Check for odd sky sizes
		float tx = 0.125f;
		float ty = 2.0f;
		if (sky->getWidth() > 256)
			tx = 0.125f / ((float)sky->getWidth() / 256.0f);
		if (sky->getHeight() > 128)
			ty = 1.0f;
		
		renderSkySlice(1.0f, 0.5f, 0.0f, 1.0f, size, tx, ty);	// Top
		renderSkySlice(0.5f, -0.5f, 1.0f, 1.0f, size, tx, ty);	// Middle
		renderSkySlice(-0.5f, -1.0f, 1.0f, 0.0f, size, tx, ty);	// Bottom
	}

	glPopMatrix();
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
}

void MapRenderer3D::updateSectorVBO(unsigned index) {
	// Check index
	if (index >= map->nSectors())
		return;

	// --- Update texture coordinates ---
	MapSector* sector = map->getSector(index);
	Polygon2D* poly = sector->getPolygon();

	// Floor
	GLTexture* tex = theMapEditor->textureManager().getTexture(sector->floorTexture());
	if (tex) {
		// Set polygon texture
		poly->setTexture(tex);
		
		// Get scaling/offset info
		double ox = 0;
		double oy = 0;
		double sx = 1;
		double sy = 1;
		double rot = 0;
		
		// Check for UDMF + ZDoom extensions
		if (theGameConfiguration->getMapFormat() == MAP_UDMF && S_CMPNOCASE(theGameConfiguration->udmfNamespace(), "zdoom")) {
			ox = sector->floatProperty("xpanningfloor");
			oy = sector->floatProperty("ypanningfloor");
			sx = sector->floatProperty("xscalefloor");
			sy = sector->floatProperty("yscalefloor");
			rot = sector->floatProperty("rotationfloor");
		}

		poly->updateTextureCoords(sx, sy, ox, oy, rot);
	}

	// Update polygon VBO data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_floors);
	poly->setZ(sector->intProperty("heightfloor"));
	poly->updateVBOData();

	// Ceiling
	tex = theMapEditor->textureManager().getTexture(sector->ceilingTexture());
	if (tex) {
		// Set polygon texture
		poly->setTexture(tex);
		
		// Get scaling/offset info
		double ox = 0;
		double oy = 0;
		double sx = 1;
		double sy = 1;
		double rot = 0;
		
		// Check for UDMF + ZDoom extensions
		if (theGameConfiguration->getMapFormat() == MAP_UDMF && S_CMPNOCASE(theGameConfiguration->udmfNamespace(), "zdoom")) {
			ox = sector->floatProperty("xpanningceiling");
			oy = sector->floatProperty("ypanningceiling");
			sx = sector->floatProperty("xscaleceiling");
			sy = sector->floatProperty("yscaleceiling");
			rot = sector->floatProperty("rotationceiling");
		}

		poly->updateTextureCoords(sx, sy, ox, oy, rot);
	}

	// Update polygon VBO data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ceilings);
	poly->setZ(sector->intProperty("heightceiling"));
	poly->updateVBOData();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapRenderer3D::renderFlats() {
	// Check for map
	if (!map)
		return;

	// Init textures
	glEnable(GL_TEXTURE_2D);
	GLTexture* tex = NULL;
	GLTexture* tex_last = NULL;

	// Use VBOs if supported
	if (GLEW_ARB_vertex_buffer_object)
		return renderFlatsVBO();

	// Render floors
	glCullFace(GL_FRONT);
	rgba_t col;
	uint8_t alpha = 255;
	string skyflat = theGameConfiguration->skyFlat();
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

		// Check distance if needed
		if (render_max_dist > 0) {
			if (dist_sectors[a] > render_max_dist)
				continue;
			// Double-check distance
			dist_sectors[a] = map->distanceToSector(cam_position.x, cam_position.y, a, render_max_dist);
			if (dist_sectors[a] > render_max_dist && !sector->boundingBox().point_within(cam_position.x, cam_position.y))
				continue;
		}

		// Check for sky
		if (sector->floorTexture() == skyflat)
			alpha = 0;
		else
			alpha = 255;

		// Get texture
		tex = theMapEditor->textureManager().getFlat(sector->floorTexture());

		// Bind the texture if needed
		if (tex) {
			if (!tex_last)
				glEnable(GL_TEXTURE_2D);
			if (tex != tex_last)
				tex->bind();
		}
		else if (tex_last)
			glDisable(GL_TEXTURE_2D);
		tex_last = tex;

		// Render flat
		glPushMatrix();
		glTranslated(0, 0, sector->intProperty("heightfloor"));
		col = map->getSectorColour(sector, 1, true);
		col.a = alpha;
		setLight(col, sector->intProperty("lightlevel"), calcDistFade(dist_sectors[a], render_max_dist));
		sector->getPolygon()->render();
		glPopMatrix();
	}

	// Render ceilings
	glCullFace(GL_BACK);
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

		// Check distance if needed
		if (render_max_dist > 0 && dist_sectors[a] > render_max_dist)
			continue;

		// Check for sky
		if (sector->ceilingTexture() == skyflat)
			alpha = 0;
		else
			alpha = 255;

		// Get texture
		tex = theMapEditor->textureManager().getFlat(sector->ceilingTexture());

		// Bind the texture if needed
		if (tex) {
			if (!tex_last)
				glEnable(GL_TEXTURE_2D);
			if (tex != tex_last)
				tex->bind();
		}
		else if (tex_last)
			glDisable(GL_TEXTURE_2D);
		tex_last = tex;

		// Render flat
		glPushMatrix();
		glTranslated(0, 0, sector->intProperty("heightceiling"));
		col = map->getSectorColour(sector, 1, true);
		col.a = alpha;
		setLight(col, sector->intProperty("lightlevel"), calcDistFade(dist_sectors[a], render_max_dist));
		sector->getPolygon()->render();
		glPopMatrix();
	}

	glDisable(GL_TEXTURE_2D);
}

void MapRenderer3D::renderFlatsVBO() {
	// First, check if any polygon vertex data has changed (in this case we need to refresh the entire vbo)
	bool vbo_updated = false;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		Polygon2D* poly = map->getSector(a)->getPolygon();
		if (poly && poly->vboUpdate() > 1) {
			//wxLogMessage("Updating sector %d polygon vbo data", a);
			updateFlatsVBO();
			vbo_updated = true;
		}
	}

	// Create VBO if necessary
	if (!vbo_updated && vbo_floors == 0) {
		updateFlatsVBO();
		vbo_updated = true;
	}

	// Setup opengl state
	glEnable(GL_TEXTURE_2D);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_floors);
	Polygon2D::setupVBOPointers();
	GLTexture* tex_last = NULL;
	GLTexture* tex = NULL;
	rgba_t col;

	// Render floors
	glCullFace(GL_FRONT);
	uint8_t alpha = 255;
	string skyflat = theGameConfiguration->skyFlat();
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

		// Check distance if needed
		if (render_max_dist > 0) {
			if (dist_sectors[a] > render_max_dist)
				continue;
			// Double-check distance
			dist_sectors[a] = map->distanceToSector(cam_position.x, cam_position.y, a, render_max_dist);
			if (dist_sectors[a] > render_max_dist && !sector->boundingBox().point_within(cam_position.x, cam_position.y))
				continue;
		}

		// Update sector VBO data if needed
		if (sector->getPolygon()->vboUpdate() > 0)
			updateSectorVBO(a);

		// Check for sky
		if (sector->floorTexture() == skyflat)
			alpha = 0;
		else
			alpha = 255;

		// Get texture
		tex = theMapEditor->textureManager().getFlat(sector->floorTexture());

		// Bind the texture if needed
		if (tex) {
			if (!tex_last)
				glEnable(GL_TEXTURE_2D);
			if (tex != tex_last)
				tex->bind();
		}
		else if (!tex_last)
			glDisable(GL_TEXTURE_2D);
		tex_last = tex;

		// Render flat
		col = map->getSectorColour(sector, 1, true);
		col.a = alpha;
		setLight(col, sector->intProperty("lightlevel"), calcDistFade(dist_sectors[a], render_max_dist));
		sector->getPolygon()->renderVBO(false);
	}

	// Render ceilings
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ceilings);
	Polygon2D::setupVBOPointers();
	glCullFace(GL_BACK);
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

		// Check distance if needed
		if (render_max_dist > 0 && dist_sectors[a] > render_max_dist)
			continue;

		// Check for sky
		if (sector->ceilingTexture() == skyflat)
			alpha = 0;
		else
			alpha = 255;

		// Get texture
		tex = theMapEditor->textureManager().getFlat(sector->ceilingTexture());

		// Bind the texture if needed
		if (tex) {
			if (!tex_last)
				glEnable(GL_TEXTURE_2D);
			if (tex != tex_last)
				tex->bind();
		}
		else if (!tex_last)
			glDisable(GL_TEXTURE_2D);
		tex_last = tex;

		// Render flat
		col = map->getSectorColour(sector, 1, true);
		col.a = alpha;
		setLight(col, sector->intProperty("lightlevel"), calcDistFade(dist_sectors[a], render_max_dist));
		sector->getPolygon()->renderVBO(false);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Clean up opengl state
	glDisable(GL_TEXTURE_2D);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapRenderer3D::setupQuad(MapRenderer3D::quad_3d_t* quad, double x1, double y1, double x2, double y2, double top, double bottom) {
	// Left
	quad->points[0].x = quad->points[1].x = x1;
	quad->points[0].y = quad->points[1].y = y1;

	// Right
	quad->points[2].x = quad->points[3].x = x2;
	quad->points[2].y = quad->points[3].y = y2;

	// Top/bottom
	quad->points[0].z = quad->points[3].z = top;
	quad->points[1].z = quad->points[2].z = bottom;
}

void MapRenderer3D::setupQuadTexCoords(MapRenderer3D::quad_3d_t* quad, int length, double left, double top, bool pegbottom, double sx, double sy) {
	// Check texture
	if (!quad->texture)
		return;

	// Determine integral height
	int height = MathStuff::round(quad->points[0].z - quad->points[1].z);

	// Initial offsets
	double x1 = left;
	double x2 = left + length;
	double y1 = top;
	double y2 = top + height;
	if (pegbottom) {
		y2 = top + quad->texture->getHeight();
		y1 = y2 - height;
	}

	// Set texture coordinates
	quad->points[0].tx = x1 / (quad->texture->getWidth() * sx);
	quad->points[0].ty = y1 / (quad->texture->getHeight() * sy);
	quad->points[1].tx = x1 / (quad->texture->getWidth() * sx);
	quad->points[1].ty = y2 / (quad->texture->getHeight() * sy);
	quad->points[2].tx = x2 / (quad->texture->getWidth() * sx);
	quad->points[2].ty = y2 / (quad->texture->getHeight() * sy);
	quad->points[3].tx = x2 / (quad->texture->getWidth() * sx);
	quad->points[3].ty = y1 / (quad->texture->getHeight() * sy);
}

void MapRenderer3D::updateLine(unsigned index) {
	// Check index
	if (index > lines.size())
		return;

	// Clear current line data
	lines[index].quads.clear();
	lines[index].flags = 0;

	// Skip invalid line
	MapLine* line = map->getLine(index);
	if (!line->s1()) {
		lines[index].flags |= CALCULATED;
		return;
	}

	// Get relevant line info
	bool upeg = theGameConfiguration->lineBasicFlagSet("dontpegtop", line);
	bool lpeg = theGameConfiguration->lineBasicFlagSet("dontpegbottom", line);
	double xoff, yoff, sx, sy;

	// Get first side info
	int floor1 = line->frontSector()->intProperty("heightfloor");
	int ceiling1 = line->frontSector()->intProperty("heightceiling");
	rgba_t colour1 = map->getSectorColour(line->frontSector(), 0, true);
	int light1 = line->frontSector()->intProperty("lightlevel");
	int xoff1 = line->s1()->intProperty("offsetx");
	int yoff1 = line->s1()->intProperty("offsety");

	// --- One-sided line ---
	int length = MathStuff::round(line->getLength());
	if (line->s1() && !line->s2()) {
		quad_3d_t quad;

		// Determine offsets
		xoff = xoff1;
		yoff = yoff1;
		if (udmf_zdoom) {
			if (line->s1()->hasProp("offsetx_mid"))
				xoff += line->s1()->floatProperty("offsetx_mid");
			if (line->s1()->hasProp("offsety_mid"))
				yoff += line->s1()->floatProperty("offsety_mid");
		}

		// Texture scale
		sx = sy = 1;
		if (udmf_zdoom) {
			if (line->s1()->hasProp("scalex_mid"))
				sx = line->s1()->floatProperty("scalex_mid");
			if (line->s1()->hasProp("scaley_mid"))
				sy = line->s1()->floatProperty("scaley_mid");
		}

		// Create quad
		setupQuad(&quad, line->x1(), line->y1(), line->x2(), line->y2(), ceiling1, floor1);
		quad.colour = colour1;
		quad.light = light1;
		quad.texture = theMapEditor->textureManager().getTexture(line->s1()->stringProperty("texturemiddle"));
		setupQuadTexCoords(&quad, length, xoff1, yoff1, lpeg, sx, sy);

		// Add middle quad and finish
		lines[index].quads.push_back(quad);
		lines[index].flags |= CALCULATED;
		return;
	}

	// --- Two-sided line ---

	// Get second side info
	int floor2 = line->backSector()->intProperty("heightfloor");
	int ceiling2 = line->backSector()->intProperty("heightceiling");
	rgba_t colour2 = map->getSectorColour(line->backSector(), 0, true);
	int light2 = line->backSector()->intProperty("lightlevel");
	int xoff2 = line->s2()->intProperty("offsetx");
	int yoff2 = line->s2()->intProperty("offsety");
	int lowceil = min(ceiling1, ceiling2);
	int highfloor = max(floor1, floor2);
	string sky_flat = theGameConfiguration->skyFlat();

	// Front lower
	if (floor2 > floor1) {
		quad_3d_t quad;

		// Determine offsets
		xoff = xoff1;
		yoff = yoff1;
		if (lpeg)	// Lower unpegged
			yoff += (ceiling1 - floor2);
		if (udmf_zdoom) {
			// ZDoom UDMF extra offsets
			if (line->s1()->hasProp("offsetx_bottom"))
				xoff += line->s1()->floatProperty("offsetx_bottom");
			if (line->s1()->hasProp("offsety_bottom"))
				yoff += line->s1()->floatProperty("offsety_bottom");
		}

		// Texture scale
		sx = sy = 1;
		if (udmf_zdoom) {
			if (line->s1()->hasProp("scalex_bottom"))
				sx = line->s1()->floatProperty("scalex_bottom");
			if (line->s1()->hasProp("scaley_bottom"))
				sy = line->s1()->floatProperty("scaley_bottom");
		}

		// Create quad
		setupQuad(&quad, line->x1(), line->y1(), line->x2(), line->y2(), floor2, floor1);
		quad.colour = colour1;
		quad.light = light1;
		quad.texture = theMapEditor->textureManager().getTexture(line->s1()->stringProperty("texturebottom"));
		setupQuadTexCoords(&quad, length, xoff, yoff, false, sx, sy);
		if (line->backSector()->floorTexture() == sky_flat) quad.flags |= SKY;

		// Add quad
		lines[index].quads.push_back(quad);
	}

	// Front middle
	string midtex1 = line->stringProperty("side1.texturemiddle");
	if (!midtex1.IsEmpty() && midtex1 != "-") {
		quad_3d_t quad;

		// Get texture
		quad.texture = theMapEditor->textureManager().getTexture(midtex1);

		// Determine offsets
		xoff = xoff1;
		yoff = 0;

		// Texture scale
		sx = sy = 1;
		if (udmf_zdoom) {
			if (line->s1()->hasProp("scalex_mid"))
				sx = line->s1()->floatProperty("scalex_mid");
			if (line->s1()->hasProp("scaley_mid"))
				sy = line->s1()->floatProperty("scaley_mid");
		}

		// Setup quad coordinates
		double top = lowceil + yoff1;
		double bottom = top - (quad.texture->getHeight() * sy);
		if (lpeg) {
			bottom = highfloor + yoff1;
			top = bottom + (quad.texture->getHeight() * sy);
		}
		if (bottom < highfloor)
			bottom = highfloor;
		if (top > lowceil) {
			yoff = top - lowceil;
			top = lowceil;
		}

		// Create quad
		setupQuad(&quad, line->x1(), line->y1(), line->x2(), line->y2(), top, bottom);
		quad.colour = colour1;
		quad.light = light1;
		setupQuadTexCoords(&quad, length, xoff, yoff, false, sx, sy);

		// Add quad
		lines[index].quads.push_back(quad);
	}

	// Front upper
	if (ceiling1 > ceiling2) {
		quad_3d_t quad;

		// Determine offsets
		xoff = xoff1;
		yoff = yoff1;
		if (udmf_zdoom) {
			// ZDoom UDMF extra offsets
			if (line->s1()->hasProp("offsetx_top"))
				xoff += line->s1()->floatProperty("offsetx_top");
			if (line->s1()->hasProp("offsety_top"))
				yoff += line->s1()->floatProperty("offsety_top");
		}

		// Texture scale
		sx = sy = 1;
		if (udmf_zdoom) {
			if (line->s1()->hasProp("scalex_top"))
				sx = line->s1()->floatProperty("scalex_top");
			if (line->s1()->hasProp("scaley_top"))
				sy = line->s1()->floatProperty("scaley_top");
		}

		// Create quad
		setupQuad(&quad, line->x1(), line->y1(), line->x2(), line->y2(), ceiling1, ceiling2);
		quad.colour = colour1;
		quad.light = light1;
		quad.texture = theMapEditor->textureManager().getTexture(line->s1()->stringProperty("texturetop"));
		setupQuadTexCoords(&quad, length, xoff, yoff, !upeg, sx, sy);
		if (line->backSector()->ceilingTexture() == sky_flat) quad.flags |= SKY;

		// Add quad
		lines[index].quads.push_back(quad);
	}

	// Back lower
	if (floor1 > floor2) {
		quad_3d_t quad;

		// Determine offsets
		xoff = xoff2;
		yoff = yoff2;
		if (lpeg)	// Lower unpegged
			yoff += (ceiling2 - floor1);
		if (udmf_zdoom) {
			// ZDoom UDMF extra offsets
			if (line->s2()->hasProp("offsetx_bottom"))
				xoff += line->s2()->floatProperty("offsetx_bottom");
			if (line->s2()->hasProp("offsety_bottom"))
				yoff += line->s2()->floatProperty("offsety_bottom");
		}

		// Texture scale
		sx = sy = 1;
		if (udmf_zdoom) {
			if (line->s2()->hasProp("scalex_bottom"))
				sx = line->s2()->floatProperty("scalex_bottom");
			if (line->s2()->hasProp("scaley_bottom"))
				sy = line->s2()->floatProperty("scaley_bottom");
		}

		// Create quad
		setupQuad(&quad, line->x2(), line->y2(), line->x1(), line->y1(), floor1, floor2);
		quad.colour = colour2;
		quad.light = light2;
		quad.texture = theMapEditor->textureManager().getTexture(line->s2()->stringProperty("texturebottom"));
		setupQuadTexCoords(&quad, length, xoff, yoff, false, sx, sy);
		if (line->frontSector()->floorTexture() == sky_flat) quad.flags |= SKY;

		// Add quad
		lines[index].quads.push_back(quad);
	}

	// Back middle
	string midtex2 = line->stringProperty("side2.texturemiddle");
	if (!midtex2.IsEmpty() && midtex2 != "-") {
		quad_3d_t quad;

		// Get texture
		quad.texture = theMapEditor->textureManager().getTexture(midtex2);

		// Determine offsets
		xoff = xoff2;
		yoff = 0;

		// Texture scale
		sx = sy = 1;
		if (udmf_zdoom) {
			if (line->s2()->hasProp("scalex_mid"))
				sx = line->s2()->floatProperty("scalex_mid");
			if (line->s2()->hasProp("scaley_mid"))
				sy = line->s2()->floatProperty("scaley_mid");
		}

		// Setup quad coordinates
		double top = lowceil + yoff2;
		double bottom = top - (quad.texture->getHeight() * sy);
		if (lpeg) {
			bottom = highfloor + yoff2;
			top = bottom + (quad.texture->getHeight() * sy);
		}
		if (bottom < highfloor)
			bottom = highfloor;
		if (top > lowceil) {
			yoff = top - lowceil;
			top = lowceil;
		}

		// Create quad
		setupQuad(&quad, line->x2(), line->y2(), line->x1(), line->y1(), top, bottom);
		quad.colour = colour2;
		quad.light = light2;
		setupQuadTexCoords(&quad, length, xoff, yoff, false, sx, sy);

		// Add quad
		lines[index].quads.push_back(quad);
	}

	// Back upper
	if (ceiling2 > ceiling1) {
		quad_3d_t quad;

		// Determine offsets
		xoff = xoff2;
		yoff = yoff2;
		if (udmf_zdoom) {
			// ZDoom UDMF extra offsets
			if (line->s2()->hasProp("offsetx_top"))
				xoff += line->s2()->floatProperty("offsetx_top");
			if (line->s2()->hasProp("offsety_top"))
				yoff += line->s2()->floatProperty("offsety_top");
		}

		// Texture scale
		sx = sy = 1;
		if (udmf_zdoom) {
			if (line->s2()->hasProp("scalex_top"))
				sx = line->s2()->floatProperty("scalex_top");
			if (line->s2()->hasProp("scaley_top"))
				sy = line->s2()->floatProperty("scaley_top");
		}

		// Create quad
		setupQuad(&quad, line->x2(), line->y2(), line->x1(), line->y1(), ceiling2, ceiling1);
		quad.colour = colour2;
		quad.light = light2;
		quad.texture = theMapEditor->textureManager().getTexture(line->s2()->stringProperty("texturetop"));
		setupQuadTexCoords(&quad, length, xoff, yoff, !upeg, sx, sy);
		if (line->frontSector()->ceilingTexture() == sky_flat) quad.flags |= SKY;

		// Add quad
		lines[index].quads.push_back(quad);
	}


	// Finished
	lines[index].flags |= CALCULATED;
}

void MapRenderer3D::renderQuad(MapRenderer3D::quad_3d_t* quad, float alpha) {
	// Setup colour/light
	if (quad->flags & SKY) {
		alpha = 0;
		glDisable(GL_ALPHA_TEST);
	}
	setLight(quad->colour, quad->light, alpha);

	// Draw quad
	glBegin(GL_QUADS);
	glTexCoord2f(quad->points[0].tx, quad->points[0].ty);	glVertex3f(quad->points[0].x, quad->points[0].y, quad->points[0].z);
	glTexCoord2f(quad->points[1].tx, quad->points[1].ty);	glVertex3f(quad->points[1].x, quad->points[1].y, quad->points[1].z);
	glTexCoord2f(quad->points[2].tx, quad->points[2].ty);	glVertex3f(quad->points[2].x, quad->points[2].y, quad->points[2].z);
	glTexCoord2f(quad->points[3].tx, quad->points[3].ty);	glVertex3f(quad->points[3].x, quad->points[3].y, quad->points[3].z);
	glEnd();

	if (quad->flags & SKY)
		glEnable(GL_ALPHA_TEST);
}

void MapRenderer3D::renderWalls() {
	// Create lines array if empty
	if (lines.size() == 0) {
		for (unsigned a = 0; a < map->nLines(); a++)
			lines.push_back(MapRenderer3D::line_3d_t());
	}

	// Init textures
	glEnable(GL_TEXTURE_2D);
	GLTexture* tex_last = NULL;

	// Draw all lines
	MapLine* line;
	double distfade;
	for (unsigned a = 0; a < lines.size(); a++) {
		// Check distance if needed
		if (render_max_dist > 0) {
			if (dist_lines[a] > render_max_dist)
				continue;
			// Double check distance
			line = map->getLine(a);
			dist_lines[a] = MathStuff::distanceToLine(cam_position.x, cam_position.y, line->x1(), line->y1(), line->x2(), line->y2());
			if (dist_lines[a] > render_max_dist)
				continue;
		}

		// Check for distance fade
		distfade = calcDistFade(dist_lines[a], render_max_dist);

		// Update line if needed
		if ((lines[a].flags & CALCULATED) == 0)
			updateLine(a);

		// Draw each quad on the line
		quad_3d_t* quad;
		for (unsigned q = 0; q < lines[a].quads.size(); q++) {
			// Check we're on the right side of the quad
			quad = &(lines[a].quads[q]);
			if (MathStuff::lineSide(cam_position.x, cam_position.y, quad->points[0].x, quad->points[0].y, quad->points[2].x, quad->points[2].y) < 0)
				continue;

			// Bind the texture if needed
			if (quad->texture) {
				if (!tex_last)
					glEnable(GL_TEXTURE_2D);
				if (lines[a].quads[q].texture != tex_last)
					lines[a].quads[q].texture->bind();
			}
			else if (tex_last)
				glDisable(GL_TEXTURE_2D);
			tex_last = quad->texture;

			// Render quad
			renderQuad(quad, distfade);
		}
	}

	glDisable(GL_TEXTURE_2D);
}

void MapRenderer3D::updateThing(unsigned index, MapThing* thing) {
	// Check index
	if (index >= things.size() || !thing)
		return;

	// Setup thing info
	things[index].type = theGameConfiguration->thingType(thing->getType());
	things[index].sector = map->getSector(map->inSector(thing->xPos(), thing->yPos()));

	// Get sprite texture
	uint32_t theight = render_thing_icon_size;
	things[index].sprite = theMapEditor->textureManager().getSprite(things[index].type->getSprite(), things[index].type->getTranslation(), things[index].type->getPalette());
	if (!things[index].sprite) {
		// Sprite not found, try an icon
		things[index].sprite = theMapEditor->textureManager().getEditorImage(S_FMT("thing/%s", CHR(things[index].type->getIcon())));
		things[index].flags |= ICON;
	}
	else theight = things[index].sprite->getHeight();
	if (!things[index].sprite) {
		// Icon not found either, use unknown icon
		things[index].sprite = theMapEditor->textureManager().getEditorImage("thing/unknown");
	}

	// Determine z position
	if (things[index].sector) {
		// Get sector floor (or ceiling) height
		int sheight = things[index].sector->prop("heightfloor").getIntValue();
		if (things[index].type->isHanging()) {
			sheight = things[index].sector->prop("heightceiling").getIntValue();
			sheight -= theight;
		}

		// Set height
		things[index].z = sheight;
		if (things[index].type->shrinkOnZoom())
			things[index].z -= render_thing_icon_size*0.5;
		if (things[index].z < sheight)
			things[index].z = sheight;
		things[index].z += thing->prop("height").getFloatValue();
	}

	// Adjust height by sprite Y offset if needed
	things[index].z += theMapEditor->textureManager().getVerticalOffset(things[index].type->getSprite());

	things[index].flags |= CALCULATED;
}

void MapRenderer3D::renderThings() {
	// Create things array if empty
	if (things.size() == 0) {
		for (unsigned a = 0; a < map->nThings(); a++)
			things.push_back(thing_3d_t());
	}

	// Init textures
	glEnable(GL_TEXTURE_2D);
	GLTexture* tex_last = NULL;
	GLTexture* tex = NULL;

	// Go through things
	double dist, halfwidth, theight;
	double mdist = render_max_thing_dist;
	if (mdist <= 0 || mdist > render_max_dist) mdist = render_max_dist;
	rgba_t col;
	uint8_t light;
	float x1, y1, x2, y2;
	for (unsigned a = 0; a < map->nThings(); a++) {
		MapThing* thing = map->getThing(a);

		// Check thing distance if needed
		if (mdist > 0) {
			dist = MathStuff::distance(cam_position.x, cam_position.y, thing->xPos(), thing->yPos());
			if (dist > mdist)
				continue;
		}

		// Update thing if needed
		if ((things[a].flags & CALCULATED) == 0)
			updateThing(a, thing);

		// Get thing sprite
		tex = things[a].sprite;

		// Bind texture if needed
		if (tex != tex_last) {
			tex->bind();
			tex_last = tex;
		}

		// Determine coordinates
		halfwidth = tex->getWidth() * 0.5;
		theight = tex->getHeight();
		if (things[a].flags & ICON) {
			halfwidth = render_thing_icon_size*0.5;
			theight = render_thing_icon_size;
		}
		x1 = thing->xPos() - cam_strafe.x * halfwidth;
		y1 = thing->yPos() - cam_strafe.y * halfwidth;
		x2 = thing->xPos() + cam_strafe.x * halfwidth;
		y2 = thing->yPos() + cam_strafe.y * halfwidth;

		// Set colour/brightness
		light = 255;
		if (things[a].type->isFullbright())
			col.set(255, 255, 255, 255);
		else {
			// Get light level from sector
			if (things[a].sector)
				light = things[a].sector->intProperty("lightlevel");
			
			// Icon, use thing icon colour
			if (things[a].flags & ICON)
				col.set(things[a].type->getColour());

			// Otherwise use sector colour
			else if (things[a].sector)
				col.set(map->getSectorColour(things[a].sector, 0, true));
		}
		setLight(col, light, calcDistFade(dist, mdist));

		// Draw thing
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	glVertex3f(x1, y1, things[a].z + theight);
		glTexCoord2f(0.0f, 1.0f);	glVertex3f(x1, y1, things[a].z);
		glTexCoord2f(1.0f, 1.0f);	glVertex3f(x2, y2, things[a].z);
		glTexCoord2f(1.0f, 0.0f);	glVertex3f(x2, y2, things[a].z + theight);
		glEnd();
	}
}

void MapRenderer3D::updateFlatsVBO() {
	// Create VBOs if needed
	if (vbo_floors == 0) {
		glGenBuffers(1, &vbo_floors);
		glGenBuffers(1, &vbo_ceilings);
	}

	// Get total size needed
	unsigned totalsize = 0;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		Polygon2D* poly = map->getSector(a)->getPolygon();
		totalsize += poly->vboDataSize();
	}

	// --- Floors ---

	// Allocate buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_floors);
	glBufferData(GL_ARRAY_BUFFER, totalsize, NULL, GL_STATIC_DRAW);

	// Write polygon data to VBO
	unsigned offset = 0;
	unsigned index = 0;
	int height = 0;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		// Set polygon z height
		Polygon2D* poly = map->getSector(a)->getPolygon();
		height = map->getSector(a)->intProperty("heightfloor");
		poly->setZ(height);

		// Write to VBO
		offset = poly->writeToVBO(offset, index);
		index += poly->totalVertices();
	}

	// --- Ceilings ---

	// Allocate buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ceilings);
	glBufferData(GL_ARRAY_BUFFER, totalsize, NULL, GL_STATIC_DRAW);

	// Write polygon data to VBO
	offset = 0;
	index = 0;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		// Set polygon z height
		Polygon2D* poly = map->getSector(a)->getPolygon();
		height = map->getSector(a)->intProperty("heightceiling");
		poly->setZ(height);

		// Write to VBO
		offset = poly->writeToVBO(offset, index);
		index += poly->totalVertices();

		// Reset polygon z
		poly->setZ(0.0f);
	}

	// Clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapRenderer3D::updateWallsVBO() {
}

void MapRenderer3D::quickVisDiscard() {
	// Check if vis arrays need to be rebuilt
	if (dist_lines.size() != map->nLines()) {
		dist_lines.clear();
		for (unsigned a = 0; a < map->nLines(); a++)
			dist_lines.push_back(0);
	}
	if (dist_sectors.size() != map->nSectors()) {
		dist_sectors.clear();
		for (unsigned a = 0; a < map->nSectors(); a++)
			dist_sectors.push_back(0);
	}

	// Do nothing if no max distance
	if (render_max_dist <= 0)
		return;

	// Go through all sectors
	double x = cam_position.x;
	double y = cam_position.y;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		// Get sector bbox
		bbox_t bbox = map->getSector(a)->boundingBox();

		// Check if within bbox
		if (bbox.point_within(x, y)) {
			dist_sectors[a] = 0;
			continue;
		}

		// Check distance to bbox
		double min_dist = 9999999;
		double dist = MathStuff::distanceToLine(x, y, bbox.min.x, bbox.min.y, bbox.min.x, bbox.max.y);
		if (dist < min_dist) min_dist = dist;
		dist = MathStuff::distanceToLine(x, y, bbox.min.x, bbox.max.y, bbox.max.x, bbox.max.y);
		if (dist < min_dist) min_dist = dist;
		dist = MathStuff::distanceToLine(x, y, bbox.max.x, bbox.max.y, bbox.max.x, bbox.min.y);
		if (dist < min_dist) min_dist = dist;
		dist = MathStuff::distanceToLine(x, y, bbox.max.x, bbox.min.y, bbox.min.x, bbox.min.y);
		if (dist < min_dist) min_dist = dist;

		// Check if out of max distance
		if (min_dist > render_max_dist)
			dist_sectors[a] = render_max_dist+1;
		else
			dist_sectors[a] = 0;
	}

	// Go through all sides
	for (unsigned a = 0; a < map->nSides(); a++) {
		// If this side's sector is out of range, it's parent line is too
		if (dist_sectors[map->getSide(a)->getSector()->getIndex()] > render_max_dist)
			dist_lines[map->getSide(a)->getParentLine()->getIndex()] = render_max_dist+1;
		else
			dist_lines[map->getSide(a)->getParentLine()->getIndex()] = 0;
	}
}

float MapRenderer3D::calcDistFade(double distance, double max) {
	if (max <= 0)
		return 1.0f;

	float faderange = max * 0.2f;
	if (distance > max - faderange)
		return 1.0f - ((distance - (max - faderange)) / faderange);
	else
		return 1.0f;
}
