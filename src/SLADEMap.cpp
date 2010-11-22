
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

MapVertex* SLADEMap::getVertex(unsigned index) {
	// Check index
	if (index >= vertices.size())
		return NULL;

	return vertices[index];
}

MapSide* SLADEMap::getSide(unsigned index) {
	// Check index
	if (index >= sides.size())
		return NULL;

	return sides[index];
}

MapLine* SLADEMap::getLine(unsigned index) {
	// Check index
	if (index >= lines.size())
		return NULL;

	return lines[index];
}

MapSector* SLADEMap::getSector(unsigned index) {
	// Check index
	if (index >= sectors.size())
		return NULL;

	return sectors[index];
}

MapThing* SLADEMap::getThing(unsigned index) {
	// Check index
	if (index >= things.size())
		return NULL;

	return things[index];
}

bool SLADEMap::readMap(Archive::mapdesc_t map) {
	if (map.format == MAP_DOOM)
		return readDoomMap(map);
	else if (map.format == MAP_HEXEN)
		return readHexenMap(map);
	else if (map.format == MAP_DOOM64)
		return readDoom64Map(map);
	else if (map.format == MAP_UDMF) {
		//ArchiveEntry* entry = map->getEntry("TEXTMAP");
		//return readUDMFMap(entry);
		return false;
	}

	return false;
}

bool SLADEMap::addVertex(doomvertex_t& v) {
	MapVertex* nv = new MapVertex(v.x, v.y);
	vertices.push_back(nv);
	return true;
}

bool SLADEMap::addSide(doomside_t& s) {
	// Create side
	MapSide* ns = new MapSide(getSector(s.sector));

	// Setup side properties
	ns->prop("texturetop") = wxString::From8BitData(s.tex_upper, 8);
	ns->prop("texturebottom") = wxString::From8BitData(s.tex_lower, 8);
	ns->prop("texturemiddle") = wxString::From8BitData(s.tex_middle, 8);
	ns->prop("offsetx") = s.x_offset;
	ns->prop("offsety") = s.y_offset;

	// Add side
	sides.push_back(ns);
	return true;
}

bool SLADEMap::addLine(doomline_t& l) {
	// Create line
	MapLine* nl = new MapLine(getVertex(l.vertex1), getVertex(l.vertex2), getSide(l.side1), getSide(l.side2));

	// Setup line properties
	nl->prop("arg0") = l.sector_tag;
	nl->prop("special") = l.type;

	// Flags
	nl->prop("blocking") = bool(l.flags & LINE_IMPASSIBLE);
	nl->prop("blockmonsters") = bool(l.flags & LINE_BLOCKMONSTERS);
	nl->prop("twosided") = bool(l.flags & LINE_TWOSIDED);
	nl->prop("dontpegtop") = bool(l.flags & LINE_UPPERUNPEGGED);
	nl->prop("dontpegbottom") = bool(l.flags & LINE_LOWERUNPEGGED);
	nl->prop("secret") = bool(l.flags & LINE_SECRET);
	nl->prop("blocksound") = bool(l.flags & LINE_BLOCKSOUND);
	nl->prop("dontdraw") = bool(l.flags & LINE_NOTONMAP);
	nl->prop("mapped") = bool(l.flags & LINE_STARTONMAP);
	nl->prop("passuse") = bool(l.flags & LINE_BPASSTHROUGH);

	// Add line
	lines.push_back(nl);
	return true;
}

bool SLADEMap::addSector(doomsector_t& s) {
	// Create sector
	MapSector* ns = new MapSector(wxString::FromAscii(s.f_tex, 8), wxString::FromAscii(s.c_tex, 8));

	// Setup sector properties
	ns->prop("heightfloor") = s.f_height;
	ns->prop("heightceiling") = s.c_height;
	ns->prop("lightlevel") = s.light;
	ns->prop("special") = s.special;
	ns->prop("id") = s.tag;

	// Add sector
	sectors.push_back(ns);
	return true;
}

