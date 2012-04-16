
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
#include <wx/colour.h>

SLADEMap::SLADEMap() {
	// Init variables
	/*
	this->i_lines = false;
	this->i_sides = false;
	this->i_sectors = false;
	this->i_vertices = false;
	this->i_things = false;
	*/
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
	if (v->index >= 0)
		return v->index;

	// Find vertex
	for (unsigned a = 0; a < vertices.size(); a++) {
		if (vertices[a] == v) {
			v->index = a;
			return a;
		}
	}

	// Not found
	return -1;
}

int	SLADEMap::sideIndex(MapSide* s) {
	// Check side was given
	if (!s)
		return -1;

	// Check if item index is valid
	if (s->index >= 0)
		return s->index;

	// Find side
	for (unsigned a = 0; a < sides.size(); a++) {
		if (sides[a] == s) {
			s->index = a;
			return a;
		}
	}

	// Not found
	return -1;
}

int	SLADEMap::lineIndex(MapLine* l) {
	// Check line was given
	if (!l)
		return -1;

	// Check if item index is valid
	if (l->index >= 0)
		return l->index;

	// Find line
	for (unsigned a = 0; a < lines.size(); a++) {
		if (lines[a] == l) {
			l->index = a;
			return a;
		}
	}

	// Not found
	return -1;
}

int	SLADEMap::sectorIndex(MapSector* s) {
	// Check sector was given
	if (!s)
		return -1;

	// Check if item index is valid
	if (s->index >= 0)
		return s->index;

	// Find sector
	for (unsigned a = 0; a < sectors.size(); a++) {
		if (sectors[a] == s) {
			s->index = a;
			return a;
		}
	}

	// Not found
	return -1;
}

