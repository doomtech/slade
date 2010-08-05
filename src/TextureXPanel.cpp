
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextureXPanel.cpp
 * Description: The UI for viewing/editing a texture definitions
 *              entry (TEXTURE1/2/S)
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
#include "TextureXPanel.h"
#include "Misc.h"
#include <wx/filename.h>


/*******************************************************************
 * TEXTUREXLISTVIEW CLASS FUNCTIONS
 *******************************************************************/

/* TextureXListView::TextureXListView
 * TextureXListView class constructor
 *******************************************************************/
TextureXListView::TextureXListView(wxWindow* parent, TextureXList* texturex) : VirtualListView(parent) {
	// Init variables
	this->texturex = texturex;

	// Add columns
	InsertColumn(0, "Name");
	InsertColumn(1, "Size");

	// Update
	updateList();
}

/* TextureXListView::~TextureXListView
 * TextureXListView class destructor
 *******************************************************************/
TextureXListView::~TextureXListView() {
}

/* TextureXListView::getItemText
 * Returns the string for [item] at [column]
 *******************************************************************/
string TextureXListView::getItemText(long item, long column) const {
	// Check texture list exists
	if (!texturex)
		return "INVALID INDEX";

	// Check index is ok
	if (item < 0 || (unsigned)item > texturex->nTextures())
		return "INVALID INDEX";

	// Get associated texture
	CTexture* tex = texturex->getTexture(item);

	if (column == 0)						// Name column
		return tex->getName();
	else if (column == 1)					// Size column
		return s_fmt("%dx%d", tex->getWidth(), tex->getHeight());
	else
		return "INVALID COLUMN";
}

/* TextureXListView::updateList
 * Clears the list if [clear] is true, and refreshes it
 *******************************************************************/
void TextureXListView::updateList(bool clear) {
	if (clear)
		ClearAll();

	// Set list size
	if (texturex)
		SetItemCount(texturex->nTextures());
	else
		SetItemCount(0);

	updateWidth();
	Refresh();
}


/*******************************************************************
 * TEXTUREXPANEL CLASS FUNCTIONS
 *******************************************************************/

/* TextureXPanel::TextureXPanel
 * TextureXPanel class constructor
 *******************************************************************/
TextureXPanel::TextureXPanel(wxWindow* parent, PatchTable* patch_table) : wxPanel(parent, -1) {
	// Init variables
	this->tx_entry = NULL;
	this->patch_table = patch_table;
	this->tex_current = NULL;

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Add textures list
	wxStaticBox* frame = new wxStaticBox(this, -1, "Textures");
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	label_tx_format = new wxStaticText(this, -1, "Format:");
	framesizer->Add(label_tx_format, 0, wxEXPAND|wxALL, 4);
	list_textures = new TextureXListView(this, &texturex);
	framesizer->Add(list_textures, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// New Texture button
	btn_new_texture = new wxButton(this, -1, "New");
	framesizer->Add(btn_new_texture, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// New Texture from Patch button
	btn_new_from_patch = new wxButton(this, -1, "New From Patch");
	framesizer->Add(btn_new_from_patch, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// New Texture from File button
	btn_new_from_file = new wxButton(this, -1, "New From File");
	framesizer->Add(btn_new_from_file, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Remove Texture button
	btn_remove_texture = new wxButton(this, -1, "Remove");
	framesizer->Add(btn_remove_texture, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Add texture editor area
	texture_editor = new TextureEditorPanel(this, patch_table);
	sizer->Add(texture_editor, 1, wxEXPAND|wxALL, 4);

	// Bind events
	list_textures->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureXPanel::onTextureListSelect, this);
	btn_new_texture->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnNewTexture, this);
	btn_new_from_patch->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnNewTextureFromPatch, this);
	btn_new_from_file->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnNewTextureFromFile, this);
	btn_remove_texture->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnRemoveTexture, this);
}

/* TextureXPanel::~TextureXPanel
 * TextureXPanel class destructor
 *******************************************************************/
TextureXPanel::~TextureXPanel() {
	if (tx_entry)
		tx_entry->unlock();
}

/* TextureXPanel::openTEXTUREX
 * Loads a TEXTUREX format texture list into the editor
 *******************************************************************/
