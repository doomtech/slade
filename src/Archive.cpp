
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Archive.cpp
 * Description: Archive, the 'base' archive class (Abstract)
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
	type = 0;
	modified = true;
	on_disk = false;
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
	else {
		// Get the filename without the path
		wxFileName fn(filename);
		return fn.GetFullName();
	}
}

/* Archive::checkEntry
 * Checks that the given entry is valid and part of this archive
 *******************************************************************/
bool Archive::checkEntry(ArchiveEntry* entry) {
	// Check entry is valid
	if (!entry)
		return false;

	// Check entry is part of this archive
	if (entry->getParent() != this)
		return false;

	// Entry is ok
	return true;
}

/* Archive::entryModified
 * Called when an entry in the archive is modified
 *******************************************************************/
void Archive::entryModified(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return;

	// Set the archive state to modified
	modified = true;

	// Get the entry index and announce the change
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(entry);
	DWORD index = entryIndex(entry);
	mc.write(&index, sizeof(DWORD));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("entry_modified"), mc);
}
