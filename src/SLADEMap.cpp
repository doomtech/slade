
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
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
#include "GameConfiguration.h"
#include "MainApp.h"
#include "Archive.h"
#include "WadArchive.h"

SLADEMap::SLADEMap() {
	// Init variables
	this->i_lines = false;
	this->i_sides = false;
	this->i_sectors = false;
	this->i_vertices = false;
	this->i_things = false;
	this->geometry_updated = 0;
	this->position_frac = false;
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

int SLADEMap::objectIndex(MapObject* o) {
	// Check object was given
	if (!o)
		return -1;

	// Get index depending on type
	switch (o->getObjType()) {
	case MOBJ_VERTEX: return vertexIndex((MapVertex*)o); break;
	case MOBJ_LINE: return lineIndex((MapLine*)o); break;
	case MOBJ_SIDE: return sideIndex((MapSide*)o); break;
	case MOBJ_SECTOR: return sectorIndex((MapSector*)o); break;
	case MOBJ_THING: return thingIndex((MapThing*)o); break;
	default: return o->index; break;
	}

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
	Archive::mapdesc_t omap = map;

	// Check for map archive
	Archive* tempwad = NULL;
	if (map.archive && map.head) {
		tempwad = new WadArchive();
		tempwad->open(map.head);
		vector<Archive::mapdesc_t> amaps = tempwad->detectMaps();
		if (amaps.size() > 0)
			omap = amaps[0];
		else
			return false;
	}

	bool ok = false;
	if (omap.format == MAP_DOOM)
		ok = readDoomMap(omap);
	else if (omap.format == MAP_HEXEN)
		ok = readHexenMap(omap);
	else if (omap.format == MAP_DOOM64)
		ok = readDoom64Map(omap);
	else if (omap.format == MAP_UDMF) {
		ok = readUDMFMap(omap);
	}

	if (tempwad)
		delete tempwad;

	return ok;
}

bool SLADEMap::addVertex(doomvertex_t& v) {
	MapVertex* nv = new MapVertex(v.x, v.y, this);
	vertices.push_back(nv);
	return true;
}

bool SLADEMap::addVertex(doom64vertex_t& v) {
	MapVertex* nv = new MapVertex((double)v.x/65536, (double)v.y/65536, this);
	vertices.push_back(nv);
	return true;
}

bool SLADEMap::addSide(doomside_t& s) {
	// Create side
	MapSide* ns = new MapSide(getSector(s.sector), this);

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
	MapSide* ns = new MapSide(getSector(s.sector), this);

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
	MapSide* s1 = getSide(l.side1);
	MapSide* s2 = getSide(l.side2);

	// Get relevant vertices
	MapVertex* v1 = getVertex(l.vertex1);
	MapVertex* v2 = getVertex(l.vertex2);

	// Check everything is valid
	if (!v1 || !v2)
		return false;

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
	MapLine* nl = new MapLine(v1, v2, s1, s2, this);

	// Setup line properties
	nl->prop("arg0") = l.sector_tag;
	nl->prop("id") = l.sector_tag;
	nl->prop("special") = l.type;
	nl->prop("flags") = l.flags;

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

	// Get relevant vertices
	MapVertex* v1 = getVertex(l.vertex1);
	MapVertex* v2 = getVertex(l.vertex2);

	// Check everything is valid
	if (!v1 || !v2)
		return false;

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
	MapLine* nl = new MapLine(v1, v2, s1, s2, this);

	// Setup line properties
	nl->prop("arg0") = l.sector_tag;
	if (l.type & 0x100)
		nl->prop("macro") = l.type & 0xFF;
	else
		nl->prop("special") = l.type & 0xFF;
	nl->prop("flags") = (int)l.flags;
	nl->prop("extraflags") = l.type >> 9;

	// Add line
	lines.push_back(nl);
	return true;
}

bool SLADEMap::addSector(doomsector_t& s) {
	// Create sector
	MapSector* ns = new MapSector(wxString::FromAscii(s.f_tex, 8), wxString::FromAscii(s.c_tex, 8), this);

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
		theResourceManager->getTextureName(s.c_tex), this);

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
	MapThing* nt = new MapThing(t.x, t.y, t.type, this);

	// Setup thing properties
	nt->prop("angle") = t.angle;
	nt->prop("flags") = t.flags;

	// Add thing
	things.push_back(nt);
	return true;
}

