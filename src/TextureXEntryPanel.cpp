
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
#include "ArchiveManager.h"
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
	sizer_main->Add(sizer, 1, wxEXPAND, 0);

	// Add textures list
	wxStaticBox* frame = new wxStaticBox(this, -1, _T("Textures"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	list_textures = new ListView(this, -1);
	list_textures->showIcons(false);
	framesizer->Add(list_textures, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Add texture canvas
	tex_canvas = new CTextureCanvas(this, -1);
	sizer->Add(tex_canvas, 1, wxEXPAND|wxALL, 4);

	// Add patches list
	frame = new wxStaticBox(this, -1, _T("Patches"));
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	list_patches = new ListView(this, -1);
	list_patches->showIcons(false);
	framesizer->Add(list_patches, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);


	// Bind events
	list_textures->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureXEntryPanel::onTextureListSelect, this);

	Layout();
}

/* TextureXEntryPanel::~TextureXEntryPanel
 * TextureXEntryPanel class destructor
 *******************************************************************/
TextureXEntryPanel::~TextureXEntryPanel() {
}

/* TextureXEntryPanel::loadEntry
 * Loads an entry into the TEXTUREx entry panel
 *******************************************************************/
bool TextureXEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Do nothing if entry is already open
	if (this->entry == entry)
		return true;

	// Get PNAMES entry
	ArchiveEntry* pnames = NULL;
	if (entry->getParent())
		pnames = entry->getParent()->getEntry(_T("PNAMES"));		// Try to get from entry's parent archive first

	if (!pnames)
		pnames = theArchiveManager->getResourceEntry(_T("PNAMES"));	// Then try resource archives

	if (!pnames) {
		// Can't really do anything without pnames
		Global::error = _T("Could not find PNAMES entry");
		return false;
	}

	// Copy current entry content
	entry_data.clear();
	entry_data.importMem(entry->getData(true), entry->getSize());

	// Read TEXTUREx entry into texturexlist
	texturex.readTEXTUREXData(entry, pnames);

	// Update variables
	this->entry = entry;
	setModified(false);

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

/* TextureXEntryPanel::populateTextureList
 * Clears and adds all textures to the texture list
 *******************************************************************/
void TextureXEntryPanel::populateTextureList() {
	// Clear current list
	list_textures->ClearAll();

	// Add columns
	list_textures->InsertColumn(0, _T("Name"));
	list_textures->InsertColumn(1, _T("Size"));

	// Add each texture to the list
	list_textures->enableSizeUpdate(false);
	for (uint32_t a = 0; a < texturex.nTextures(); a++) {
		CTexture* tex = texturex.getTexture(a);
		string cols[] = { tex->getName(), s_fmt(_T("%dx%d"), tex->getWidth(), tex->getHeight()) };
		list_textures->addItem(a, wxArrayString(2, cols));
	}

	// Update list width
	list_textures->enableSizeUpdate(true);
	list_textures->updateSize();
}



void TextureXEntryPanel::onTextureListSelect(wxListEvent& e) {
	// Get the selected texture
	CTexture* tex = texturex.getTexture(e.GetIndex());

	// Open it if valid index (should be)
	if (tex)
		tex_canvas->openTexture(tex);
}
