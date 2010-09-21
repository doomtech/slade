
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    SLADEMap.cpp
 * Description: SLADEMap class, the internal SLADE map handler.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "SLADEMap.h"

SLADEMap::SLADEMap() {
}

SLADEMap::~SLADEMap() {
}

bool SLADEMap::readMap(Archive* map_entries, uint8_t format) {
	if (format == MAP_DOOM)
		return readDoomMap(map_entries);
	else if (format == MAP_HEXEN)
		return readHexenMap(map_entries);
	else if (format == MAP_DOOM64)
		return readDoom64Map(map_entries);
	else if (format == MAP_UDMF) {
		ArchiveEntry* entry = map_entries->getEntry("TEXTMAP");
		return readUDMFMap(entry);
	}

	return false;
}

bool SLADEMap::readDoomVertexes(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no VERTEXES entry!";
		return false;
	}

	doomvertex_t* vert_data = (doomvertex_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomvertex_t); a++) {
		MapVertex* nv = new MapVertex(vert_data[a]);
		vertices.push_back(nv);
	}
	return true;
}

bool SLADEMap::readDoomSidedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SIDEDEFS entry!";
		return false;
	}

	doomside_t* side_data = (doomside_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomside_t); a++) {
		MapSide* ns = new MapSide(side_data[a]);
		sides.push_back(ns);
	}
	return true;
}

bool SLADEMap::readDoomLinedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no LINEDEFS entry!";
		return false;
	}

	doomline_t* line_data = (doomline_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomline_t); a++) {
		MapLine* nv = new MapLine(line_data[a]);
		lines.push_back(nv);
	}
	return true;
}

bool SLADEMap::readDoomSectors(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SECTORS entry!";
		return false;
	}

	doomsector_t* sect_data = (doomsector_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomsector_t); a++) {
		MapSector* ns = new MapSector(sect_data[a]);
		sectors.push_back(ns);
	}
	return true;
}

bool SLADEMap::readDoomThings(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no THINGS entry!";
		return false;
	}

	doomthing_t* thng_data = (doomthing_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomthing_t); a++) {
		MapThing* nv = new MapThing(thng_data[a]);
		things.push_back(nv);
	}
	return true;
}

bool SLADEMap::readDoomMap(Archive* map_entries) {

	// ---- Read vertices ----
	if (!readDoomVertexes(map_entries->getEntry("VERTEXES")))
		return false;

	// ---- Read sides ----
	if (!readDoomSidedefs(map_entries->getEntry("SIDEDEFS")))
		return false;

	// ---- Read lines ----
	if (!readDoomLinedefs(map_entries->getEntry("LINEDEFS")))
		return false;

	// ---- Read sectors ----
	if (!readDoomSectors(map_entries->getEntry("SECTORS")))
		return false;

	// ---- Read things ----
	if (!readDoomThings(map_entries->getEntry("THINGS")))
		return false;

	return true;
}

bool SLADEMap::readHexenLinedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no LINEDEFS entry!";
		return false;
	}

	hexenline_t* line_data = (hexenline_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(hexenline_t); a++) {
		MapLine* nv = new MapLine(line_data[a]);
		lines.push_back(nv);
	}
	return true;
}

bool SLADEMap::readHexenThings(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no THINGS entry!";
		return false;
	}

	hexenthing_t* thng_data = (hexenthing_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(hexenthing_t); a++) {
		MapThing* nv = new MapThing(thng_data[a]);
		things.push_back(nv);
	}
	return true;
}

bool SLADEMap::readHexenMap(Archive* map_entries) {

	// ---- Read vertices ----
	if (!readDoomVertexes(map_entries->getEntry("VERTEXES")))
		return false;

	// ---- Read sides ----
	if (!readDoomSidedefs(map_entries->getEntry("SIDEDEFS")))
		return false;

	// ---- Read lines ----
	if (!readHexenLinedefs(map_entries->getEntry("LINEDEFS")))
		return false;

	// ---- Read sectors ----
	if (!readDoomSectors(map_entries->getEntry("SECTORS")))
		return false;

	// ---- Read things ----
	if (!readHexenThings(map_entries->getEntry("THINGS")))
		return false;

	return true;
}

