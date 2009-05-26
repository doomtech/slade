
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

/* ZipArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string ZipArchive::getFileExtensionString() {
	return _T("Any Zip Format File (*.zip;*.pk3;*.jdf)|*.zip;*.pk3;*.jdf|Zip File (*.zip)|*.zip|Pk3 File (*.pk3)|*.pk3|JDF File (*.jdf)|*.jdf");
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
			setEntryDirectory(nlump, fn.GetPath(true, wxPATH_UNIX));
			setEntryZipIndex(nlump, entry_index);
			nlump->setState(0);

			//wxLogMessage(_T("Entry: ") + nlump->getExProp(_T("directory")) + nlump->getName());

			// Add to entry list
			entries.push_back(nlump);
		}

		// Go to next entry in the zip file
		entry = zip.GetNextEntry();
		entry_index++;
	}

	// Setup variables
	this->filename = filename;
	modified = false;

	return true;
}

/* ZipArchive::save
 * Saves the archive as a zip format file, if no filename is given,
 * the archive's current filename is used.
 * Returns true if successful, false otherwise
 *******************************************************************/
bool ZipArchive::save(string filename) {
	// If no filename specified, just use the current filename
	if (filename == _T(""))
		filename = this->filename;

	// Create a backup copy if needed
	string bakfile = filename + _T(".bak");
	if (wxFileName::FileExists(filename)) {
		// Remove old backup file
		remove(chr(bakfile));

		// Copy current file contents to new backup file
		wxCopyFile(this->filename, bakfile);
	}
	else
		bakfile = this->filename;

	// Open the file
	wxFFileOutputStream out(filename);
	if (!out.IsOk()) {
		Global::error = _T("Unable to open file for saving. Make sure it isn't in use by another program.");
		return false;
	}

	// Open as a zip to save
	wxZipOutputStream zip(out, 9);
	if (!zip.IsOk()) {
		Global::error = _T("Unable to create zip");
		return false;
	}

	// Open old zip for copying, if it exists. This is used to copy any entries
	// that have been previously saved/compressed and are unmodified, to greatly
	// speed up zip file saving (especially for large zips)
	wxFFileInputStream in(bakfile);
	wxZipInputStream inzip(in);

	// Get a list of all entries in the old zip
	wxZipEntry **c_entries = new wxZipEntry*[inzip.GetTotalEntries()];
	for (int a = 0; a < inzip.GetTotalEntries(); a++)
		c_entries[a] = inzip.GetNextEntry();

	// Go through all archive entries
	for (size_t a = 0; a < entries.size(); a++) {
		if (!inzip.IsOk() || entries[a]->getState() > 0 || getEntryZipIndex(entries[a]) == -1) {
			// If the current entry has been changed, or doesn't exist in the old zip,
			// (re)compress it's data and write it to the zip
			wxZipEntry* zipentry = new wxZipEntry(getEntryFullPath(entries[a]));
			zip.PutNextEntry(zipentry);
			zip.Write(entries[a]->getData(), entries[a]->getSize());
		}
		else {
			// If the entry is unmodified and was already compressed in the old zip, just copy it over
			zip.CopyEntry(c_entries[getEntryZipIndex(entries[a])], inzip);
			inzip.Reset();
		}

		// Update entry info
		entries[a]->setState(0);
		setEntryZipIndex(entries[a], a);
	}

	// Clean up and update variables
	delete[] c_entries;
	zip.Close();
	this->filename = filename;

	return true;
}

/* ZipArchive::loadEntryData
 * Loads an entry's data from the zipfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool ZipArchive::loadEntryData(ArchiveEntry* entry) {
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

/* ZipArchive::setEntryDirectory
 * Sets the specified entry's directory, if it is part of this
 * archive
 *******************************************************************/
void ZipArchive::setEntryDirectory(ArchiveEntry* entry, string dir) {
	if (entry->getParent() == this)
		entry->setExProp(_T("directory"), dir);
}

/* ZipArchive::getEntryDirectory
 * Gets the specified entry's directory, returns an empty string if
 * the entry isn't part of this archive
 *******************************************************************/
string ZipArchive::getEntryDirectory(ArchiveEntry* entry) {
	if (entry->getParent() != this)
		return _T("");
	else
		return entry->getExProp(_T("directory"));
}

/* ZipArchive::getEntryFullPath
 * Gets the specified entry's full path (directory+name+extension),
 * returns an empty string if the entry isn't part of this archive
 *******************************************************************/
string ZipArchive::getEntryFullPath(ArchiveEntry* entry) {
	if (entry->getParent() != this)
		return _T("");
	else
		return getEntryDirectory(entry) + entry->getName();
}

/* ZipArchive::setEntryZipIndex
 * Sets the entry's zip index. This is the index of the entry in the
 * currently saved zip file, used to copy the compressed entry data
 * when saving if the entry data hasn't changed.
 *******************************************************************/
void ZipArchive::setEntryZipIndex(ArchiveEntry* entry, int index) {
	if (entry->getParent() == this)
		entry->setExProp(_T("zip_index"), s_fmt(_T("%d"), index));
}

/* ZipArchive::getEntryZipIndex
 * Gets the entry's zip index. Returns -1 if the entry is not part
 * of this archive
 *******************************************************************/
int ZipArchive::getEntryZipIndex(ArchiveEntry* entry) {
	if (entry->getParent() != this)
		return -1;
	else
		return atoi(entry->getExProp(_T("zip_index")).ToAscii());
}

/* ZipArchive::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
vector<Archive::mapdesc_t> ZipArchive::detectMaps() {
	vector<mapdesc_t> maps;

	return maps;
}
