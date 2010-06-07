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
	patch_invalid.name = "INVALID_PATCH";
}

/* PatchTable::~PatchTable
 * PatchTable class destructor
 *******************************************************************/
PatchTable::~PatchTable() {
}

/* PatchTable::patch
 * Returns the patch at [index], or an 'invalid' patch if [index] is
 * out of bounds
 *******************************************************************/
patch_t& PatchTable::patch(size_t index) {
	// Check index
	if (index >= patches.size())
		return patch_invalid;

	// Return patch at index
	return patches[index];
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

	// Update patch entry if needed
	if (!patches[index].entry)
		updatePatchEntry(index);

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
		if (!patches[a].name.CmpNoCase(name)) {
			// Update patch entry if needed
			if (!patches[a].entry)
				updatePatchEntry(a);

			return patches[a].entry;
		}
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

/* PatchTable::removePatch
 * Removes the patch at [index]. Returns false if [index] is out of
 * range, true otherwise
 *******************************************************************/
bool PatchTable::removePatch(unsigned index) {
	// Check index
	if (index >= patches.size())
		return false;

	// Remove the patch
	patches.erase(patches.begin() + index);

	return true;
}

/* PatchTable::replacePatch
 * Replaces the patch info at [index] with a new name (newname).
 * Also attempts to find the ArchiveEntry matching [newname] in
 * [parent] and resource archives. Returns false if [index] is out
 * of range or no matching entry was found, true otherwise
 *******************************************************************/
bool PatchTable::replacePatch(unsigned index, string newname) {
	// Check index
	if (index >= patches.size())
		return false;

	// Change the patch name
	patches[index].name = newname;

	// Update patch entry
	updatePatchEntry(index);

	return !!patch(index).entry;
}

/* PatchTable::addPatch
 * Adds a new patch with [name] to the end of the list. Also attempts
 * to find the ArchiveEntry matching [name] in [parent] and resource
 * archives. Returns false if no matching entry was found,
 * true otherwise
 *******************************************************************/
bool PatchTable::addPatch(string name) {
	// Create/init new patch
	patch_t patch;
	patch.name = name;

	// Add the patch
	patches.push_back(patch);

	return !!patch.entry;
}

void PatchTable::updatePatchEntry(unsigned index) {
	// Check index
	if (index >= patches.size())
		return;

	// Get patch name
	string name = patch(index).name;

	// Attempt to find patch entry
	ArchiveEntry* entry = NULL;
	Archive::search_options_t options;
	options.match_name = name;
	options.match_namespace = "patches";
	if (parent)
		entry = parent->findFirst(options);								// Search parent archive first
	if (!entry)
		entry = theArchiveManager->findResourceEntry(options, parent);	// Next search open resource archives + base resource archive

	// Set patch entry
	patch(index).entry = entry;
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
		wxLogMessage("Error: PNAMES entry is corrupt");
		return false;
	}

	// Read pnames content
	for (uint32_t a = 0; a < n_pnames; a++) {
		char pname[9] = "";
		pname[8] = 0;

		// Try to read pname
		if (!pnames->read(&pname, 8)) {
			wxLogMessage("Error: PNAMES entry is corrupt");
			return false;
		}

		// Add new patch
		bool success = addPatch(wxString(pname).Upper());

		// Write log message if patch entry not found
		//if (!success)
		//	wxLogMessage("Patch \"%s\" not found", pname);
	}

	// Update variables
	this->parent = parent;

	return true;
}

void PatchTable::clearPatchUsage() {
	for (size_t a = 0; a < patches.size(); a++)
		patches[a].used = 0;
}
