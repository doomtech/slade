
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Archive.cpp
 * Description: Archive, the 'base' archive class.
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
#include "Archive.h"
#include <wx/filename.h>


/* Archive::Archive
 * Archive class constructor
 *******************************************************************/
Archive::Archive() {
}

/* Archive::~Archive
 * Archive class destructor
 *******************************************************************/
Archive::~Archive() {
}

/* Archive::getFileName
 * Returns the archive's filename, including the path if specified
 *******************************************************************/
string Archive::getFileName(bool fullpath)
{
	if (fullpath)
		return filename;
	else
	{
		// Get the filename without the path
		wxFileName fn(filename);
		return fn.GetName() + _T(".") + fn.GetExt();
	}
}

/* Archive::getEntry
 * Returns the entry at the index specified,
 * or NULL if the index is invalid
 *******************************************************************/
ArchiveEntry* Archive::getEntry(DWORD index) {
	// Check index
	if (index >= entries.size())
		return NULL;

	return entries[index];
}

/* Archive::getEntry
 * Returns the first entry with the specified name,
 * or NULL if no lump exists with that name
 *******************************************************************/
ArchiveEntry* Archive::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a]->getName() == name)
			return entries[a];
	}

	return NULL;
}

/* Archive::openFile
 * Function to open a file, does nothing here, to be overridden by
 * any subclass of Archive
 *******************************************************************/
bool Archive::openFile(string filename) {
	return false;
}

/* Archive::openFile
 * Function to load in an entry's data, does nothing here,
 * to be overridden by any subclass of Archive
 *******************************************************************/
bool Archive::loadEntryData(ArchiveEntry* entry) {
	return false;
}

/* Archive::numLumps
 * Returns the number of entries in the archive
 *******************************************************************/
DWORD Archive::numEntries() {
	return entries.size();
}

/* Archive::openFile
 * Function to detect any maps in the archive, does nothing here,
 * to be overridden by any subclass of Archive
 *******************************************************************/
vector<Archive::mapdesc_t> Archive::detectMaps() {
	vector<mapdesc_t> maps;
	return maps;
}