bool SLADEMap::readDoom64Vertexes(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no VERTEXES entry!";
		return false;
	}

	doom64vertex_t* vert_data = (doom64vertex_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64vertex_t); a++) {
		MapVertex* nv = new MapVertex(vert_data[a]);
		vertices.push_back(nv);
	}
	return true;
}

bool SLADEMap::readDoom64Sidedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SIDEDEFS entry!";
		return false;
	}

	doom64side_t* side_data = (doom64side_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64side_t); a++) {
		MapSide* ns = new MapSide(side_data[a]);
		sides.push_back(ns);
	}
	return true;
}

bool SLADEMap::readDoom64Linedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no LINEDEFS entry!";
		return false;
	}

	doom64line_t* line_data = (doom64line_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64line_t); a++) {
		MapLine* nv = new MapLine(line_data[a]);
		lines.push_back(nv);
	}
	return true;
}

bool SLADEMap::readDoom64Sectors(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SECTORS entry!";
		return false;
	}

	doom64sector_t* sect_data = (doom64sector_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64sector_t); a++) {
		MapSector* ns = new MapSector(sect_data[a]);
		sectors.push_back(ns);
	}
	return true;
}

bool SLADEMap::readDoom64Things(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no THINGS entry!";
		return false;
	}

	doom64thing_t* thng_data = (doom64thing_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64thing_t); a++) {
		MapThing* nv = new MapThing(thng_data[a]);
		things.push_back(nv);
	}
	return true;
}

bool SLADEMap::readDoom64Map(Archive* map_entries) {

	// ---- Read vertices ----
	if (!readDoom64Vertexes(map_entries->getEntry("VERTEXES")))
		return false;

	// ---- Read sides ----
	if (!readDoom64Sidedefs(map_entries->getEntry("SIDEDEFS")))
		return false;

	// ---- Read lines ----
	if (!readDoom64Linedefs(map_entries->getEntry("LINEDEFS")))
		return false;

	// ---- Read sectors ----
	if (!readDoom64Sectors(map_entries->getEntry("SECTORS")))
		return false;

	// ---- Read things ----
	if (!readDoom64Things(map_entries->getEntry("THINGS")))
		return false;

	return true;
}

bool SLADEMap::readUDMFMap(ArchiveEntry* map_data) {
	Tokenizer udmf_data;
	udmf_data.openMem((char*)map_data->getData(true), map_data->getSize(), map_data->getName());

	string token = udmf_data.getToken();
	while (token != "") {
		// Vertex definition
		if (token == "vertex") {
			MapVertex* new_vertex = new MapVertex();
			if (new_vertex->parseUDMF(udmf_data))
				vertices.push_back(new_vertex);
			else
				delete new_vertex;
		}

		// Line definition
		if (token == "linedef") {
			MapLine* new_line = new MapLine();
			if (new_line->parseUDMF(udmf_data))
				lines.push_back(new_line);
			else
				delete new_line;
		}

		// Side definition
		if (token == "sidedef") {
			MapSide* new_side = new MapSide();
			if (new_side->parseUDMF(udmf_data))
				sides.push_back(new_side);
			else
				delete new_side;
		}

		// Sector definition
		if (token == "sector") {
			MapSector* new_sector = new MapSector();
			if (new_sector->parseUDMF(udmf_data))
				sectors.push_back(new_sector);
			else
				delete new_sector;
		}

		token = udmf_data.getToken();
	}

	return true;
}


void SLADEMap::drawVertices() {
	glPointSize(4.0f);
	rgba_t(0, 255, 0, 255, 1).set_gl();

	glBegin(GL_POINTS);
	for (size_t a = 0; a < vertices.size(); a++)
		glVertex2d(vertices[a]->xPos(), vertices[a]->yPos());
	glEnd();
}

void SLADEMap::drawLines() {
	COL_WHITE.set_gl();

	for (size_t a = 0; a < lines.size(); a++) {
		MapVertex* v1 = lines[a]->v1();
		MapVertex* v2 = lines[a]->v2();

		glBegin(GL_LINES);
		glVertex2d(v1->xPos(), v1->yPos());
		glVertex2d(v2->xPos(), v2->yPos());
		glEnd();
	}
}
