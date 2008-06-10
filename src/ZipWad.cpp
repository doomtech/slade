
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ZipWad.cpp
 * Description: ZipWad file handling class functions
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
#include "Wad.h"
#include "Lump.h"
#include <wx/log.h>


/* ZipWad::ZipWad
 * ZipWad class constructor
 *******************************************************************/
ZipWad::ZipWad()
{
	filename = _T("");
}

/* ZipWad::~ZipWad
 * ZipWad class destructor
 *******************************************************************/
ZipWad::~ZipWad()
{
}

/* ZipWad::openFile
 * Reads a zip/pk3 format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool ZipWad::openFile(string filename, string &error)
{
	// Not yet implemented
	error = _T("Not Implemented");
	return false;
}

/* ZipWad::loadLump
 * Loads a lump's data from the wadfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool ZipWad::loadLump(Lump* lump)
{
	return false;
}

/* ZipWad::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
vector<Wad::mapdesc_t> ZipWad::detectMaps()
{
	vector<mapdesc_t> maps;
	return maps;
}