bool SLADEMap::addThing(doom64thing_t& t) {
	// Create thing
	MapThing* nt = new MapThing(t.x, t.y, t.type, this);

	// Setup thing properties
	nt->prop("angle") = t.angle;
	nt->prop("z") = (double)t.z;
	nt->prop("flags") = t.flags;
	nt->prop("id") = t.tid;

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

	// Check for empty entry
	if (entry->getSize() < sizeof(doomvertex_t)) {
		wxLogMessage("Read 0 vertices");
		return true;
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doomside_t)) {
		wxLogMessage("Read 0 sides");
		return true;
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doomline_t)) {
		wxLogMessage("Read 0 lines");
		return true;
	}

	doomline_t* line_data = (doomline_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(doomline_t); a++) {
		if (!addLine(line_data[a]))
			wxLogMessage("Line %d invalid, not added", a);
	}

	wxLogMessage("Read %d lines", lines.size());

	return true;
}

bool SLADEMap::readDoomSectors(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no SECTORS entry!";
		wxLogMessage(Global::error);
		return false;
	}

	// Check for empty entry
	if (entry->getSize() < sizeof(doomsector_t)) {
		wxLogMessage("Read 0 sectors");
		return true;
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doomthing_t)) {
		wxLogMessage("Read 0 things");
		return true;
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
		if (!v && entry->getName() == "VERTEXES")
			v = entry;
		else if (!si && entry->getName() == "SIDEDEFS")
			si = entry;
		else if (!l && entry->getName() == "LINEDEFS")
			l = entry;
		else if (!se && entry->getName() == "SECTORS")
			se = entry;
		else if (!t && entry->getName() == "THINGS")
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

	// Update variables
	geometry_updated = theApp->runTimer();

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

	// Get relevant vertices
	MapVertex* v1 = getVertex(l.vertex1);
	MapVertex* v2 = getVertex(l.vertex2);

	// Check everything is valid
	if (!v1 || !v2)
		return false;

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
	MapLine* nl = new MapLine(v1, v2, s1, s2, this);

	// Setup line properties
	nl->prop("arg0") = l.args[0];
	nl->prop("arg1") = l.args[1];
	nl->prop("arg2") = l.args[2];
	nl->prop("arg3") = l.args[3];
	nl->prop("arg4") = l.args[4];
	nl->prop("special") = l.type;
	nl->prop("flags") = l.flags;

	// Handle some special cases
	if (l.type) {
		int needs_tag = theGameConfiguration->actionSpecial(l.type)->needsTag();
		if (needs_tag == AS_TT_LINEID || needs_tag == AS_TT_1LINEID_2LINE) {
			nl->prop("id") = l.args[0];
		} else if (needs_tag == AS_TT_LINEID_HI5) {
			nl->prop("id") = (l.args[0] + (l.args[4]<<8));
		}
	}

	// Add line
	lines.push_back(nl);
	return true;
}

bool SLADEMap::addThing(hexenthing_t& t) {
	// Create thing
	MapThing* nt = new MapThing(t.x, t.y, t.type, this);

	// Setup thing properties
	nt->prop("angle") = t.angle;
	nt->prop("height") = (double)t.z;
	nt->prop("special") = t.special;
	nt->prop("flags") = t.flags;
	nt->prop("id") = t.tid;
	nt->prop("arg0") = t.args[0];
	nt->prop("arg1") = t.args[1];
	nt->prop("arg2") = t.args[2];
	nt->prop("arg3") = t.args[3];
	nt->prop("arg4") = t.args[4];

	// Add thing
	things.push_back(nt);
	return true;
}

bool SLADEMap::readHexenLinedefs(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no LINEDEFS entry!";
		return false;
	}

	// Check for empty entry
	if (entry->getSize() < sizeof(hexenline_t)) {
		wxLogMessage("Read 0 lines");
		return true;
	}

	hexenline_t* line_data = (hexenline_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(hexenline_t); a++)
		addLine(line_data[a]);

	wxLogMessage("Read %d lines", lines.size());

	return true;
}