bool SLADEMap::addThing(doomthing_t& t) {
	// Create thing
	MapThing* nt = new MapThing(t.x, t.y, t.type);

	// Setup thing properties
	nt->prop("angle") = t.angle;

	// Flags
	nt->prop("skill1") = bool(t.flags & THING_EASY);
	nt->prop("skill2") = bool(t.flags & THING_EASY);
	nt->prop("skill3") = bool(t.flags & THING_MEDIUM);
	nt->prop("skill4") = bool(t.flags & THING_HARD);
	nt->prop("skill5") = bool(t.flags & THING_HARD);
	nt->prop("ambush") = bool(t.flags & THING_DEAF);
	nt->prop("single") = !bool(t.flags & THING_MULTI);
	nt->prop("dm") = !bool(t.flags & THING_BNOTDM);
	nt->prop("coop") = !bool(t.flags & THING_BNOTCOOP);

	// Add thing
	things.push_back(nt);
	return true;
}

bool SLADEMap::readDoomVertexes(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no VERTEXES entry!";
		wxLogMessage(Global::error);
		return false;
	}

	doomvertex_t* vert_data = (doomvertex_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomvertex_t); a++)
		addVertex(vert_data[a]);

	wxLogMessage("Read %d vertices", vertices.size());

	return true;
}

bool SLADEMap::readDoomSidedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SIDEDEFS entry!";
		wxLogMessage(Global::error);
		return false;
	}

	doomside_t* side_data = (doomside_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomside_t); a++)
		addSide(side_data[a]);

	wxLogMessage("Read %d sides", sides.size());

	return true;
}

bool SLADEMap::readDoomLinedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no LINEDEFS entry!";
		wxLogMessage(Global::error);
		return false;
	}

	doomline_t* line_data = (doomline_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomline_t); a++)
		addLine(line_data[a]);

	wxLogMessage("Read %d lines", lines.size());

	return true;
}

bool SLADEMap::readDoomSectors(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SECTORS entry!";
		wxLogMessage(Global::error);
		return false;
	}

	doomsector_t* sect_data = (doomsector_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomsector_t); a++)
		addSector(sect_data[a]);

	wxLogMessage("Read %d sectors", sectors.size());

	return true;
}

bool SLADEMap::readDoomThings(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no THINGS entry!";
		wxLogMessage(Global::error);
		return false;
	}

	doomthing_t* thng_data = (doomthing_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomthing_t); a++)
		addThing(thng_data[a]);

	wxLogMessage("Read %d things", things.size());

	return true;
}

bool SLADEMap::readDoomMap(Archive::mapdesc_t map) {
	wxLogMessage("Reading Doom format map");

	// Find map entries
	ArchiveEntry* v = NULL;
	ArchiveEntry* si = NULL;
	ArchiveEntry* l = NULL;
	ArchiveEntry* se = NULL;
	ArchiveEntry* t = NULL;
	ArchiveEntry* entry = map.head;
	while (entry != map.end->nextEntry()) {
		EntryType* type = entry->getType();

		if (!v && type->getId() == "map_vertexes")
			v = entry;
		else if (!si && type->getId() == "map_sidedefs")
			si = entry;
		else if (!l && type->getId() == "map_linedefs")
			l = entry;
		else if (!se && type->getId() == "map_sectors")
			se = entry;
		else if (!t && type->getId() == "map_things")
			t = entry;

		// Next entry
		entry = entry->nextEntry();
	}

	// ---- Read vertices ----
	if (!readDoomVertexes(v))
		return false;

	// ---- Read sectors ----
	if (!readDoomSectors(se))
		return false;

	// ---- Read sides ----
	if (!readDoomSidedefs(si))
		return false;

	// ---- Read lines ----
	if (!readDoomLinedefs(l))
		return false;

	// ---- Read things ----
	if (!readDoomThings(t))
		return false;

	// Remove detached vertices
	removeDetachedVertices();

	return true;
}

