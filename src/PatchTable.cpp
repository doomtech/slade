/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PatchTable.cpp
 * Description: Handles a collection of Patches and their corresponding
 *              archive entries (ie, encapsulates a PNAMES entry)
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
#include "PatchTable.h"
#include "ArchiveManager.h"


/*******************************************************************
 * PATCHTABLE CLASS FUNCTIONS
 *******************************************************************/

/* PatchTable::PatchTable
 * PatchTable class constructor
 *******************************************************************/
PatchTable::PatchTable(Archive* parent) {
	this->parent = parent;
}

/* PatchTable::~PatchTable
 * PatchTable class destructor
 *******************************************************************/
PatchTable::~PatchTable() {
}

/* PatchTable::patchName
 * Returns the name of the patch at [index], or an empty string if
 * [index] is out of bounds
 *******************************************************************/
string PatchTable::patchName(size_t index) {
	// Check index
	if (index >= patches.size())
		return wxEmptyString;

	// Return name at index
	return patches[index].name;
}

/* PatchTable::patchEntry
 * Returns the entry associated with the patch at [index], or NULL if
 * [index] is out of bounds
 *******************************************************************/
ArchiveEntry* PatchTable::patchEntry(size_t index) {
	// Check index
	if (index >= patches.size())
		return NULL;

	// Return entry at index
	return patches[index].entry;
}

/* PatchTable::patchEntry
 * Returns the entry associated with the patch matching [name], or
 * NULL if no match found
 *******************************************************************/
ArchiveEntry* PatchTable::patchEntry(string name) {
	// Search for patch by name
	for (size_t a = 0; a < patches.size(); a++) {
		if (!patches[a].name.CmpNoCase(name))
			return patches[a].entry;
	}

	// Not found
	return NULL;
}

/* PatchTable::patchIndex
 * Returns the index of the patch matching [name], or -1 if no
 * match found
 *******************************************************************/
int32_t PatchTable::patchIndex(string name) {
	// Search for patch by name
	for (size_t a = 0; a < patches.size(); a++) {
		if (!patches[a].name.CmpNoCase(name))
			return a;
	}

	// Not found
	return -1;
}

/* PatchTable::patchIndex
 * Returns the index of the patch associated with [entry], or NULL
 * if no match found
 *******************************************************************/
int32_t PatchTable::patchIndex(ArchiveEntry* entry) {
	// Search for patch by entry
	for (size_t a = 0; a < patches.size(); a++) {
		if (patches[a].entry == entry)
			return a;
	}

	// Not found
	return -1;
}

/* PatchTable::loadPNAMES
 * Loads a PNAMES entry, returns true on success, false otherwise
 *******************************************************************/
bool PatchTable::loadPNAMES(ArchiveEntry* pnames, Archive* parent) {
	// Check entry was given
	if (!pnames)
		return false;

	// Clear current table
	patches.clear();

	// Setup parent archive
	if (!parent)
		parent = pnames->getParent();

	// Read number of pnames
	uint32_t n_pnames = 0;
	pnames->seek(0, SEEK_SET);
	if (!pnames->read(&n_pnames, 4)) {
		wxLogMessage(_T("Error: PNAMES entry is corrupt"));
		return false;
	}

	// Read pnames content
	for (uint32_t a = 0; a < n_pnames; a++) {
		char pname[9] = "";
		pname[8] = 0;

		// Try to read pname
		if (!pnames->read(&pname, 8)) {
			wxLogMessage(_T("Error: PNAMES entry is corrupt"));
			return false;
		}

		// Create patch info
		patch_t patch;
		patch.name = wxString(pname).Upper();

		// Attempt to find patch entry
		ArchiveEntry* entry = NULL;
		if (parent)
			entry = parent->getEntry(patch.name);						// Search parent archive first
		if (!entry)
			entry = theArchiveManager->getResourceEntry(patch.name);	// Next search open resource archives + base resource archive

		if (entry)
			patch.entry = entry;
		else
			wxLogMessage(_T("Patch \"%s\" not found"), pname);

		// Add patch to list
		patches.push_back(patch);
	}

	// Update variables
	this->parent = parent;

	return true;
}
