
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
#include "Parser.h"
#include "MathStuff.h"
#include "ResourceManager.h"

SLADEMap::SLADEMap() {
	// Init variables
	this->i_lines = false;
	this->i_sides = false;
	this->i_sectors = false;
	this->i_vertices = false;
	this->i_things = false;
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

int	SLADEMap::vertexIndex(MapVertex* v) {
	// Check vertex was given
	if (!v)
		return -1;

	// Check if item index is valid
	if (i_vertices)
		return v->index;

	// Find vertex
	for (unsigned a = 0; a < vertices.size(); a++) {
		if (vertices[a] == v)
			return a;
	}

	// Not found
	return -1;
}

int	SLADEMap::sideIndex(MapSide* s) {
	// Check side was given
	if (!s)
		return -1;

	// Check if item index is valid
	if (i_sides)
		return s->index;

	// Find side
	for (unsigned a = 0; a < sides.size(); a++) {
		if (sides[a] == s)
			return a;
	}

	// Not found
	return -1;
}

int	SLADEMap::lineIndex(MapLine* l) {
	// Check line was given
	if (!l)
		return -1;

	// Check if item index is valid
	if (i_lines)
		return l->index;

	// Find line
	for (unsigned a = 0; a < lines.size(); a++) {
		if (lines[a] == l)
			return a;
	}

	// Not found
	return -1;
}

int	SLADEMap::sectorIndex(MapSector* s) {
	// Check sector was given
	if (!s)
		return -1;

	// Check if item index is valid
	if (i_sectors)
		return s->index;

	// Find sector
	for (unsigned a = 0; a < sectors.size(); a++) {
		if (sectors[a] == s)
			return a;
	}

	// Not found
	return -1;
}

int	SLADEMap::thingIndex(MapThing* t) {
	// Check thing was given
	if (!t)
		return -1;

	// Check if item index is valid
	if (i_things)
		return t->index;

	// Find thing
	for (unsigned a = 0; a < things.size(); a++) {
		if (things[a] == t)
			return a;
	}

	// Not found
	return -1;
}

void SLADEMap::refreshIndices() {
	// Vertex indices
	i_vertices = true;
	for (unsigned a = 0; a < vertices.size(); a++)
		vertices[a]->index = a;

	// Side indices
	i_sides = true;
	for (unsigned a = 0; a < sides.size(); a++)
		sides[a]->index = a;

	// Line indices
	i_lines = true;
	for (unsigned a = 0; a < lines.size(); a++)
		lines[a]->index = a;

	// Sector indices
	i_sectors = true;
	for (unsigned a = 0; a < sectors.size(); a++)
		sectors[a]->index = a;

	// Thing indices
	i_things = true;
	for (unsigned a = 0; a < things.size(); a++)
		things[a]->index = a;
}

bool SLADEMap::readMap(Archive::mapdesc_t map) {
	if (map.format == MAP_DOOM)
		return readDoomMap(map);
	else if (map.format == MAP_HEXEN)
		return readHexenMap(map);
	else if (map.format == MAP_DOOM64)
		return readDoom64Map(map);
	else if (map.format == MAP_UDMF) {
		return readUDMFMap(map);
	}

	return false;
}

bool SLADEMap::addVertex(doomvertex_t& v) {
	MapVertex* nv = new MapVertex(v.x, v.y);
	vertices.push_back(nv);
	return true;
}

bool SLADEMap::addVertex(doom64vertex_t& v) {
	MapVertex* nv = new MapVertex((double)v.x/65536, (double)v.y/65536);
	vertices.push_back(nv);
	return true;
}

bool SLADEMap::addSide(doomside_t& s) {
	// Create side
	MapSide* ns = new MapSide(getSector(s.sector));

	// Setup side properties
	ns->prop("texturetop") = wxString::FromAscii(s.tex_upper, 8);
	ns->prop("texturebottom") = wxString::FromAscii(s.tex_lower, 8);
	ns->prop("texturemiddle") = wxString::FromAscii(s.tex_middle, 8);
	ns->prop("offsetx") = s.x_offset;
	ns->prop("offsety") = s.y_offset;

	// Add side
	sides.push_back(ns);
	return true;
}

bool SLADEMap::addSide(doom64side_t& s) {
	// Create side
	MapSide* ns = new MapSide(getSector(s.sector));

	// Setup side properties
	ns->prop("texturetop") = theResourceManager->getTextureName(s.tex_upper);
	ns->prop("texturebottom") = theResourceManager->getTextureName(s.tex_lower);
	ns->prop("texturemiddle") = theResourceManager->getTextureName(s.tex_middle);
	ns->prop("offsetx") = s.x_offset;
	ns->prop("offsety") = s.y_offset;

	// Add side
	sides.push_back(ns);
	return true;
}

bool SLADEMap::addLine(doomline_t& l) {
	// Get relevant sides
	MapSide* s1 = NULL;
	MapSide* s2 = NULL;
	if (sides.size() > 32767) {
		// Support for > 32768 sides
		s1 = getSide(static_cast<unsigned short>(l.side1));
		s2 = getSide(static_cast<unsigned short>(l.side2));
	}
	else {
		s1 = getSide(l.side1);
		s2 = getSide(l.side2);
	}

	// Check if side1 already belongs to a line
	if (s1 && s1->parent) {
		// Duplicate side
		s1 = new MapSide(*s1);
		s1->setSector(s1->getSector());
		sides.push_back(s1);
	}

	// Check if side2 already belongs to a line
	if (s2 && s2->parent) {
		s2 = new MapSide(*s2);
		s2->setSector(s2->getSector());
		sides.push_back(s2);
	}

	// Create line
	MapLine* nl = new MapLine(getVertex(l.vertex1), getVertex(l.vertex2), s1, s2);

	// Setup line properties
	nl->prop("arg0") = l.sector_tag;
	nl->prop("special") = l.type;
	nl->prop("flags") = l.flags;

	// Flags
	/*
	nl->prop("blocking") = ((l.flags & LINE_IMPASSIBLE) != 0);
	nl->prop("blockmonsters") = ((l.flags & LINE_BLOCKMONSTERS) != 0);
	nl->prop("twosided") = ((l.flags & LINE_TWOSIDED) != 0);
	nl->prop("dontpegtop") = ((l.flags & LINE_UPPERUNPEGGED) != 0);
	nl->prop("dontpegbottom") = ((l.flags & LINE_LOWERUNPEGGED) != 0);
	nl->prop("secret") = ((l.flags & LINE_SECRET) != 0);
	nl->prop("blocksound") = ((l.flags & LINE_BLOCKSOUND) != 0);
	nl->prop("dontdraw") = ((l.flags & LINE_NOTONMAP) != 0);
	nl->prop("mapped") = ((l.flags & LINE_STARTONMAP) != 0);
	nl->prop("passuse") = ((l.flags & LINE_BPASSTHROUGH) != 0);
	*/

	// Add line
	lines.push_back(nl);
	return true;
}

bool SLADEMap::addLine(doom64line_t& l) {
	// Get relevant sides
	MapSide* s1 = NULL;
	MapSide* s2 = NULL;
	if (sides.size() > 32767) {
		// Support for > 32768 sides
		s1 = getSide(static_cast<unsigned short>(l.side1));
		s2 = getSide(static_cast<unsigned short>(l.side2));
	}
	else {
		s1 = getSide(l.side1);
		s2 = getSide(l.side2);
	}

	// Check if side1 already belongs to a line
	if (s1 && s1->parent) {
		// Duplicate side
		s1 = new MapSide(*s1);
		s1->setSector(s1->getSector());
		sides.push_back(s1);
	}

	// Check if side2 already belongs to a line
	if (s2 && s2->parent) {
		s2 = new MapSide(*s2);
		s2->setSector(s2->getSector());
		sides.push_back(s2);
	}

	// Create line
	MapLine* nl = new MapLine(getVertex(l.vertex1), getVertex(l.vertex2), s1, s2);

	// Setup line properties
	nl->prop("arg0") = l.sector_tag;
	if (l.type & 0x100)
		nl->prop("macro") = l.type & 0xFF;
	else
		nl->prop("special") = l.type & 0xFF;
	nl->prop("flags") = (int)l.flags;
	nl->prop("extraflags") = l.type >> 9;

	// Flags
	/*
	later
	*/

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

bool SLADEMap::addSector(doom64sector_t& s) {
	// Create sector
	// We need to retrieve the texture name from the hash value
	MapSector* ns = new MapSector(theResourceManager->getTextureName(s.f_tex),
		theResourceManager->getTextureName(s.c_tex));

	// Setup sector properties
	ns->prop("heightfloor") = s.f_height;
	ns->prop("heightceiling") = s.c_height;
	ns->prop("lightlevel") = 255;
	ns->prop("special") = s.special;
	ns->prop("id") = s.tag;
	ns->prop("flags") = s.flags;
	ns->prop("color_things") = s.color[0];
	ns->prop("color_floor") = s.color[1];
	ns->prop("color_ceiling") = s.color[2];
	ns->prop("color_upper") = s.color[3];
	ns->prop("color_lower") = s.color[4];

	// Add sector
	sectors.push_back(ns);
	return true;
}

bool SLADEMap::addThing(doomthing_t& t) {
	// Create thing
	MapThing* nt = new MapThing(t.x, t.y, t.type);

	// Setup thing properties
	nt->prop("angle") = t.angle;
	nt->prop("flags") = t.flags;

	// Flags
	/*
	nt->prop("skill1") = ((t.flags & THING_EASY)!=0);
	nt->prop("skill2") = ((t.flags & THING_EASY)!=0);
	nt->prop("skill3") = ((t.flags & THING_MEDIUM)!=0);
	nt->prop("skill4") = ((t.flags & THING_HARD)!=0);
	nt->prop("skill5") = ((t.flags & THING_HARD)!=0);
	nt->prop("ambush") = ((t.flags & THING_DEAF)!=0);
	nt->prop("single") = !((t.flags & THING_MULTI)!=0);
	nt->prop("dm") = !((t.flags & THING_BNOTDM)!=0);
	nt->prop("coop") = !((t.flags & THING_BNOTCOOP)!=0);
	nt->prop("raw_flags") = t.flags;
	*/

	// Add thing
	things.push_back(nt);
	return true;
}

bool SLADEMap::addThing(doom64thing_t& t) {
	// Create thing
	MapThing* nt = new MapThing(t.x, t.y, t.type);

	// Setup thing properties
	nt->prop("angle") = t.angle;
	nt->prop("z") = (double)t.z;
	nt->prop("flags") = t.flags;
	nt->prop("id") = t.tid;

	// Flags
	/*
	later
	*/

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

	// Update item indices
	refreshIndices();

	// Update sector bounding boxes
	for (unsigned a = 0; a < sectors.size(); a++)
		sectors[a]->updateBBox();

	return true;
}

bool SLADEMap::addLine(hexenline_t& l) {
	// Get relevant sides
	MapSide* s1 = NULL;
	MapSide* s2 = NULL;
	if (sides.size() > 32767) {
		// Support for > 32768 sides
		s1 = getSide(static_cast<unsigned short>(l.side1));
		s2 = getSide(static_cast<unsigned short>(l.side2));
	}
	else {
		s1 = getSide(l.side1);
		s2 = getSide(l.side2);
	}

	// Check if side1 already belongs to a line
	if (s1 && s1->parent) {
		// Duplicate side
		s1 = new MapSide(*s1);
		sides.push_back(s1);
	}

	// Check if side2 already belongs to a line
	if (s2 && s2->parent) {
		s2 = new MapSide(*s2);
		sides.push_back(s2);
	}

	// Create line
	MapLine* nl = new MapLine(getVertex(l.vertex1), getVertex(l.vertex2), s1, s2);

	// Setup line properties
	nl->prop("arg0") = l.args[0];
	nl->prop("arg1") = l.args[1];
	nl->prop("arg2") = l.args[2];
	nl->prop("arg3") = l.args[3];
	nl->prop("arg4") = l.args[4];
	nl->prop("special") = l.type;
	nl->prop("flags") = l.flags;

	// Flags
	/*
	nl->prop("blocking") = ((l.flags & LINE_IMPASSIBLE)!=0);
	nl->prop("blockmonsters") = ((l.flags & LINE_BLOCKMONSTERS)!=0);
	nl->prop("twosided") = ((l.flags & LINE_TWOSIDED)!=0);
	nl->prop("dontpegtop") = ((l.flags & LINE_UPPERUNPEGGED)!=0);
	nl->prop("dontpegbottom") = ((l.flags & LINE_LOWERUNPEGGED)!=0);
	nl->prop("secret") = ((l.flags & LINE_SECRET)!=0);
	nl->prop("blocksound") = ((l.flags & LINE_BLOCKSOUND)!=0);
	nl->prop("dontdraw") = ((l.flags & LINE_NOTONMAP)!=0);
	nl->prop("mapped") = ((l.flags & LINE_STARTONMAP)!=0);
	nl->prop("repeatspecial") = ((l.flags & LINE_HREPEATABLE)!=0);
	// TODO: SPAC stuff
	*/

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
	nt->prop("flags") = t.flags;
	nt->prop("id") = t.tid;
	nt->prop("arg0") = t.args[0];
	nt->prop("arg1") = t.args[1];
	nt->prop("arg2") = t.args[2];
	nt->prop("arg3") = t.args[3];
	nt->prop("arg4") = t.args[4];

	// Flags
	/*
	nt->prop("skill1") = ((t.flags & THING_EASY)!=0);
	nt->prop("skill2") = ((t.flags & THING_EASY)!=0);
	nt->prop("skill3") = ((t.flags & THING_MEDIUM)!=0);
	nt->prop("skill4") = ((t.flags & THING_HARD)!=0);
	nt->prop("skill5") = ((t.flags & THING_HARD)!=0);
	nt->prop("ambush") = ((t.flags & THING_DEAF)!=0);
	nt->prop("dormant") = ((t.flags & THING_HDORMANT)!=0);
	nt->prop("class1") = ((t.flags & THING_HCLASS1)!=0);
	nt->prop("class2") = ((t.flags & THING_HCLASS2)!=0);
	nt->prop("class3") = ((t.flags & THING_HCLASS3)!=0);
	nt->prop("single") = ((t.flags & THING_HSP)!=0);
	nt->prop("dm") = ((t.flags & THING_HDM)!=0);
	nt->prop("coop") = ((t.flags & THING_HCOOP)!=0);
	nt->prop("raw_flags") = t.flags;
	*/

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

	// Update item indices
	refreshIndices();

	// Update sector bounding boxes
	for (unsigned a = 0; a < sectors.size(); a++)
		sectors[a]->updateBBox();

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

	wxLogMessage("Read %d vertices", vertices.size());

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

	wxLogMessage("Read %d sides", sides.size());

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

	wxLogMessage("Read %d lines", lines.size());

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

	wxLogMessage("Read %d sectors", sectors.size());

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

	wxLogMessage("Read %d things", things.size());

	return true;
}

bool SLADEMap::readDoom64Map(Archive::mapdesc_t map) {
	wxLogMessage("Reading Doom 64 format map");

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
	if (!readDoom64Vertexes(v))
		return false;

	// ---- Read sectors ----
	if (!readDoom64Sectors(se))
		return false;

	// ---- Read sides ----
	if (!readDoom64Sidedefs(si))
		return false;

	// ---- Read lines ----
	if (!readDoom64Linedefs(l))
		return false;

	// ---- Read things ----
	if (!readDoom64Things(t))
		return false;

	// Remove detached vertices
	removeDetachedVertices();

	// Update item indices
	refreshIndices();

	// Update sector bounding boxes
	for (unsigned a = 0; a < sectors.size(); a++)
		sectors[a]->updateBBox();

	return true;
}

bool SLADEMap::addVertex(ParseTreeNode* def) {
	// Check for required properties
	ParseTreeNode* prop_x = (ParseTreeNode*)def->getChild("x");
	ParseTreeNode* prop_y = (ParseTreeNode*)def->getChild("y");
	if (!prop_x || !prop_y)
		return false;

	// Create new vertex
	MapVertex* nv = new MapVertex(prop_x->getFloatValue(), prop_y->getFloatValue());

	// Add extra vertex info
	ParseTreeNode* prop = NULL;
	for (unsigned a = 0; a < def->nChildren(); a++) {
		prop = (ParseTreeNode*)def->getChild(a);

		// Skip required properties
		if (prop == prop_x || prop == prop_y)
			continue;

		nv->prop(prop->getName()) = prop->getValue();
	}

	// Add vertex to map
	vertices.push_back(nv);

	return true;
}

bool SLADEMap::addSide(ParseTreeNode* def) {
	// Check for required properties
	ParseTreeNode* prop_sector = (ParseTreeNode*)def->getChild("sector");
	if (!prop_sector)
		return false;

	// Check sector index
	int sector = prop_sector->getIntValue();
	if (sector < 0 || sector >= (int)sectors.size())
		return false;

	// Create new side
	MapSide* ns = new MapSide(sectors[sector]);

	// Add extra side info
	ParseTreeNode* prop = NULL;
	for (unsigned a = 0; a < def->nChildren(); a++) {
		prop = (ParseTreeNode*)def->getChild(a);

		// Skip required properties
		if (prop == prop_sector)
			continue;

		ns->prop(prop->getName()) = prop->getValue();
	}

	// Add side to map
	sides.push_back(ns);

	return true;
}

bool SLADEMap::addLine(ParseTreeNode* def) {
	// Check for required properties
	ParseTreeNode* prop_v1 = (ParseTreeNode*)def->getChild("v1");
	ParseTreeNode* prop_v2 = (ParseTreeNode*)def->getChild("v2");
	ParseTreeNode* prop_s1 = (ParseTreeNode*)def->getChild("sidefront");
	if (!prop_v1 || !prop_v2 || !prop_s1)
		return false;

	// Check indices
	int v1 = prop_v1->getIntValue();
	int v2 = prop_v2->getIntValue();
	int s1 = prop_s1->getIntValue();
	if (v1 < 0 || v1 >= (int)vertices.size())
		return false;
	if (v2 < 0 || v2 >= (int)vertices.size())
		return false;
	if (s1 < 0 || s1 >= (int)sides.size())
		return false;

	// Get second side if any
	MapSide* side2 = NULL;
	ParseTreeNode* prop_s2 = (ParseTreeNode*)def->getChild("sideback");
	if (prop_s2) side2 = getSide(prop_s2->getIntValue());

	// Create new line
	MapLine* nl = new MapLine(vertices[v1], vertices[v2], sides[s1], side2);

	// Set default values
	// TODO: Nicer way to deal with default udmf values
	nl->prop("special") = 0;

	// Add extra line info
	ParseTreeNode* prop = NULL;
	for (unsigned a = 0; a < def->nChildren(); a++) {
		prop = (ParseTreeNode*)def->getChild(a);

		// Skip required properties
		if (prop == prop_v1 || prop == prop_v2 || prop == prop_s1 || prop == prop_s2)
			continue;

		nl->prop(prop->getName()) = prop->getValue();
	}

	// Add line to map
	lines.push_back(nl);

	return true;
}

bool SLADEMap::addSector(ParseTreeNode* def) {
	// Check for required properties
	ParseTreeNode* prop_ftex = (ParseTreeNode*)def->getChild("texturefloor");
	ParseTreeNode* prop_ctex = (ParseTreeNode*)def->getChild("textureceiling");
	if (!prop_ftex || !prop_ctex)
		return false;

	// Create new sector
	MapSector* ns = new MapSector(prop_ftex->getStringValue(), prop_ctex->getStringValue());

	// Add extra sector info
	ParseTreeNode* prop = NULL;
	for (unsigned a = 0; a < def->nChildren(); a++) {
		prop = (ParseTreeNode*)def->getChild(a);

		// Skip required properties
		if (prop == prop_ftex || prop == prop_ctex)
			continue;

		ns->prop(prop->getName()) = prop->getValue();
	}

	// Add sector to map
	sectors.push_back(ns);

	return true;
}

bool SLADEMap::addThing(ParseTreeNode* def) {
	// Check for required properties
	ParseTreeNode* prop_x = (ParseTreeNode*)def->getChild("x");
	ParseTreeNode* prop_y = (ParseTreeNode*)def->getChild("y");
	ParseTreeNode* prop_type = (ParseTreeNode*)def->getChild("type");
	if (!prop_x || !prop_y || !prop_type)
		return false;

	// Create new thing
	MapThing* nt = new MapThing(prop_x->getFloatValue(), prop_y->getFloatValue(), prop_type->getIntValue());

	// Add extra thing info
	ParseTreeNode* prop = NULL;
	for (unsigned a = 0; a < def->nChildren(); a++) {
		prop = (ParseTreeNode*)def->getChild(a);

		// Skip required properties
		if (prop == prop_x || prop == prop_y || prop == prop_type)
			continue;

		nt->prop(prop->getName()) = prop->getValue();
	}

	// Add thing to map
	things.push_back(nt);

	return true;
}

bool SLADEMap::readUDMFMap(Archive::mapdesc_t map) {
	// Get TEXTMAP entry (will always be after the 'head' entry)
	ArchiveEntry* textmap = map.head->nextEntry();

	// --- Parse UDMF text ---
	Parser parser;
	if (!parser.parseText(textmap->getMCData()))
		return false;

	// --- Process parsed data ---

	// First we have to sort the definition blocks by type so they can
	// be created in the correct order (verts->sides->lines->sectors->things),
	// even if they aren't defined in that order.
	// Unknown definitions are also kept, just in case
	ParseTreeNode* root = parser.parseTreeRoot();
	vector<ParseTreeNode*> defs_vertices;
	vector<ParseTreeNode*> defs_lines;
	vector<ParseTreeNode*> defs_sides;
	vector<ParseTreeNode*> defs_sectors;
	vector<ParseTreeNode*> defs_things;
	vector<ParseTreeNode*> defs_other;
	for (unsigned a = 0; a < root->nChildren(); a++) {
		ParseTreeNode* node = (ParseTreeNode*)root->getChild(a);

		// Vertex definition
		if (S_CMPNOCASE(node->getName(), "vertex"))
			defs_vertices.push_back(node);

		// Line definition
		else if (S_CMPNOCASE(node->getName(), "linedef"))
			defs_lines.push_back(node);

		// Side definition
		else if (S_CMPNOCASE(node->getName(), "sidedef"))
			defs_sides.push_back(node);

		// Sector definition
		else if (S_CMPNOCASE(node->getName(), "sector"))
			defs_sectors.push_back(node);

		// Thing definition
		else if (S_CMPNOCASE(node->getName(), "thing"))
			defs_things.push_back(node);

		// Namespace
		else if (S_CMPNOCASE(node->getName(), "namespace"))
			udmf_namespace = node->getStringValue();

		// Unknown
		else
			defs_other.push_back(node);
	}

	// Now create map structures from parsed data, in the right order

	// Create vertices from parsed data
	for (unsigned a = 0; a < defs_vertices.size(); a++)
		addVertex(defs_vertices[a]);

	// Create sectors from parsed data
	for (unsigned a = 0; a < defs_sectors.size(); a++)
		addSector(defs_sectors[a]);

	// Create sides from parsed data
	for (unsigned a = 0; a < defs_sides.size(); a++)
		addSide(defs_sides[a]);

	// Create lines from parsed data
	for (unsigned a = 0; a < defs_lines.size(); a++)
		addLine(defs_lines[a]);

	// Create things from parsed data
	for (unsigned a = 0; a < defs_things.size(); a++)
		addThing(defs_things[a]);

	// Update item indices
	refreshIndices();

	// Update sector bounding boxes
	for (unsigned a = 0; a < sectors.size(); a++)
		sectors[a]->updateBBox();

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

int SLADEMap::nearestVertex(double x, double y, double min) {
	// Go through vertices
	double min_dist = 999999999;
	MapVertex* v = NULL;
	double dist = 0;
	int index = -1;
	for (unsigned a = 0; a < vertices.size(); a++) {
		v = vertices[a];

		// Get 'quick' distance (no need to get real distance)
		if (v->x < x)	dist = x - v->x;
		else			dist = v->x - x;
		if (v->y < y)	dist += y - v->y;
		else			dist += v->y - y;

		// Check if it's nearer than the previous nearest
		if (dist < min_dist) {
			index = a;
			min_dist = dist;
		}
	}

	// Now determine the real distance to the closest vertex,
	// to check for minimum hilight distance
	if (index >= 0) {
		v = vertices[index];
		double rdist = MathStuff::distance(v->x, v->y, x, y);
		if (rdist > min)
			return -1;
	}

	return index;
}

int SLADEMap::nearestLine(double x, double y, double min) {
	// Go through lines
	double min_dist = 999999999;
	MapLine* l = NULL;
	double dist = 0;
	double x1, y1, x2, y2, len, u, lbound, ix, iy;
	int index = -1;
	for (unsigned a = 0; a < lines.size(); a++) {
		// Get line and some values
		l = lines[a];
		x1 = l->vertex1->x;
		y1 = l->vertex1->y;
		x2 = l->vertex2->x;
		y2 = l->vertex2->y;

		// Get line length
		len = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

		// Calculate intersection distance
		u = 0;
		if (len > 0) {
			u = ((x-x1)*(x2-x1) + (y-y1)*(y2-y1)) / (len*len);

			// Limit intersection distance to the line
			lbound = 1 / len;
			if(u < lbound) u = lbound;
			if(u > (1.0-lbound)) u = 1.0-lbound;
		}

		// Calculate intersection point
		ix = x1 + u*(x2 - x1);
		iy = y1 + u*(y2 - y1);

		// Calculate 'quick' distance to line
		dist = (ix-x)*(ix-x) + (iy-y)*(iy-y);

		// Check if it's nearer than the previous nearest
		if (dist < min_dist) {
			index = a;
			min_dist = dist;
		}
	}

	// Now determine the real distance to the closest line,
	// to check for minimum hilight distance
	if (index >= 0) {
		l = lines[index];
		double rdist = MathStuff::distanceToLine(x, y, l->vertex1->x, l->vertex1->y, l->vertex2->x, l->vertex2->y);
		if (rdist > min)
			return -1;
	}

	return index;
}

int SLADEMap::nearestThing(double x, double y, double min) {
	// Go through things
	double min_dist = 999999999;
	MapThing* t = NULL;
	double dist = 0;
	int index = -1;
	for (unsigned a = 0; a < things.size(); a++) {
		t = things[a];

		// Get 'quick' distance (no need to get real distance)
		if (t->x < x)	dist = x - t->x;
		else			dist = t->x - x;
		if (t->y < y)	dist += y - t->y;
		else			dist += t->y - y;

		// Check if it's nearer than the previous nearest
		if (dist < min_dist) {
			index = a;
			min_dist = dist;
		}
	}

	// Now determine the real distance to the closest thing,
	// to check for minimum hilight distance
	if (index >= 0) {
		t = things[index];
		double rdist = MathStuff::distance(t->x, t->y, x, y);
		if (rdist > min)
			return -1;
	}

	return index;
}

vector<int> SLADEMap::nearestThingMulti(double x, double y) {
	// Go through things
	vector<int> ret;
	double min_dist = 999999999;
	MapThing* t = NULL;
	double dist = 0;
	for (unsigned a = 0; a < things.size(); a++) {
		t = things[a];

		// Get 'quick' distance (no need to get real distance)
		if (t->x < x)	dist = x - t->x;
		else			dist = t->x - x;
		if (t->y < y)	dist += y - t->y;
		else			dist += t->y - y;

		// Check if it's nearer than the previous nearest
		if (dist < min_dist) {
			ret.clear();
			ret.push_back(a);
			min_dist = dist;
		}
		else if (dist == min_dist)
			ret.push_back(a);
	}

	return ret;
}

int SLADEMap::inSector(double x, double y) {
	// First, get nearest line
	int index = nearestLine(x, y, 999999999);

	// If there are no lines there can't be any (selectable) sectors
	if (index < 0)
		return -1;

	// Check what side of the line the point is on
	MapLine* l = lines[index];
	MapSide* s = NULL;
	if (MathStuff::lineSide(x, y, l->vertex1->x, l->vertex1->y, l->vertex2->x, l->vertex2->y) >= 0)
		s = l->side1;
	else
		s = l->side2;

	// If no side, not in sector
	if (!s)
		return -1;

	return sectorIndex(s->sector);
}

bbox_t SLADEMap::getMapBBox() {
	bbox_t bbox;

	// Return invalid bbox if no sectors
	if (sectors.size() == 0)
		return bbox;

	// Go through sectors
	// This is quicker than generating it from vertices,
	// but relies on sector bboxes being up-to-date (which they should be)
	bbox = sectors[0]->boundingBox();
	for (unsigned a = 1; a < sectors.size(); a++) {
		bbox_t sbb = sectors[a]->boundingBox();
		if (sbb.min.x < bbox.min.x)
			bbox.min.x = sbb.min.x;
		if (sbb.min.y < bbox.min.y)
			bbox.min.y = sbb.min.y;
		if (sbb.max.x > bbox.max.x)
			bbox.max.x = sbb.max.x;
		if (sbb.max.y > bbox.max.y)
			bbox.max.y = sbb.max.y;
	}

	return bbox;
}

bool SLADEMap::lineInSector(MapLine* line, MapSector* sector) {
	if (line->side1 && line->side1->sector == sector ||
		line->side2 && line->side2->sector == sector)
		return true;
	else
		return false;
}

bool SLADEMap::getLinesOfSector(unsigned index, vector<MapLine*>& list) {
	// Get sector
	MapSector* sector = getSector(index);
	if (!sector) return false;

	// Go through sides in sector
	MapSide* side = NULL;
	for (unsigned a = 0; a < sector->connected_sides.size(); a++) {
		side = sector->connected_sides[a];

		// Add the side's parent line to the list if it doesn't already exist
		if (std::find(list.begin(), list.end(), side->parent) == list.end())
			list.push_back(side->parent);
	}

	return true;
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
