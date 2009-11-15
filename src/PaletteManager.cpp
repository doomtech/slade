
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


/*******************************************************************
 * VARIABLES
 *******************************************************************/
PaletteManager* PaletteManager::instance = NULL;


PaletteManager::PaletteManager() {
	// Load palettes from SLADE.pk3
	loadResourcePalettes();

	// Load custom palettes (from <user directory>/palettes)
	loadCustomPalettes();
}

PaletteManager::~PaletteManager() {
	for (int a = 0; a < palettes.size(); a++)
		delete[] palettes[a];
}

bool PaletteManager::loadResourcePalettes() {
	// Get the 'palettes' directory of SLADE.pk3
	ZipArchive* res_archive = (ZipArchive*)(theArchiveManager->resourceArchive());
	zipdir_t* dir_palettes = res_archive->getDirectory(_T("palettes"));

	// Go through all entries in the directory
	for (int a = 0; a < dir_palettes->entries.size(); a++) {
		// Load palette data
		Palette8bit* pal = new Palette8bit();
		MemChunk mc(dir_palettes->entries[a]->getData(true), dir_palettes->entries[a]->getSize());
		pal->loadMem(mc);

		// Add the palette
		palettes.push_back(pal);
		pal_names.push_back(dir_palettes->entries[a]->getName());
	}
}

bool PaletteManager::loadCustomPalettes() {
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
		pal_names.push_back(filename);

		// Next file
		files = res_dir.GetNext(&filename);
	}
}