bool TextureXPanel::openTEXTUREX(ArchiveEntry* entry) {
	if (texturex.readTEXTUREXData(entry, *patch_table)) {
		tx_entry = entry;

		// Update patch table usage info
		for (size_t a = 0; a < texturex.nTextures(); a++) {
			CTexture* tex = texturex.getTexture(a);

			// Go through texture's patches
			for (size_t p = 0; p < tex->nPatches(); p++)
				patch_table->patch(tex->getPatch(p)->getName()).used_in.push_back(tex->getName());
		}

		// Update format label
		label_tx_format->SetLabel("Format: " + texturex.getTextureXFormatString());

		// Update texture list
		list_textures->updateList();

		return true;
	}
	else
		return false;
}

/* TextureXPanel::saveTEXTUREX
 * Saves a TEXTUREX format texture list
 *******************************************************************/
bool TextureXPanel::saveTEXTUREX() {
	// Save any changes to current texture
	applyChanges();

	tx_entry->unlock();	// Have to unlock the entry first
	bool ok = texturex.writeTEXTUREXData(tx_entry, *patch_table);
	tx_entry->lock();

	return ok;
}


/* TextureXPanel::setPalette
 * Sets the texture editor's palette
 *******************************************************************/
void TextureXPanel::setPalette(Palette8bit *pal) {
	texture_editor->setPalette(pal);
}

/* TextureXPanel::applyChanges
 * Applies changes to the current texture, if any
 *******************************************************************/
void TextureXPanel::applyChanges() {
	if (texture_editor->texModified() && tex_current) {
		tex_current->copyTexture(texture_editor->getTexture());
		patch_table->updatePatchUsage(tex_current);
		list_textures->updateList();
	}
}

/* TextureXPanel::newTextureFromPatch
 * Creates a new texture called [name] from [patch]. The new texture
 * will be set to the dimensions of the patch, with the patch added
 * at 0,0
 *******************************************************************/
CTexture* TextureXPanel::newTextureFromPatch(string name, string patch) {
	// Load patch image to get dimensions (yeah it's not optimal, but at the moment it's the best I can do)
	SImage image;
	ArchiveEntry* patch_entry = patch_table->patchEntry(patch);
	Misc::loadImageFromEntry(&image, patch_entry);

	// Create new texture
	CTexture* tex = new CTexture();
	tex->setName(name);

	// Set dimensions
	tex->setWidth(image.getWidth());
	tex->setHeight(image.getHeight());

	// Add patch
	tex->addPatch(patch, 0, 0, patch_entry);

	// Return the new texture
	return tex;
}


/*******************************************************************
 * TEXTUREXPANEL EVENTS
 *******************************************************************/

/* TextureXPanel::onTextureListSelect
 * Called when an item on the texture list is selected
 *******************************************************************/
void TextureXPanel::onTextureListSelect(wxListEvent& e) {
	// Get selected texture
	CTexture* tex = texturex.getTexture(e.GetIndex());

	// Save any changes to previous texture
	applyChanges();

	// Open texture in editor
	texture_editor->openTexture(tex);

	// Set current texture
	tex_current = tex;
}

