
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
	if (format == 0)
		return readDoomMap(map_entries);
	else if (format == 1)
		return readHexenMap(map_entries);
	else if (format == 2) {
		ArchiveEntry* entry = map_entries->getEntry(_T("TEXTMAP"));
		return readUDMFMap(entry);
	}
}

bool SLADEMap::readDoomMap(Archive* map_entries) {

	// ---- Read vertices ----
	ArchiveEntry* entry = map_entries->getEntry(_T("VERTEXES"));
	if (!entry) {
		Global::error = _T("Map has no VERTEXES entry!");
		return false;
	}

	doomvertex_t* vert_data = (doomvertex_t*)entry->getData(true);
	for (int a = 0; a < entry->getSize() / 4; a++) {
		MapVertex* nv = new MapVertex(vert_data[a]);
		vertices.push_back(nv);
	}

	return true;
}

bool SLADEMap::readHexenMap(Archive* map_entries) {

	// ---- Read vertices ----
	ArchiveEntry* entry = map_entries->getEntry(_T("VERTEXES"));
	if (!entry) {
		Global::error = _T("Map has no VERTEXES entry!");
		return false;
	}

	doomvertex_t* vert_data = (doomvertex_t*)entry->getData(true);
	for (int a = 0; a < entry->getSize() / 4; a++) {
		MapVertex* nv = new MapVertex(vert_data[a]);
		vertices.push_back(nv);
	}

	return true;
}

bool SLADEMap::readUDMFMap(ArchiveEntry* map_data) {
	Tokenizer udmf_data;
	udmf_data.openMem((char*)map_data->getData(true), map_data->getSize());

	string token = udmf_data.getToken();
	while (token != _T("")) {
		// Vertex definition
		if (token == _T("vertex")) {
			MapVertex* new_vertex = new MapVertex();
			if (new_vertex->parseUDMF(udmf_data))
				vertices.push_back(new_vertex);
			else
				delete new_vertex;
		}

		// Line definition
		if (token == _T("linedef")) {
			MapLine* new_line = new MapLine();
			if (new_line->parseUDMF(udmf_data))
				lines.push_back(new_line);
			else
				delete new_line;
		}

		// Side definition
		if (token == _T("sidedef")) {
			MapSide* new_side = new MapSide();
			if (new_side->parseUDMF(udmf_data))
				sides.push_back(new_side);
			else
				delete new_side;
		}

		// Sector definition
		if (token == _T("sector")) {
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