bool SLADEMap::readHexenThings(ArchiveEntry * entry) {
	if (!entry) {
		Global::error = "Map has no THINGS entry!";
		return false;
	}

	// Check for empty entry
	if (entry->getSize() < sizeof(hexenthing_t)) {
		wxLogMessage("Read 0 things");
		return true;
	}

	hexenthing_t* thng_data = (hexenthing_t*)entry->getData(true);
	for (size_t a = 0; a < entry->getSize() / sizeof(hexenthing_t); a++)
		addThing(thng_data[a]);

	wxLogMessage("Read %d things", things.size());

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
		if (!v && entry->getName() == "VERTEXES")
			v = entry;
		else if (!si && entry->getName() == "SIDEDEFS")
			si = entry;
		else if (!l && entry->getName() == "LINEDEFS")
			l = entry;
		else if (!se && entry->getName() == "SECTORS")
			se = entry;
		else if (!t && entry->getName() == "THINGS")
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doom64vertex_t)) {
		wxLogMessage("Read 0 vertices");
		return true;
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doom64side_t)) {
		wxLogMessage("Read 0 sides");
		return true;
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doom64line_t)) {
		wxLogMessage("Read 0 lines");
		return true;
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doom64sector_t)) {
		wxLogMessage("Read 0 sectors");
		return true;
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

	// Check for empty entry
	if (entry->getSize() < sizeof(doom64thing_t)) {
		wxLogMessage("Read 0 things");
		return true;
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
		if (!v && entry->getName() == "VERTEXES")
			v = entry;
		else if (!si && entry->getName() == "SIDEDEFS")
			si = entry;
		else if (!l && entry->getName() == "LINEDEFS")
			l = entry;
		else if (!se && entry->getName() == "SECTORS")
			se = entry;
		else if (!t && entry->getName() == "THINGS")
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
	MapVertex* nv = new MapVertex(prop_x->getFloatValue(), prop_y->getFloatValue(), this);

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
	MapSide* ns = new MapSide(sectors[sector], this);

	// Set some reasonable defaults
	ns->prop("texturetop").setValue(string("-"));
	ns->prop("texturemiddle").setValue(string("-"));
	ns->prop("texturebottom").setValue(string("-"));
	ns->prop("offsetx") = 0;
	ns->prop("offsety") = 0;

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
	MapLine* nl = new MapLine(vertices[v1], vertices[v2], sides[s1], side2, this);

	// Set some reasonable defaults
	nl->prop("special") = 0;
	nl->prop("arg0") = 0;
	nl->prop("arg1") = 0;
	nl->prop("arg2") = 0;
	nl->prop("arg3") = 0;
	nl->prop("arg4") = 0;

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
	MapSector* ns = new MapSector(prop_ftex->getStringValue(), prop_ctex->getStringValue(), this);

	// Set some reasonable defaults
	ns->prop("id") = 0;

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
	MapThing* nt = new MapThing(prop_x->getFloatValue(), prop_y->getFloatValue(), prop_type->getIntValue(), this);

	// Set some reasonable defaults
	nt->prop("height") = 0;
	nt->prop("angle") = 0;
	nt->prop("id") = 0;
	nt->prop("special") = 0;
	nt->prop("arg0") = 0;
	nt->prop("arg1") = 0;
	nt->prop("arg2") = 0;
	nt->prop("arg3") = 0;
	nt->prop("arg4") = 0;

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

	// Remove detached vertices
	removeDetachedVertices();

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

bool SLADEMap::removeVertex(MapVertex* vertex) {
	// Check line was given
	if (!vertex)
		return false;

	// Get line index
	int index = vertexIndex(vertex);

	if (index >= 0) {
		// Remove all connected lines
		for (unsigned a = 0; a < vertex->connected_lines.size(); a++)
			removeLine(vertex->connectedLine(a));

		// Remove the vertex
		delete vertex;
		vertices.erase(vertices.begin() + index);

		// Vertex indices are now invalid
		i_vertices = false;

		return true;
	}
	else
		return false;
}

bool SLADEMap::removeVertex(unsigned index) {
	// Check index
	if (index > vertices.size())
		return false;

	// Remove all connected lines
	for (unsigned a = 0; a < vertices[index]->connected_lines.size(); a++)
		removeLine(vertices[index]->connectedLine(a));

	// Remove the vertex
	delete vertices[index];
	vertices.erase(vertices.begin() + index);

	// Vertex indices are now invalid
	i_vertices = false;

	return true;
}

bool SLADEMap::removeLine(MapLine* line) {
	// Check line was given
	if (!line)
		return false;

	return removeLine(lineIndex(line));
}

bool SLADEMap::removeLine(unsigned index) {
	// Check index
	if (index > lines.size())
		return false;

	// Detach line from its vertices
	lines[index]->vertex1->disconnectLine(lines[index]);
	lines[index]->vertex2->disconnectLine(lines[index]);

	// Delete the line's sides
	if (lines[index]->side1) {
		sides.erase(sides.begin() + sideIndex(lines[index]->side1));
		delete lines[index]->side1;
		i_sides = false;
	}
	if (lines[index]->side2) {
		sides.erase(sides.begin() + sideIndex(lines[index]->side2));
		delete lines[index]->side2;
		i_sides = false;
	}

	// Remove the line
	delete lines[index];
	lines.erase(lines.begin() + index);

	// Line indices are now invalid
	i_lines = false;

	return true;
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

double SLADEMap::fastDistanceToLine(double x, double y, unsigned line, double mindist) {
	MapLine* l = lines[line];

	// Check with line bounding box first (since we have a minimum distance)
	if (x < min(l->vertex1->x, l->vertex2->x) - mindist || x > max(l->vertex1->x, l->vertex2->x) + mindist ||
		y < min(l->vertex1->y, l->vertex2->y) - mindist || y > max(l->vertex1->y, l->vertex2->y) + mindist)
		return mindist+1;

	// Calculate intersection point
	double len = l->getLength();
	double mx, ix, iy;
	mx = (-l->vertex1->x+x)*l->ca + (-l->vertex1->y+y)*l->sa;
	if (mx <= 0)		mx = 0.00001;		// Clip intersection to line (but not exactly on endpoints)
	else if (mx >= len)	mx = len - 0.00001;	// ^^
	ix = l->vertex1->x + mx*l->ca;
	iy = l->vertex1->y + mx*l->sa;

	// Calculate distance to line
	return sqrt((ix-x)*(ix-x) + (iy-y)*(iy-y));
}

int SLADEMap::nearestLine(double x, double y, double mindist) {
	// Go through lines
	double min_dist = mindist;
	double dist = 0;
	int index = -1;
	for (unsigned a = 0; a < lines.size(); a++) {
		// Calculate distance to line (fast)
		dist = fastDistanceToLine(x, y, a, min_dist);

		// Check if it's nearer than the previous nearest
		if (dist < min_dist && dist < mindist) {
			index = a;
			min_dist = dist;
		}
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
	// Find nearest line
	int nline = nearestLine(x, y, 999999);
	if (nline < 0) return -1;

	// Check what side of the line the point is on
	MapLine* line = lines[nline];
	double side = MathStuff::lineSide(x, y, line->x1(), line->y1(), line->x2(), line->y2());

	// Return the sector on that side
	if (side >= 0)
		return sectorIndex(line->frontSector());
	else
		return sectorIndex(line->backSector());
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

bool SLADEMap::getVerticesOfSector(unsigned index, vector<MapVertex*>& list) {
	// Get sector
	MapSector* sector = getSector(index);
	if (!sector) return false;

	// Go through sides in sector
	MapSide* side = NULL;
	MapLine* line = NULL;
	for (unsigned a = 0; a < sector->connected_sides.size(); a++) {
		side = sector->connected_sides[a];
		line = side->getParentLine();

		// Add the side's parent line's vertices to the list if they doesn't already exist
		if (line->v1() && std::find(list.begin(), list.end(), line->v1()) == list.end())
			list.push_back(line->v1());
		if (line->v2() && std::find(list.begin(), list.end(), line->v2()) == list.end())
			list.push_back(line->v2());
	}

	return true;
}

void SLADEMap::getSectorsByTag(int tag, vector<MapSector*>& list) {
	// Find sectors with matching tag
	for (unsigned a = 0; a < sectors.size(); a++) {
		if (sectors[a]->prop("id").getIntValue() == tag)
			list.push_back(sectors[a]);
	}
}

void SLADEMap::getThingsById(int id, vector<MapThing*>& list) {
	// Find things with matching id
	for (unsigned a = 0; a < things.size(); a++) {
		if (things[a]->prop("id").getIntValue() == id)
			list.push_back(things[a]);
	}
}

void SLADEMap::getThingsByIdInSectorTag(int id, int tag, vector<MapThing*>& list) {
	// Find things with matching id contained in sector with matching tag
	for (unsigned a = 0; a < things.size(); a++) {
		if (things[a]->prop("id").getIntValue() == id) {
			int si = inSector(things[a]->xPos(), things[a]->yPos());
			if (si > -1 && (unsigned)si < sectors.size() && sectors[si]->prop("id").getIntValue() == tag) {
				list.push_back(things[a]);
			}
		}
	}
}

void SLADEMap::getLinesById(int id, vector<MapLine*>& list) {
	// Find lines with matching id
	for (unsigned a = 0; a < lines.size(); a++) {
		if (lines[a]->prop("id").getIntValue() == id)
			list.push_back(lines[a]);
	}
}

MapVertex* SLADEMap::createVertex(double x, double y) {
	// Round position to integral if fractional positions are disabled
	if (!position_frac) {
		x = MathStuff::round(x);
		y = MathStuff::round(y);
	}

	// First check that it won't overlap any other vertex
	for (unsigned a = 0; a < vertices.size(); a++) {
		if (vertices[a]->x == x && vertices[a]->y == y)
			return vertices[a];
	}

	// Create the vertex
	MapVertex* nv = new MapVertex(x, y, this);
	nv->index = vertices.size();
	vertices.push_back(nv);

	return nv;
}

void SLADEMap::moveVertex(unsigned vertex, double nx, double ny) {
	// Check index
	if (vertex >= vertices.size())
		return;

	// Move the vertex
	MapVertex* v = vertices[vertex];
	v->x = nx;
	v->y = ny;

	// Go through attached lines
	for (unsigned a = 0; a < v->connected_lines.size(); a++) {
		MapLine* line = v->connected_lines[a];
		MapSector* s1 = line->frontSector();
		MapSector* s2 = line->backSector();

		// Reset line internals
		line->length = -1;

		// Reset front sector internals
		if (s1) {
			s1->resetPolygon();
			s1->resetBBox();
		}

		// Reset back sector internals
		if (s2) {
			s2->resetPolygon();
			s2->resetBBox();
		}
	}

	geometry_updated = theApp->runTimer();
}

void SLADEMap::mergeVertices(unsigned vertex1, unsigned vertex2) {
	// Check indices
	if (vertex1 >= vertices.size() || vertex2 >= vertices.size() || vertex1 == vertex2)
		return;

	// Go through lines of second vertex
	MapVertex* v1 = vertices[vertex1];
	MapVertex* v2 = vertices[vertex2];
	vector<MapLine*> zlines;
	for (unsigned a = 0; a < v2->connected_lines.size(); a++) {
		MapLine* line = v2->connected_lines[a];

		// Change first vertex if needed
		if (line->vertex1 == v2) {
			line->vertex1 = v1;
			line->length = -1;
			v1->connectLine(line);
		}

		// Change second vertex if needed
		if (line->vertex2 == v2) {
			line->vertex2 = v1;
			line->length = -1;
			v1->connectLine(line);
		}

		if (lines[a]->vertex1 == v1 && lines[a]->vertex2 == v1)
			zlines.push_back(lines[a]);
	}

	// Delete the vertex
	delete v2;
	vertices.erase(vertices.begin() + vertex2);
	i_vertices = false;

	// Delete any resulting zero-length lines
	for (unsigned a = 0; a < zlines.size(); a++)
		removeLine(zlines[a]);

	geometry_updated = theApp->runTimer();
}

void SLADEMap::mergeVerticesPoint(double x, double y) {
	// Go through all vertices
	int merge = -1;
	for (unsigned a = 0; a < vertices.size(); a++) {
		// Skip if vertex isn't on the point
		if (vertices[a]->x != x || vertices[a]->y != y)
			continue;

		// Set as the merge target vertex if we don't have one already
		if (merge < 0) {
			merge = a;
			continue;
		}

		// Otherwise, merge this vertex with the merge target
		mergeVertices(merge, a);
		a--;
	}

	geometry_updated = theApp->runTimer();
}

void SLADEMap::splitLine(unsigned line, unsigned vertex) {
	// Check indices
	if (line >= lines.size() || vertex >= vertices.size())
		return;

	// Get objects
	MapLine* l = lines[line];
	MapVertex* v = vertices[vertex];

	// Shorten line
	MapVertex* v2 = l->vertex2;
	v2->disconnectLine(l);
	l->vertex2 = v;
	v->connectLine(l);
	l->length = -1;

	// Create new sides
	MapSide* s1 = NULL;
	MapSide* s2 = NULL;
	if (l->side1) {
		s1 = new MapSide(*l->side1);
		s1->setSector(l->side1->sector);
		if (s1->sector) {
			s1->sector->resetBBox();
			s1->sector->resetPolygon();
		}
	}
	if (l->side2) {
		s2 = new MapSide(*l->side2);
		s2->setSector(l->side2->sector);
		if (s2->sector) {
			s2->sector->resetBBox();
			s2->sector->resetPolygon();
		}
	}

	// Create and add new line
	MapLine* nl = new MapLine(v, v2, s1, s2, this);
	nl->copy(l);
	nl->index = lines.size();
	lines.push_back(nl);

	geometry_updated = theApp->runTimer();
}

void SLADEMap::moveThing(unsigned thing, double nx, double ny) {
	// Check index
	if (thing >= things.size())
		return;

	// Move the thing
	MapThing* t = things[thing];
	t->x = nx;
	t->y = ny;
}

int SLADEMap::removeDetachedVertices() {
	int count = 0;

	for (int a = vertices.size() - 1; a >= 0; a--) {
		if (vertices[a]->nConnectedLines() == 0) {
			delete vertices[a];
			vertices.erase(vertices.begin() + a);
			count++;
		}
	}

	/*
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
	*/

	return count;
}
