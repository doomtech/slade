
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


/*******************************************************************
 * ARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* Archive::Archive
 * Archive class constructor
 *******************************************************************/
Archive::Archive(uint8_t type) {
	this->type = type;
	modified = true;
	on_disk = false;
	parent = NULL;
}

/* Archive::~Archive
 * Archive class destructor
 *******************************************************************/
Archive::~Archive() {
}

/* Archive::getFileName
 * Returns the archive's filename, including the path if specified
 *******************************************************************/
string Archive::getFileName(bool fullpath) {
	// If the archive is within another archive, return "<parent archive>/<entry name>"
	if (parent) {
		string parent_archive = "";
		if (parent->getParent())
			parent_archive = parent->getParent()->getFileName(false) + "/";

		return parent_archive + parent->getName(false);
	}

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

	// Get the entry index and announce the change
	if (!isMuted()) {
		MemChunk mc;
		mc.reSize(8, false);
		wxUIntPtr ptr = wxPtrToUInt(entry);
		uint32_t index = entryIndex(entry);
		mc.write(&index, sizeof(uint32_t));
		mc.write(&ptr, sizeof(wxUIntPtr));
		announce("entry_modified", mc);
	}

	// If entry was set to unmodified, don't set the archive to modified
	// (should really rename this stuff to 'state changed' rather than 'modified')
	if (entry->getState() == 0)
		return;

	// Set the archive state to modified
	setModified(true);
}

void Archive::setModified(bool mod) {
	// Set modified
	modified = mod;

	// Announce
	announce("modified");
}

/* Archive::save
 * This is the general, all-purpose 'save archive' function. Takes
 * into account whether the archive is contained within another,
 * is already on the disk, etc etc. Does a 'save as' if [filename]
 * is specified, unless the archive is contained within another.
 * Returns false if saving was unsuccessful, true otherwise
 *******************************************************************/
bool Archive::save(string filename) {
	bool success = false;

	// If the archive has a parent ArchiveEntry, just write it to that
	if (parent) {
		success = write(parent->getMCData());
		parent->setState(1);
	}
	else {
		// Otherwise, file stuff
		if (!filename.IsEmpty()) {
			// New filename is given (ie 'save as'), write to new file and change archive filename accordingly
			success = write(filename);
			if (success) this->filename = filename;

			// Update variables
			this->on_disk = true;
		}
		else if (!this->filename.IsEmpty()) {
			// No filename is given, but the archive has a filename, so overwrite it (and make a backup)

			// Create backup
			if (wxFileName::FileExists(this->filename)) {
				string bakfile = this->filename + ".bak";

				// Remove old backup file
				wxRemoveFile(bakfile);

				// Copy current file contents to new backup file
				wxLogMessage("Creating backup %s", bakfile.c_str());
				wxCopyFile(this->filename, bakfile);
			}

			// Write it to the file
			success = write(this->filename);

			// Update variables
			this->on_disk = true;
		}
	}

	// If saving was successful, update variables and announce save
	if (success) {
		setModified(false);
		announce("saved");
	}

	return success;
}