bool SLADEMap::addLine(hexenline_t& l) {
	// Create line
	MapLine* nl = new MapLine(getVertex(l.vertex1), getVertex(l.vertex2), getSide(l.side1), getSide(l.side2));

	// Setup line properties
	nl->prop("arg0") = l.args[0];
	nl->prop("arg1") = l.args[1];
	nl->prop("arg2") = l.args[2];
	nl->prop("arg3") = l.args[3];
	nl->prop("arg4") = l.args[4];
	nl->prop("special") = l.type;

	// Flags
	nl->prop("blocking") = bool(l.flags & LINE_IMPASSIBLE);
	nl->prop("blockmonsters") = bool(l.flags & LINE_BLOCKMONSTERS);
	nl->prop("twosided") = bool(l.flags & LINE_TWOSIDED);
	nl->prop("dontpegtop") = bool(l.flags & LINE_UPPERUNPEGGED);
	nl->prop("dontpegbottom") = bool(l.flags & LINE_LOWERUNPEGGED);
	nl->prop("secret") = bool(l.flags & LINE_SECRET);
	nl->prop("blocksound") = bool(l.flags & LINE_BLOCKSOUND);
	nl->prop("dontdraw") = bool(l.flags & LINE_NOTONMAP);
	nl->prop("mapped") = bool(l.flags & LINE_STARTONMAP);
	nl->prop("repeatspecial") = bool(l.flags & LINE_HREPEATABLE);
	// TODO: SPAC stuff

	// Add line
	lines.push_back(nl);
	return true;
}

bool SLADEMap::addThing(hexenthing_t& t) {
	// Create thing
	MapThing* nt = new MapThing(t.x, t.y, t.type);

	// Setup thing properties
	nt->prop("angle") = t.angle;
	nt->prop("z") = (double)t.z;
	nt->prop("special") = t.special;
	nt->prop("id") = t.tid;
	nt->prop("arg0") = t.args[0];
	nt->prop("arg1") = t.args[1];
	nt->prop("arg2") = t.args[2];
	nt->prop("arg3") = t.args[3];
	nt->prop("arg4") = t.args[4];

	// Flags
	nt->prop("skill1") = bool(t.flags & THING_EASY);
	nt->prop("skill2") = bool(t.flags & THING_EASY);
	nt->prop("skill3") = bool(t.flags & THING_MEDIUM);
	nt->prop("skill4") = bool(t.flags & THING_HARD);
	nt->prop("skill5") = bool(t.flags & THING_HARD);
	nt->prop("ambush") = bool(t.flags & THING_DEAF);
	nt->prop("dormant") = bool(t.flags & THING_HDORMANT);
	nt->prop("class1") = bool(t.flags & THING_HCLASS1);
	nt->prop("class2") = bool(t.flags & THING_HCLASS2);
	nt->prop("class3") = bool(t.flags & THING_HCLASS3);
	nt->prop("single") = bool(t.flags & THING_HSP);
	nt->prop("dm") = bool(t.flags & THING_HDM);
	nt->prop("coop") = bool(t.flags & THING_HCOOP);

	// Add thing
	things.push_back(nt);
	return true;
}

bool SLADEMap::readHexenLinedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no LINEDEFS entry!";
		return false;
	}

	hexenline_t* line_data = (hexenline_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(hexenline_t); a++)
		addLine(line_data[a]);

	return true;
}

bool SLADEMap::readHexenThings(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no THINGS entry!";
		return false;
	}

	hexenthing_t* thng_data = (hexenthing_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(hexenthing_t); a++)
		addThing(thng_data[a]);

	return true;
}

bool SLADEMap::readHexenMap(Archive::mapdesc_t map) {
	wxLogMessage("Reading Hexen format map");

	// Find map entries
	ArchiveEntry* v = NULL;
	ArchiveEntry* si = NULL;
	ArchiveEntry* l = NULL;
	ArchiveEntry* se = NULL;
	ArchiveEntry* t = NULL;
	ArchiveEntry* entry = map.head;
	while (entry != map.end->nextEntry()) {
		EntryType* type = entry->getType();

		if (!v && type->getId() == "map_vertexes")
			v = entry;
		else if (!si && type->getId() == "map_sidedefs")
			si = entry;
		else if (!l && type->getId() == "map_linedefs")
			l = entry;
		else if (!se && type->getId() == "map_sectors")
			se = entry;
		else if (!t && type->getId() == "map_things")
			t = entry;

		// Next entry
		entry = entry->nextEntry();
	}

	// ---- Read vertices ----
	if (!readDoomVertexes(v))
		return false;

	// ---- Read sectors ----
	if (!readDoomSectors(se))
		return false;

	// ---- Read sides ----
	if (!readDoomSidedefs(si))
		return false;

	// ---- Read lines ----
	if (!readHexenLinedefs(l))
		return false;

	// ---- Read things ----
	if (!readHexenThings(t))
		return false;

	// Remove detached vertices
	removeDetachedVertices();

	return true;
}

