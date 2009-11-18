
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PaletteManager.cpp
 * Description: PaletteManager class. Manages all resource/custom
 *              palettes for viewing doom gfx/flats and conversions
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
#include "PaletteManager.h"
#include "ArchiveManager.h"
#include "ZipArchive.h"
#include <wx/dir.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
PaletteManager* PaletteManager::instance = NULL;



/* PaletteManager::PaletteManager
 * PaletteManager class constructor
 *******************************************************************/
PaletteManager::PaletteManager() {
	// Load palettes from SLADE.pk3
	loadResourcePalettes();

	// Load custom palettes (from <user directory>/palettes)
	loadCustomPalettes();

	// Init other stuff
	pal_default = new Palette8bit();
	pal_global = new Palette8bit();
}

/* PaletteManager::~PaletteManager
 * PaletteManager class destructor
 *******************************************************************/
PaletteManager::~PaletteManager() {
	for (int a = 0; a < palettes.size(); a++)
		delete[] palettes[a];
}

/* PaletteManager::getPalette
 * Returns the palette at [index]. or the default palette (greyscale)
 * if index is out of bounds
 *******************************************************************/
Palette8bit* PaletteManager::getPalette(int index) {
	if (index < 0 || index >= numPalettes())
		return pal_default;
	else
		return palettes[index];
}

/* PaletteManager::getPalette
 * Returns the palette matching the given name, or the default
 * palette (greyscale) if no matching palette found
 *******************************************************************/
Palette8bit* PaletteManager::getPalette(string name) {
	for (uint32_t a = 0; a < pal_names.size(); a++) {
		if (pal_names[a].Cmp(name) == 0)
			return palettes[a];
	}

	return pal_default;
}

/* PaletteManager::getPalName
 * Returns the name of the palette at [index], or an empty string if
 * index is out of bounds
 *******************************************************************/
string PaletteManager::getPalName(int index) {
	if (index < 0 || index >= numPalettes())
		return _T("");
	else
		return pal_names[index];
}

/* PaletteManager::getPalName
 * Returns the name of the given palette, or an empty string if the
 * palette isn't managed by the PaletteManager
 *******************************************************************/
string PaletteManager::getPalName(Palette8bit* pal) {
	for (uint32_t a = 0; a < palettes.size(); a++) {
		if (palettes[a] == pal)
			return pal_names[a];
	}

	return _T("");
}

/* PaletteManager::loadResourcePalettes
 * Loads any entries in the 'palettes' directory of SLADE.pk3 as
 * palettes, with names from the entries (minus the entry extension)
 *******************************************************************/
bool PaletteManager::loadResourcePalettes() {
	// Get the 'palettes' directory of SLADE.pk3
	ZipArchive* res_archive = (ZipArchive*)(theArchiveManager->resourceArchive());
	zipdir_t* dir_palettes = res_archive->getDirectory(_T("palettes"));

	// Check it exists
	if (!dir_palettes)
		return false;

	// Go through all entries in the directory
	for (int a = 0; a < dir_palettes->entries.size(); a++) {
		// Load palette data
		Palette8bit* pal = new Palette8bit();
		MemChunk mc(dir_palettes->entries[a]->getData(true), dir_palettes->entries[a]->getSize());
		pal->loadMem(mc);

		// Add the palette
		palettes.push_back(pal);
		pal_names.push_back(dir_palettes->entries[a]->getName(true));
	}
}

/* PaletteManager::loadCustomPalettes
 * Loads any files in the '<userdir>/palettes' directory as palettes,
 * with names from the files (minus the file extension)
 *******************************************************************/
bool PaletteManager::loadCustomPalettes() {
	// If the directory doesn't exist create it
	if (!wxDirExists(appPath(_T("palettes"), DIR_USER)))
		wxMkdir(appPath(_T("palettes"), DIR_USER));

	// Open the custom palettes directory
	wxDir res_dir;
	res_dir.Open(appPath(_T("palettes"), DIR_USER));

	// Go through each file in the directory
	string filename = wxEmptyString;
	bool files = res_dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (files) {
		// Load palette data
		Palette8bit* pal = new Palette8bit();
		MemChunk mc;
		mc.loadFile(filename);
		pal->loadMem(mc);

		// Add the palette
		palettes.push_back(pal);
		pal_names.push_back(wxFileName(filename).GetName());

		// Next file
		files = res_dir.GetNext(&filename);
	}
}