/* TextureXPanel::onBtnNewTexture
 * Called when the 'New Texture' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnNewTexture(wxCommandEvent& e) {
	// Prompt for new texture name
	string name = wxGetTextFromUser("Enter a texture name:", "New Texture");

	// Do nothing if no name entered
	if (name.IsEmpty())
		return;

	// Process name
	name = name.Upper().Truncate(8);

	// Create new texture
	CTexture* tex = new CTexture();
	tex->setName(name);

	// Default size = 64x128
	tex->setWidth(64);
	tex->setHeight(128);

	// Add it after the last selected item
	int selected = list_textures->getLastSelected();
	if (selected == -1) selected = texturex.nTextures() - 1; // Add to end of the list if nothing selected
	texturex.addTexture(tex, selected + 1);

	// Update texture list
	list_textures->updateList();

	// Select the new texture
	list_textures->clearSelection();
	list_textures->selectItem(selected + 1);
	list_textures->EnsureVisible(selected + 1);
}

/* TextureXPanel::onBtnNewTextureFromPatch
 * Called when the 'New Texture from Patch' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnNewTextureFromPatch(wxCommandEvent& e) {
	// Do nothing if patch list is empty
	if (patch_table->nPatches() == 0)
		return;

	// Temporary choice dialog
	wxArrayString patches;
	for (size_t a = 0; a < patch_table->nPatches(); a++) patches.Add(patch_table->patchName(a));
	wxSingleChoiceDialog dlg(this, "Select a patch", "Create Texture from Patch", patches);

	if (dlg.ShowModal() == wxID_OK) {
		// Prompt for new texture name
		string name = wxGetTextFromUser("Enter a texture name:", "New Texture", dlg.GetStringSelection());

		// Do nothing if no name entered
		if (name.IsEmpty())
			return;

		// Process name
		name = name.Upper().Truncate(8);

		// Create new texture from patch
		CTexture* tex = newTextureFromPatch(name, dlg.GetStringSelection());

		// Add texture after the last selected item
		int selected = list_textures->getLastSelected();
		if (selected == -1) selected = texturex.nTextures() - 1; // Add to end of the list if nothing selected
		texturex.addTexture(tex, selected + 1);

		// Update texture list
		list_textures->updateList();

		// Select the new texture
		list_textures->clearSelection();
		list_textures->selectItem(selected + 1);
		list_textures->EnsureVisible(selected + 1);

		// Update patch table counts
		patch_table->updatePatchUsage(tex);
	}
}

/* TextureXPanel::onBtnNewTextureFromFile
 * Called when the 'New Texture from File' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnNewTextureFromFile(wxCommandEvent& e) {
	// Get all entry types
	vector<EntryType*> etypes = EntryType::allTypes();

	// Go through types
	string ext_filter = "All files (*.*)|*.*|";
	for (unsigned a = 0; a < etypes.size(); a++) {
		// If the type is a valid image type, add its extension filter
		if (etypes[a]->extraProps().propertyExists("image")) {
			ext_filter += etypes[a]->getFileFilterString();
			ext_filter += "|";
		}
	}

	// Create open file dialog
	wxFileDialog dialog_open(this, "Choose file(s) to open", wxEmptyString, wxEmptyString,
			ext_filter, wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		// Get file selection
		wxArrayString files;
		dialog_open.GetPaths(files);

		// Go through file selection
		for (unsigned a = 0; a < files.size(); a++) {
			// Load the file into a temporary ArchiveEntry
			ArchiveEntry* entry = new ArchiveEntry();
			entry->importFile(files[a]);

			// Determine type
			EntryType::detectEntryType(entry);

			// If it's not a valid image type, ignore this file
			if (!entry->getType()->extraProps().propertyExists("image")) {
				wxLogMessage("%s is not a valid image file", chr(files[a]));
				continue;
			}

			// Ask for name for texture
			wxFileName fn(files[a]);
			string name = fn.GetName().Upper().Truncate(8);
			name = wxGetTextFromUser(s_fmt("Enter a texture name for %s:", chr(fn.GetFullName())), "New Texture", name);
			name = name.Truncate(8);

			// Add patch to archive
			entry->setName(name);
			entry->setExtensionByType();
			tx_entry->getParent()->addEntry(entry, "patches");

			// Add patch to patch table
			patch_table->addPatch(name);


			// Create new texture from patch
			CTexture* tex = newTextureFromPatch(name, name);

			// Add texture after the last selected item
			int selected = list_textures->getLastSelected();
			if (selected == -1) selected = texturex.nTextures() - 1; // Add to end of the list if nothing selected
			texturex.addTexture(tex, selected + 1);

			// Update texture list
			list_textures->updateList();

			// Select the new texture
			list_textures->clearSelection();
			list_textures->selectItem(selected + 1);
			list_textures->EnsureVisible(selected + 1);

			// Update patch table counts
			patch_table->updatePatchUsage(tex);
		}
	}
}

/* TextureXPanel::onBtnRemoveTexture
 * Called when the 'Remove Texture' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnRemoveTexture(wxCommandEvent& e) {
	// Get selected textures
	vector<long> selection = list_textures->getSelection();

	// Go through selection backwards
	for (int a = selection.size() - 1; a >= 0; a--) {
		// Remove texture from patch table entries
		CTexture* tex = texturex.getTexture(selection[a]);
		for (unsigned p = 0; p < tex->nPatches(); p++)
			patch_table->patch(tex->getPatch(p)->getName()).removeTextureUsage(tex->getName());

		// Remove texture from list
		texturex.removeTexture(selection[a]);
	}

	// Clear selection & refresh
	list_textures->clearSelection();
	list_textures->updateList();
}