bool SLADEMap::readDoom64Vertexes(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no VERTEXES entry!";
		return false;
	}

	doom64vertex_t* vert_data = (doom64vertex_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64vertex_t); a++)
		addVertex(vert_data[a]);

	return true;
}

bool SLADEMap::readDoom64Sidedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SIDEDEFS entry!";
		return false;
	}

	doom64side_t* side_data = (doom64side_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64side_t); a++)
		addSide(side_data[a]);

	return true;
}

bool SLADEMap::readDoom64Linedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no LINEDEFS entry!";
		return false;
	}

	doom64line_t* line_data = (doom64line_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64line_t); a++)
		addLine(line_data[a]);

	return true;
}

bool SLADEMap::readDoom64Sectors(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SECTORS entry!";
		return false;
	}

	doom64sector_t* sect_data = (doom64sector_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64sector_t); a++)
		addSector(sect_data[a]);

	return true;
}

bool SLADEMap::readDoom64Things(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no THINGS entry!";
		return false;
	}

	doom64thing_t* thng_data = (doom64thing_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doom64thing_t); a++)
		addThing(thng_data[a]);

	return true;
}

bool SLADEMap::readDoom64Map(Archive::mapdesc_t map) {
	wxLogMessage("Reading Doom format map");

	// Find map entries
	ArchiveEntry* v = NULL;
	ArchiveEntry* si = NULL;
	ArchiveEntry* l = NULL;
	ArchiveEntry* se = NULL;
	ArchiveEntry* t = NULL;
	ArchiveEntry* entry = map.head;
	while (entry != map.end->nextEntry()) {
		EntryType* type = entry->getType();

		if (!v && type->getId() == "map_vertexes")
			v = entry;
		else if (!si && type->getId() == "map_sidedefs")
			si = entry;
		else if (!l && type->getId() == "map_linedefs")
			l = entry;
		else if (!se && type->getId() == "map_sectors")
			se = entry;
		else if (!t && type->getId() == "map_things")
			t = entry;

		// Next entry
		entry = entry->nextEntry();
	}

	// ---- Read vertices ----
	if (!readDoomVertexes(v))
		return false;

	// ---- Read sectors ----
	if (!readDoomSectors(se))
		return false;

	// ---- Read sides ----
	if (!readDoomSidedefs(si))
		return false;

	// ---- Read lines ----
	if (!readDoomLinedefs(l))
		return false;

	// ---- Read things ----
	if (!readDoomThings(t))
		return false;

	// Remove detached vertices
	removeDetachedVertices();

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


void SLADEMap::clearMap() {
	// Clear vertices
	for (unsigned a = 0; a < vertices.size(); a++)
		delete vertices[a];
	vertices.clear();

	// Clear sides
	for (unsigned a = 0; a < sides.size(); a++)
		delete sides[a];
	sides.clear();

	// Clear lines
	for (unsigned a = 0; a < lines.size(); a++)
		delete lines[a];
	lines.clear();

	// Clear sectors
	for (unsigned a = 0; a < sectors.size(); a++)
		delete sectors[a];
	sectors.clear();

	// Clear things
	for (unsigned a = 0; a < things.size(); a++)
		delete things[a];
	things.clear();
}

void SLADEMap::drawVertices() {
	rgba_t(140, 140, 255, 255, 0).set_gl();

	glBegin(GL_POINTS);
	for (size_t a = 0; a < vertices.size(); a++)
		glVertex2d(vertices[a]->xPos(), vertices[a]->yPos());
	glEnd();
}

void SLADEMap::drawLines() {
	glEnable(GL_LINE_SMOOTH);

	for (size_t a = 0; a < lines.size(); a++)
		lines[a]->draw(false);
}


int SLADEMap::removeDetachedVertices() {
	int count = 0;

	vector<MapVertex*>::iterator i = vertices.begin();
	while (i != vertices.end()) {
		if ((*i)->nConnectedLines() == 0) {
			delete (*i);
			vertices.erase(i);
			count++;
		}
		else
			i++;
	}

	return count;
}
