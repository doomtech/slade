
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextureXEntryPanel.cpp
 * Description: TextureXEntryPanel class. The UI for editing TEXTUREx
 *              entries (and PNAMES)
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
#include "WxStuff.h"
#include "TextureXEntryPanel.h"
#include "Archive.h"
#include <wx/listctrl.h>


/*******************************************************************
 * TEXTENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* TextureXEntryPanel::TextureXEntryPanel
 * TextureXEntryPanel class constructor
 *******************************************************************/
TextureXEntryPanel::TextureXEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Setup panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Add textures list
	wxStaticBox* frame = new wxStaticBox(this, -1, _T("Textures"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	list_textures = new ListView(this, -1);
	list_textures->showIcons(false);
	framesizer->Add(list_textures, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Add texture canvas
	tex_canvas = new CompositeTextureCanvas(this, -1);
	sizer->Add(tex_canvas, 1, wxEXPAND|wxALL, 4);

	Layout();
}

/* TextureXEntryPanel::~TextureXEntryPanel
 * TextureXEntryPanel class destructor
 *******************************************************************/
TextureXEntryPanel::~TextureXEntryPanel() {
	for (size_t a = 0; a < textures.size(); a++)
		delete textures[a];
}

/* TextureXEntryPanel::loadEntry
 * Loads an entry into the TEXTUREx entry panel
 *******************************************************************/
bool TextureXEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Do nothing if entry is already open
	if (this->entry == entry)
		return true;

	// Read PNAMES entry
	vector<string> patch_names;
	ArchiveEntry* pnames = NULL;
	if (entry->getParent())
		pnames = entry->getParent()->getEntry(_T("PNAMES"));

	if (!pnames)
		return false;

	// Read number of pnames
	uint32_t n_pnames = 0;
	pnames->seek(0, SEEK_SET);
	pnames->read(&n_pnames, 4);

	// Read pnames content
	for (uint32_t a = 0; a < n_pnames; a++) {
		char pname[9] = "";
		pname[8] = 0;
		pnames->read(&pname, 8);
		patch_names.push_back(wxString(pname).Upper());
	}


	// Read TEXTUREx entry
	textures.clear();

	// Read header
	entry->seek(0, SEEK_SET);
	int32_t		n_tex = 0;
	int32_t*	offsets = NULL;

	// Number of textures
	entry->read(&n_tex, 4);

	// Texture definition offsets
	offsets = new int32_t[n_tex];
	entry->read(offsets, n_tex * 4);

	// Read all texture definitions
	for (int32_t a = 0; a < n_tex; a++) {
		// Read name
		char name[9] = "";
		name[8] = 0;
		entry->read(name, 8);

		// Read flags
		uint16_t flags;
		entry->read(&flags, 2);

		// Read scale
		uint8_t scale[2];
		entry->read(&scale, 2);

		// Read dimensions
		uint16_t width = 0;
		uint16_t height = 0;
		entry->read(&width, 2);
		entry->read(&height, 2);

		// Skip unused
		entry->seek(4, SEEK_CUR);

		// Create texture
		CompositeTexture* tex = new CompositeTexture(name, width, height);

		// Read patches
		uint16_t n_patches = 0;
		entry->read(&n_patches, 2);
		for (uint16_t p = 0; p < n_patches; p++) {
			// Read left offset
			uint16_t left = 0;
			entry->read(&left, 2);

			// Read top offset
			uint16_t top = 0;
			entry->read(&top, 2);

			// Read patch id
			uint16_t patch = 0;
			entry->read(&patch, 2);

			// Create patch
			CTPatch* ctp = new CTPatch();
			ctp->setOffsets(left, top);

			// Find patch image entry
			ArchiveEntry* img = entry->getParent()->getEntry(patch_names[patch]);
			if (!img) wxLogMessage(s_fmt(_T("Couldn't find patch %s"), patch_names[patch].c_str()));
			ctp->loadImage(img);

			// Add it to the texture
			tex->addPatch(ctp);

			// Skip unused
			entry->seek(4, SEEK_CUR);
		}

		// Add texture to list
		textures.push_back(tex);
	}

	// Clean up
	delete[] offsets;

	// Update variables
	this->entry = entry;

	// Refresh controls
	populateTextureList();
	Layout();

	return true;
}

/* TextureXEntryPanel::saveEntry
 * Saves any changes made to the entry
 *******************************************************************/
bool TextureXEntryPanel::saveEntry() {
	return false;
}

void TextureXEntryPanel::populateTextureList() {
	// Clear current list
	list_textures->ClearAll();

	// Add columns
	list_textures->InsertColumn(0, _T("Name"));
	list_textures->InsertColumn(1, _T("Size"));

	// Add each texture to the list
	list_textures->enableSizeUpdate(false);
	for (size_t a = 0; a < textures.size(); a++) {
		string cols[] = { textures[a]->getName(), s_fmt(_T("%dx%d"), textures[a]->getWidth(), textures[a]->getHeight()) };
		list_textures->addItem(a, wxArrayString(2, cols));
	}

	// Update list width
	list_textures->enableSizeUpdate(true);
	list_textures->updateSize();
}
