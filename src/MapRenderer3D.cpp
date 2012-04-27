
#include "Main.h"
#include "WxStuff.h"
#include "MapRenderer3D.h"
#include "SLADEMap.h"
#include "MathStuff.h"
#include "MapEditorWindow.h"

MapRenderer3D::MapRenderer3D(SLADEMap* map) {
	// Init variables
	this->map = map;
	this->fog = true;
	this->fullbright = false;

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
	cam_position.set(0,0,64);
	cam_direction.set(0,1);
	cam_pitch = 0;
	cameraUpdateVectors();

	// Setup GL stuff
	

	refresh();

	return true;
}

void MapRenderer3D::refresh() {
	// Clear any existing map data
	lines.clear();
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
	gluPerspective(60.0f, aspect, 0.5f, 20000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Calculate up vector
	fpoint3_t up = cam_strafe.cross(cam_dir3d).normalize();
	
	// Setup camera view
	gluLookAt(cam_position.x, cam_position.y, cam_position.z,
				cam_position.x+cam_dir3d.x, cam_position.y+cam_dir3d.y, cam_position.z+cam_dir3d.z,
				up.x, up.y, up.z);
}

void MapRenderer3D::setLight(rgba_t& colour, uint8_t light) {
	// Fullbright
	if (fullbright || light == 255) {
		glDisable(GL_FOG);
		colour.set_gl(false);
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
	glColor4f(colour.fr()*mult, colour.fg()*mult, colour.fb()*mult, colour.fa());
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
	//glHint(GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 3000.0f);
	
	// Render walls
	renderWalls();
	
	// Render flats
	renderFlats();
	
	// Cleanup gl state
	glDisable(GL_ALPHA_TEST);
}

void MapRenderer3D::renderFlats() {
	// Check for map
	if (!map)
		return;

	// Init textures
	glEnable(GL_TEXTURE_2D);
	GLTexture* tex = NULL;
	GLTexture* tex_last = NULL;

	// Render floors
	glCullFace(GL_FRONT);
	rgba_t col;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

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
		setLight(col, sector->intProperty("lightlevel"));
		sector->getPolygon()->render();
		glPopMatrix();
	}

	// Render ceilings
	glCullFace(GL_BACK);
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

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
		setLight(col, sector->intProperty("lightlevel"));
		sector->getPolygon()->render();
		glPopMatrix();
	}

	glDisable(GL_TEXTURE_2D);
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

	// Skip invalid line
	MapLine* line = map->getLine(index);
	if (!line->s1()) {
		lines[index].calculated = true;
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
		lines[index].calculated = true;
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

		// Add quad
		lines[index].quads.push_back(quad);
	}


	// Finished
	lines[index].calculated = true;
}

void MapRenderer3D::renderQuad(MapRenderer3D::quad_3d_t* quad) {
	// Setup colour/light
	setLight(quad->colour, quad->light);

	// Draw quad
	glBegin(GL_QUADS);
	glTexCoord2f(quad->points[0].tx, quad->points[0].ty);	glVertex3f(quad->points[0].x, quad->points[0].y, quad->points[0].z);
	glTexCoord2f(quad->points[1].tx, quad->points[1].ty);	glVertex3f(quad->points[1].x, quad->points[1].y, quad->points[1].z);
	glTexCoord2f(quad->points[2].tx, quad->points[2].ty);	glVertex3f(quad->points[2].x, quad->points[2].y, quad->points[2].z);
	glTexCoord2f(quad->points[3].tx, quad->points[3].ty);	glVertex3f(quad->points[3].x, quad->points[3].y, quad->points[3].z);
	glEnd();
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
	for (unsigned a = 0; a < lines.size(); a++) {
		// Update line if needed
		if (!lines[a].calculated)
			updateLine(a);

		// Draw each quad on the line
		for (unsigned q = 0; q < lines[a].quads.size(); q++) {
			// Bind the texture if needed
			if (lines[a].quads[q].texture) {
				if (!tex_last)
					glEnable(GL_TEXTURE_2D);
				if (lines[a].quads[q].texture != tex_last)
					lines[a].quads[q].texture->bind();
			}
			else if (tex_last)
				glDisable(GL_TEXTURE_2D);
			tex_last = lines[a].quads[q].texture;

			// Render quad
			renderQuad(&(lines[a].quads[q]));
		}
	}

	glDisable(GL_TEXTURE_2D);
}
