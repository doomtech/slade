
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ZipArchive.cpp
 * Description: ZipArchive, archive class to handle zip format
 *              archives
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
#include "ZipArchive.h"
#include <wx/log.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/ptr_scpd.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/


/* ZipArchive::ZipArchive
 * ZipArchive class constructor
 *******************************************************************/
ZipArchive::ZipArchive()
: Archive() {
	type = ARCHIVE_ZIP;
}

/* ZipArchive::~ZipArchive
 * ZipArchive class destructor
 *******************************************************************/
ZipArchive::~ZipArchive() {
}

/* ZipArchive::openFile
 * Reads a zip format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool ZipArchive::openFile(string filename) {
	wxZipEntry* entry;

	// Open the file
	wxFFileInputStream in(filename);
	if (!in.IsOk()) {
		Global::error = _T("Unable to open file");
		return false;
	}

	// Create zip stream
	wxZipInputStream zip(in);
	if (!zip.IsOk()) {
		Global::error = _T("Invalid zip file");
		return false;
	}

	// Go through all zip entries
	int entry_index = 0;
	entry = zip.GetNextEntry();
	while (entry) {
		if (!entry->IsDir()) {
			// Get the entry name as a wxFileName (so we can break it up)
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);

			// Create entry
			ArchiveEntry *nlump = new ArchiveEntry(fn.GetName() + _T(".") + fn.GetExt(), this);

			// Setup entry info
			nlump->setSize(entry->GetSize());
			nlump->setLoaded(false);
			nlump->setExProp(_T("zip_index"), s_fmt(_T("%d"), entry_index));
			nlump->setExProp(_T("directory"), fn.GetPath(true, wxPATH_UNIX));

			wxLogMessage(_T("Entry: ") + nlump->getExProp(_T("directory")) + nlump->getName());

			// Add to entry list
			entries.push_back(nlump);
		}

		// Go to next entry in the zip file
		entry = zip.GetNextEntry();
		entry_index++;
	}

	// Setup variables
	this->filename = filename;

	return true;
}

/* ZipArchive::loadEntryData
 * Loads an entry's data from the zipfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool ZipArchive::loadEntryData(ArchiveEntry* entry) {
	wxLogMessage(_T("Load entry ") + entry->getName());

	// Check that the lump belongs to this wadfile
	if (entry->getParent() != this) {
		wxLogMessage(_T("ZipArchive::loadEntryData: Entry %s attempting to load data from wrong parent!"), entry->getName().c_str());
		return false;
	}

	// Do nothing if the lump's size is zero,
	// or if it has already been loaded
	if (entry->getSize() == 0 || entry->isLoaded()) {
		entry->setLoaded();
		return true;
	}

	// Check that the entry has a zip index
	int zip_index = 0;
	if (entry->hasExProp(_T("zip_index")))
		zip_index = atoi(chr(entry->getExProp(_T("zip_index"))));
	else {
		wxLogMessage(_T("ZipArchive::loadEntryData: Entry %s has no zip entry index!"), entry->getName().c_str());
		return false;
	}

	// Open the file
	wxFFileInputStream in(filename);
	if (!in.IsOk()) {
		wxLogMessage(_T("ZipArchive::loadEntryData: Unable to open zip file \"%s\"!"), filename.c_str());
		return false;
	}

	// Create zip stream
	wxZipInputStream zip(in);
	if (!zip.IsOk()) {
		wxLogMessage(_T("ZipArchive::loadEntryData: Invalid zip file \"%s\"!"), filename.c_str());
		return false;
	}

	// Skip to correct entry in zip
	wxZipEntry* zentry = zip.GetNextEntry();
	for (long a = 0; a < zip_index; a++)
		zentry = zip.GetNextEntry();

	// Read the data
	BYTE* data = new BYTE[zentry->GetSize()];
	zip.Read(data, zentry->GetSize());
	entry->setData(data);

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* ZipArchive::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
vector<Archive::mapdesc_t> ZipArchive::detectMaps() {
	vector<mapdesc_t> maps;

	return maps;
}