int	SLADEMap::thingIndex(MapThing* t) {
	// Check thing was given
	if (!t)
		return -1;

	// Check if item index is valid
	if (t->index >= 0)
		return t->index;

	// Find thing
	for (unsigned a = 0; a < things.size(); a++) {
		if (things[a] == t) {
			t->index = a;
			return a;
		}
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
	//i_vertices = true;
	for (unsigned a = 0; a < vertices.size(); a++)
		vertices[a]->index = a;

	// Side indices
	//i_sides = true;
	for (unsigned a = 0; a < sides.size(); a++)
		sides[a]->index = a;

	// Line indices
	//i_lines = true;
	for (unsigned a = 0; a < lines.size(); a++)
		lines[a]->index = a;

	// Sector indices
	//i_sectors = true;
	for (unsigned a = 0; a < sectors.size(); a++)
		sectors[a]->index = a;

	// Thing indices
	//i_things = true;
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
	nt->prop("height") = (double)t.z;
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

bool SLADEMap::writeDoomVertexes(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Init entry data
	entry->clearData();
	entry->resize(vertices.size() * 4, false);
	entry->seek(0, 0);

	// Write vertex data
	short x, y;
	for (unsigned a = 0; a < vertices.size(); a++) {
		x = vertices[a]->xPos();
		y = vertices[a]->yPos();
		entry->write(&x, 2);
		entry->write(&y, 2);
	}

	return true;
}

bool SLADEMap::writeDoomSidedefs(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Init entry data
	entry->clearData();
	entry->resize(sides.size() * 30, false);
	entry->seek(0, 0);

	// Write side data
	doomside_t side;
	string t_m, t_u, t_l;
	for (unsigned a = 0; a < sides.size(); a++) {
		memset(&side, 0, 30);

		// Offsets
		side.x_offset = sides[a]->intProperty("offsetx");
		side.y_offset = sides[a]->intProperty("offsety");

		// Sector
		side.sector = -1;
		if (sides[a]->sector) side.sector = sides[a]->sector->getIndex();
		
		// Textures
		t_m = sides[a]->stringProperty("texturemiddle");
		t_u = sides[a]->stringProperty("texturetop");
		t_l = sides[a]->stringProperty("texturebottom");
		memcpy(side.tex_middle, CHR(t_m), t_m.Length());
		memcpy(side.tex_upper, CHR(t_u), t_u.Length());
		memcpy(side.tex_lower, CHR(t_l), t_l.Length());

		entry->write(&side, 30);
	}

	return true;
}

bool SLADEMap::writeDoomLinedefs(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Init entry data
	entry->clearData();
	entry->resize(lines.size() * 14, false);
	entry->seek(0, 0);

	// Write line data
	doomline_t line;
	for (unsigned a = 0; a < lines.size(); a++) {
		// Vertices
		line.vertex1 = lines[a]->v1Index();
		line.vertex2 = lines[a]->v2Index();

		// Properties
		line.flags = lines[a]->intProperty("flags");
		line.type = lines[a]->intProperty("special");
		line.sector_tag = lines[a]->intProperty("arg0");

		// Sides
		line.side1 = -1;
		line.side2 = -1;
		if (lines[a]->side1) line.side1 = lines[a]->side1->getIndex();
		if (lines[a]->side2) line.side2 = lines[a]->side2->getIndex();

		entry->write(&line, 14);
	}

	return true;
}

bool SLADEMap::writeDoomSectors(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Init entry data
	entry->clearData();
	entry->resize(sectors.size() * 26, false);
	entry->seek(0, 0);

	// Write sector data
	doomsector_t sector;
	string t_f, t_c;
	for (unsigned a = 0; a < sectors.size(); a++) {
		memset(&sector, 0, 26);

		// Height
		sector.f_height = sectors[a]->intProperty("heightfloor");
		sector.c_height = sectors[a]->intProperty("heightceiling");

		// Textures
		t_f = sectors[a]->stringProperty("texturefloor");
		t_c = sectors[a]->stringProperty("textureceiling");
		memcpy(sector.f_tex, CHR(t_f), t_f.Length());
		memcpy(sector.c_tex, CHR(t_c), t_c.Length());

		// Properties
		sector.light = sectors[a]->intProperty("lightlevel");
		sector.special = sectors[a]->intProperty("special");
		sector.tag = sectors[a]->intProperty("id");

		entry->write(&sector, 26);
	}

	return true;
}

bool SLADEMap::writeDoomThings(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Init entry data
	entry->clearData();
	entry->resize(things.size() * 10, false);
	entry->seek(0, 0);

	// Write thing data
	doomthing_t thing;
	for (unsigned a = 0; a < things.size(); a++) {
		// Position
		thing.x = things[a]->xPos();
		thing.y = things[a]->yPos();

		// Properties
		thing.angle = things[a]->intProperty("angle");
		thing.type = things[a]->type;
		thing.flags = things[a]->intProperty("flags");

		entry->write(&thing, 10);
	}

	return true;
}

bool SLADEMap::writeDoomMap(vector<ArchiveEntry*>& map_entries) {
	// Init entry list
	map_entries.clear();

	// Write THINGS
	ArchiveEntry* entry = new ArchiveEntry("THINGS");
	writeDoomThings(entry);
	map_entries.push_back(entry);

	// Write LINEDEFS
	entry = new ArchiveEntry("LINEDEFS");
	writeDoomLinedefs(entry);
	map_entries.push_back(entry);

	// Write SIDEDEFS
	entry = new ArchiveEntry("SIDEDEFS");
	writeDoomSidedefs(entry);
	map_entries.push_back(entry);

	// Write VERTEXES
	entry = new ArchiveEntry("VERTEXES");
	writeDoomVertexes(entry);
	map_entries.push_back(entry);

	// Write SECTORS
	entry = new ArchiveEntry("SECTORS");
	writeDoomSectors(entry);
	map_entries.push_back(entry);

	return true;
}

bool SLADEMap::writeHexenLinedefs(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Init entry data
	entry->clearData();
	entry->resize(lines.size() * 16, false);
	entry->seek(0, 0);

	// Write line data
	hexenline_t line;
	for (unsigned a = 0; a < lines.size(); a++) {
		// Vertices
		line.vertex1 = lines[a]->v1Index();
		line.vertex2 = lines[a]->v2Index();

		// Properties
		line.flags = lines[a]->intProperty("flags");
		line.type = lines[a]->intProperty("special");

		// Args
		for (unsigned arg = 0; arg < 5; arg++)
			line.args[arg] = lines[a]->intProperty(S_FMT("arg%d", arg));

		// Sides
		line.side1 = -1;
		line.side2 = -1;
		if (lines[a]->side1) line.side1 = lines[a]->side1->getIndex();
		if (lines[a]->side2) line.side2 = lines[a]->side2->getIndex();

		entry->write(&line, 16);
	}

	return true;
}

bool SLADEMap::writeHexenThings(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Init entry data
	entry->clearData();
	entry->resize(things.size() * 20, false);
	entry->seek(0, 0);

	// Write thing data
	hexenthing_t thing;
	for (unsigned a = 0; a < things.size(); a++) {
		// Position
		thing.x = things[a]->xPos();
		thing.y = things[a]->yPos();
		thing.z = things[a]->intProperty("height");

		// Properties
		thing.angle = things[a]->intProperty("angle");
		thing.type = things[a]->type;
		thing.flags = things[a]->intProperty("flags");
		thing.tid = things[a]->intProperty("id");
		thing.special = things[a]->intProperty("special");

		// Args
		for (unsigned arg = 0; arg < 5; arg++)
			thing.args[arg] = things[a]->intProperty(S_FMT("arg%d", arg));

		entry->write(&thing, 20);
	}

	return true;
}

bool SLADEMap::writeHexenMap(vector<ArchiveEntry*>& map_entries) {
	// Init entry list
	map_entries.clear();

	// Write THINGS
	ArchiveEntry* entry = new ArchiveEntry("THINGS");
	writeHexenThings(entry);
	map_entries.push_back(entry);

	// Write LINEDEFS
	entry = new ArchiveEntry("LINEDEFS");
	writeHexenLinedefs(entry);
	map_entries.push_back(entry);

	// Write SIDEDEFS
	entry = new ArchiveEntry("SIDEDEFS");
	writeDoomSidedefs(entry);
	map_entries.push_back(entry);

	// Write VERTEXES
	entry = new ArchiveEntry("VERTEXES");
	writeDoomVertexes(entry);
	map_entries.push_back(entry);

	// Write SECTORS
	entry = new ArchiveEntry("SECTORS");
	writeDoomSectors(entry);
	map_entries.push_back(entry);

	return true;
}

bool SLADEMap::writeDoom64Map(vector<ArchiveEntry*>& map_entries) {
	return false;
}

bool SLADEMap::writeUDMFMap(ArchiveEntry* textmap) {
	return false;
}


void SLADEMap::clearMap() {
	// Clear sides
	for (unsigned a = 0; a < sides.size(); a++)
		delete sides[a];
	sides.clear();

	// Clear lines
	for (unsigned a = 0; a < lines.size(); a++)
		delete lines[a];
	lines.clear();

	// Clear vertices
	for (unsigned a = 0; a < vertices.size(); a++)
		delete vertices[a];
	vertices.clear();

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
	// Check vertex was given
	if (!vertex)
		return false;

	return removeVertex(vertexIndex(vertex));
}

bool SLADEMap::removeVertex(unsigned index) {
	// Check index
	if (index >= vertices.size())
		return false;

	// Remove all connected lines
	vector<MapLine*> clines = vertices[index]->connected_lines;
	for (unsigned a = 0; a < clines.size(); a++)
		removeLine(clines[a]);

	// Remove the vertex
	delete vertices[index];
	vertices[index] = vertices.back();
	vertices[index]->index = index;
	vertices.pop_back();

	geometry_updated = theApp->runTimer();

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
	if (index >= lines.size())
		return false;

	// Init
	lines[index]->resetInternals();
	MapVertex* v1 = lines[index]->vertex1;
	MapVertex* v2 = lines[index]->vertex2;

	// Remove the line's sides
	if (lines[index]->side1)
		removeSide(lines[index]->side1);
	if (lines[index]->side2)
		removeSide(lines[index]->side2);

	// Remove the line
	delete lines[index];
	lines[index] = lines[lines.size()-1];
	lines[index]->index = index;
	lines.pop_back();

	geometry_updated = theApp->runTimer();

	return true;
}

bool SLADEMap::removeSide(MapSide* side) {
	// Check side was given
	if (!side)
		return false;

	return removeSide(sideIndex(side));
}

bool SLADEMap::removeSide(unsigned index) {
	// Check index
	if (index >= sides.size())
		return false;

	// Remove from parent line
	MapLine* l = sides[index]->parent;
	if (l->side1 == sides[index])
		l->side1 = NULL;
	if (l->side2 == sides[index])
		l->side2 = NULL;

	// Remove the side
	delete sides[index];
	sides[index] = sides.back();
	sides[index]->index = index;
	sides.pop_back();

	return true;
}

bool SLADEMap::removeSector(MapSector* sector) {
	// Check sector was given
	if (!sector)
		return false;

	return removeSector(sectorIndex(sector));
}

bool SLADEMap::removeSector(unsigned index) {
	// Check index
	if (index >= sectors.size())
		return false;

	// Clear connected sides' sectors
	for (unsigned a = 0; a < sectors[index]->connected_sides.size(); a++)
		sectors[index]->connected_sides[a]->sector = NULL;

	// Remove the sector
	delete sectors[index];
	sectors[index] = sectors.back();
	sectors[index]->index = index;
	sectors.pop_back();

	return true;
}

bool SLADEMap::removeThing(MapThing* thing) {
	// Check thing was given
	if (!thing)
		return false;

	return removeThing(thingIndex(thing));
}

bool SLADEMap::removeThing(unsigned index) {
	// Check index
	if (index >= things.size())
		return false;

	// Remove the thing
	delete things[index];
	things[index] = things.back();
	things[index]->index = index;
	things.pop_back();

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

MapVertex* SLADEMap::vertexAt(double x, double y) {
	// Go through all vertices
	for (unsigned a = 0; a < vertices.size(); a++) {
		if (vertices[a]->x == x && vertices[a]->y == y)
			return vertices[a];
	}

	// No vertex at [x,y]
	return NULL;
}

// Sorting functions for SLADEMap::cutLines
bool sortVPosXAsc(const fpoint2_t& left, const fpoint2_t& right) {
	return left.x < right.x;
}
bool sortVPosXDesc(const fpoint2_t& left, const fpoint2_t& right) {
	return left.x > right.x;
}
bool sortVPosYAsc(const fpoint2_t& left, const fpoint2_t& right) {
	return left.y < right.y;
}
bool sortVPosYDesc(const fpoint2_t& left, const fpoint2_t& right) {
	return left.y > right.y;
}

vector<fpoint2_t> SLADEMap::cutLines(double x1, double y1, double x2, double y2) {
	// Init
	vector<fpoint2_t> intersect_points;
	double x, y;

	// Go through map lines
	for (unsigned a = 0; a < lines.size(); a++) {
		// Check for intersection
		if (MathStuff::linesIntersect(x1, y1, x2, y2, lines[a]->x1(), lines[a]->y1(), lines[a]->x2(), lines[a]->y2(), x, y)) {
			// Add intersection point to vector
			intersect_points.push_back(fpoint2_t(x, y));
		}
	}

	// Return if no intersections
	if (intersect_points.size() == 0)
		return intersect_points;

	// Check cutting line direction
	double xdif = x2 - x1;
	double ydif = y2 - y1;
	if ((xdif*xdif) > (ydif*ydif)) {
		// Sort points along x axis
		if (xdif >= 0)
			std::sort(intersect_points.begin(), intersect_points.end(), sortVPosXAsc);
		else
			std::sort(intersect_points.begin(), intersect_points.end(), sortVPosXDesc);
	}
	else {
		// Sort points along y axis
		if (ydif >= 0)
			std::sort(intersect_points.begin(), intersect_points.end(), sortVPosYAsc);
		else
			std::sort(intersect_points.begin(), intersect_points.end(), sortVPosYDesc);
	}

	return intersect_points;
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
	return getLinesOfSector(sector, list);
}

bool SLADEMap::getLinesOfSector(MapSector* sector, vector<MapLine*>& list) {
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

rgba_t SLADEMap::getSectorColour(MapSector* sector, int where) {
	// Check for UDMF+ZDoom namespace
	if (theGameConfiguration->getMapFormat() == MAP_UDMF && S_CMPNOCASE(theGameConfiguration->udmfNamespace(), "zdoom")) {
		// Get sector light colour
		int intcol = sector->intProperty("lightcolor");
		wxColour wxcol(intcol);

		// Get sector light level
		int light = sector->intProperty("lightlevel");

		// Get specific light level
		if (where == 1) {
			// Floor
			int fl = sector->intProperty("lightfloor");
			if (sector->boolProperty("lightfloorabsolute"))
				light = fl;
			else
				light += fl;
		}
		else if (where == 2) {
			// Ceiling
			int cl = sector->intProperty("lightceiling");
			if (sector->boolProperty("lightceilingabsolute"))
				light = cl;
			else
				light += cl;
		}

		// Calculate and return the colour
		float lightmult = (float)light / 255.0f;
		return rgba_t(wxcol.Blue() * lightmult, wxcol.Green() * lightmult, wxcol.Red() * lightmult, 255);
	}
	else {
		// Other format, simply return the light level
		int light = sector->intProperty("lightlevel");
		return rgba_t(light, light, light, 255);
	}
}

MapVertex* SLADEMap::createVertex(double x, double y, double split_dist) {
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

	// Check if this vertex splits any lines (if needed)
	if (split_dist >= 0) {
		unsigned nlines = lines.size();
		for (unsigned a = 0; a < nlines; a++) {
			// Skip line if it shares the vertex
			if (lines[a]->v1() == nv || lines[a]->v2() == nv)
				continue;

			if (fastDistanceToLine(x, y, a, split_dist) < split_dist) {
				wxLogMessage("Vertex at (%1.2f,%1.2f) splits line %d", x, y, a);
				splitLine(a, nv->index);
			}
		}
	}

	// Set geometry age
	geometry_updated = theApp->runTimer();

	return nv;
}

MapLine* SLADEMap::createLine(double x1, double y1, double x2, double y2, double split_dist) {
	// Round coordinates to integral if fractional positions are disabled
	if (!position_frac) {
		x1 = MathStuff::round(x1);
		y1 = MathStuff::round(y1);
		x2 = MathStuff::round(x2);
		y2 = MathStuff::round(y2);
	}

	//wxLogMessage("Create line (%1.2f,%1.2f) to (%1.2f,%1.2f)", x1, y1, x2, y2);

	// Get vertices at points
	MapVertex* vertex1 = vertexAt(x1, y1);
	MapVertex* vertex2 = vertexAt(x2, y2);

	// Create vertices if required
	if (!vertex1)
		vertex1 = createVertex(x1, y1, split_dist);
	if (!vertex2)
		vertex2 = createVertex(x2, y2, split_dist);

	// Create line between vertices
	return createLine(vertex1, vertex2);
}

MapLine* SLADEMap::createLine(MapVertex* vertex1, MapVertex* vertex2) {
	// Check both vertices were given
	if (!vertex1 || vertex1->parent_map != this)
		return NULL;
	if (!vertex2 || vertex2->parent_map != this)
		return NULL;

	// Check if there is already a line along the two given vertices
	for (unsigned a = 0; a < lines.size(); a++) {
		if ((lines[a]->vertex1 == vertex1 && lines[a]->vertex2 == vertex2) ||
			(lines[a]->vertex2 == vertex1 && lines[a]->vertex1 == vertex2))
			return lines[a];
	}

	// Create new line between vertices
	MapLine* nl = new MapLine(vertex1, vertex2, NULL, NULL, this);
	nl->index = lines.size();
	lines.push_back(nl);

	// Connect line to vertices
	vertex1->connectLine(nl);
	vertex2->connectLine(nl);

	// Set geometry age
	geometry_updated = theApp->runTimer();

	return nl;
}

MapThing* SLADEMap::createThing(double x, double y) {
	// Create the thing
	MapThing* nt = new MapThing(this);

	// Setup initial values
	nt->x = x;
	nt->y = y;
	nt->index = things.size();
	nt->type = 1;

	// Add to things
	things.push_back(nt);

	return nt;
}

MapSector* SLADEMap::createSector() {
	// Create the sector
	MapSector* ns = new MapSector(this);

	// Setup initial values
	ns->index = sectors.size();

	// Add to sectors
	sectors.push_back(ns);

	return ns;
}

void SLADEMap::moveVertex(unsigned vertex, double nx, double ny) {
	// Check index
	if (vertex >= vertices.size())
		return;

	// Move the vertex
	MapVertex* v = vertices[vertex];
	v->x = nx;
	v->y = ny;

	// Reset all attached lines' geometry info
	for (unsigned a = 0; a < v->connected_lines.size(); a++)
		v->connected_lines[a]->resetInternals();

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
	vertices[vertex2] = vertices.back();
	vertices[vertex2]->index = vertex2;
	vertices.pop_back();

	// Delete any resulting zero-length lines
	for (unsigned a = 0; a < zlines.size(); a++)
		removeLine(zlines[a]);

	geometry_updated = theApp->runTimer();
}

MapVertex* SLADEMap::mergeVerticesPoint(double x, double y) {
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

	// Return the final merged vertex
	return getVertex(merge);
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

	// Create and add new sides
	MapSide* s1 = NULL;
	MapSide* s2 = NULL;
	if (l->side1) {
		// Create side 1
		s1 = new MapSide(*l->side1);
		s1->setSector(l->side1->sector);
		if (s1->sector) {
			s1->sector->resetBBox();
			s1->sector->resetPolygon();
		}

		// Add side
		s1->index = sides.size();
		sides.push_back(s1);
	}
	if (l->side2) {
		// Create side 2
		s2 = new MapSide(*l->side2);
		s2->setSector(l->side2->sector);
		if (s2->sector) {
			s2->sector->resetBBox();
			s2->sector->resetPolygon();
		}

		// Add side
		s2->index = sides.size();
		sides.push_back(s2);
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

void SLADEMap::thingSetAnglePoint(unsigned thing, fpoint2_t point) {
	// Check index
	if (thing >= things.size())
		return;

	// Calculate direction vector
	MapThing* t = things[thing];
	fpoint2_t vec(point.x - t->x, point.y - t->y);
	double mag = sqrt((vec.x * vec.x) + (vec.y * vec.y));
	double x = vec.x / mag;
	double y = vec.y / mag;

	// Determine angle
	int angle = 0;
	if (x > 0.89)				// east
		angle = 0;
	else if (x < -0.89)			// west
		angle = 180;
	else if (y > 0.89)			// north
		angle = 90;
	else if (y < -0.89)			// south
		angle = 270;
	else if (x > 0 && y > 0)	// northeast
		angle = 45;
	else if (x < 0 && y > 0)	// northwest
		angle = 135;
	else if (x < 0 && y < 0)	// southwest
		angle = 225;
	else if (x > 0 && y < 0)	// southeast
		angle = 315;

	// Set thing angle
	t->setIntProperty("angle", angle);
}

void SLADEMap::splitLinesAt(MapVertex* vertex, double split_dist) {
	// Check if this vertex splits any lines (if needed)
	unsigned nlines = lines.size();
	for (unsigned a = 0; a < nlines; a++) {
		// Skip line if it shares the vertex
		if (lines[a]->v1() == vertex || lines[a]->v2() == vertex)
			continue;

		if (fastDistanceToLine(vertex->x, vertex->y, a, split_dist) < split_dist) {
			wxLogMessage("Vertex at (%1.2f,%1.2f) splits line %d", vertex->x, vertex->y, a);
			splitLine(a, vertexIndex(vertex));
		}
	}
}

void SLADEMap::setLineSector(unsigned line, unsigned sector, bool front) {
	// Check indices
	if (line >= lines.size() || sector >= sectors.size())
		return;

	// Get the MapSide to set
	MapSide* side = NULL;
	if (front)
		side = lines[line]->side1;
	else
		side = lines[line]->side2;

	// Create side if needed
	if (!side) {
		side = new MapSide(sectors[sector], this);
		side->index = sides.size();
		side->parent = lines[line];
		sides.push_back(side);

		if (front)
			lines[line]->side1 = side;
		else
			lines[line]->side2 = side;
	}
	else {
		// Set the side's sector
		side->setSector(sectors[sector]);
	}
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

	refreshIndices();

	return count;
}

int SLADEMap::removeZeroLengthLines() {
	int count = 0;
	for (unsigned a = 0; a < lines.size(); a++) {
		if (lines[a]->vertex1 == lines[a]->vertex2) {
			removeLine(a);
			a--;
			count++;
		}
	}

	return count;
}
