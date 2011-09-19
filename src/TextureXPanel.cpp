
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
#include "TextureXEditor.h"
#include "ZTextureEditorPanel.h"
#include "Clipboard.h"
#include "ArchiveManager.h"
#include "Icons.h"
#include "ResourceManager.h"
#include "GfxConvDialog.h"
#include "SplashWindow.h"
#include <wx/filename.h>
#include <wx/gbsizer.h>


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(String, dir_last)
EXTERN_CVAR(Bool, wad_force_uppercase)


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
		return S_FMT("%dx%d", tex->getWidth(), tex->getHeight());
	else if (column == 2)					// Type column
		return tex->getType();
	else
		return "INVALID COLUMN";
}

/* TextureXListView::updateItemAttr
 * Called when widget requests the attributes (text colour /
 * background colour / font) for [item]
 *******************************************************************/
void TextureXListView::updateItemAttr(long item) const {
	// Check texture list exists
	if (!texturex)
		return;

	// Check index is ok
	if (item < 0 || (unsigned)item > texturex->nTextures())
		return;

	// Get associated texture
	CTexture* tex = texturex->getTexture(item);

	// Init attributes
	item_attr->SetTextColour(ListView::colourError());

	// If texture doesn't exist, return error colour
	if (!tex)
		return;

	// Set colour depending on entry state
	switch (tex->getState()) {
	case 1:
		item_attr->SetTextColour(ListView::colourModified());
		break;
	case 2:
		item_attr->SetTextColour(ListView::colourNew());
		break;
	default:
		item_attr->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
		break;
	};
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
TextureXPanel::TextureXPanel(wxWindow* parent, TextureXEditor* tx_editor) : wxPanel(parent, -1) {
	// Init variables
	this->tx_entry = NULL;
	this->tx_editor = tx_editor;
	this->tex_current = NULL;
	this->modified = false;

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

	// Add texture operations buttons
	wxGridBagSizer* gbsizer = new wxGridBagSizer(4, 4);
	framesizer->Add(gbsizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	btn_move_up = new wxBitmapButton(this, -1, getIcon("t_up"));
	btn_move_up->SetToolTip("Move Up");
	btn_move_down = new wxBitmapButton(this, -1, getIcon("t_down"));
	btn_move_down->SetToolTip("Move Down");
	btn_new_texture = new wxBitmapButton(this, -1, getIcon("t_tex_new"));
	btn_new_texture->SetToolTip("New");
	btn_remove_texture = new wxBitmapButton(this, -1, getIcon("t_tex_delete"));
	btn_remove_texture->SetToolTip("Remove");
	btn_new_from_patch = new wxBitmapButton(this, -1, getIcon("t_tex_newpatch"));
	btn_new_from_patch->SetToolTip("New from Patch");
	btn_new_from_file = new wxBitmapButton(this, -1, getIcon("t_tex_newfile"));
	btn_new_from_file->SetToolTip("New from File");
	gbsizer->Add(btn_new_texture, wxGBPosition(0, 0), wxDefaultSpan);
	gbsizer->Add(btn_new_from_patch, wxGBPosition(0, 1), wxDefaultSpan);
	gbsizer->Add(btn_new_from_file, wxGBPosition(0, 2), wxDefaultSpan);
	gbsizer->Add(btn_remove_texture, wxGBPosition(0, 3), wxDefaultSpan);
	gbsizer->Add(btn_move_up, wxGBPosition(0, 4), wxDefaultSpan);
	gbsizer->Add(btn_move_down, wxGBPosition(0, 5), wxDefaultSpan);

	// Bind events
	list_textures->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureXPanel::onTextureListSelect, this);
	list_textures->Bind(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, &TextureXPanel::onTextureListRightClick, this);
	list_textures->Bind(wxEVT_KEY_DOWN, &TextureXPanel::onTextureListKeyDown, this);
	btn_new_texture->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnNewTexture, this);
	btn_new_from_patch->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnNewTextureFromPatch, this);
	btn_new_from_file->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnNewTextureFromFile, this);
	btn_remove_texture->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnRemoveTexture, this);
	btn_move_up->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnMoveUp, this);
	btn_move_down->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXPanel::onBtnMoveDown, this);
	Bind(wxEVT_SHOW, &TextureXPanel::onShow, this);
}

/* TextureXPanel::~TextureXPanel
 * TextureXPanel class destructor
 *******************************************************************/
TextureXPanel::~TextureXPanel() {
	if (tx_entry)
		tx_entry->unlock();
}

/* TextureXPanel::openTEXTUREX
 * Loads a TEXTUREx or TEXTURES format texture list into the editor
 *******************************************************************/
bool TextureXPanel::openTEXTUREX(ArchiveEntry* entry) {
	// Open texture list (check format)
	if (entry->getType()->getFormat() == "texturex") {
		// TEXTURE1/2 format
		if (!texturex.readTEXTUREXData(entry, tx_editor->patchTable()))
			return false;

		// Create default texture editor
		texture_editor = new TextureEditorPanel(this, tx_editor);

		// Update patch table usage info
		for (size_t a = 0; a < texturex.nTextures(); a++) {
			CTexture* tex = texturex.getTexture(a);

			// Go through texture's patches
			for (size_t p = 0; p < tex->nPatches(); p++)
				tx_editor->patchTable().patch(tex->getPatch(p)->getName()).used_in.push_back(tex->getName());
		}
	}
	else {
		// TEXTURES format
		if (!texturex.readTEXTURESData(entry))
			return false;

		// Create extended texture editor
		texture_editor = new ZTextureEditorPanel(this, tx_editor);

		// Add 'type' column
		list_textures->InsertColumn(2, "Type");
	}

	tx_entry = entry;

	// Add texture editor area
	GetSizer()->Add(texture_editor, 1, wxEXPAND|wxALL, 4);
	texture_editor->setupLayout();

	// Update format label
	label_tx_format->SetLabel("Format: " + texturex.getTextureXFormatString());

	// Update texture list
	list_textures->updateList();

	return true;
}

/* TextureXPanel::saveTEXTUREX
 * Saves a TEXTUREX format texture list
 *******************************************************************/
bool TextureXPanel::saveTEXTUREX() {
	// Save any changes to current texture
	applyChanges();

	// Write list to entry, in the correct format
	tx_entry->unlock();	// Have to unlock the entry first
	bool ok = false;
	if (texturex.getFormat() == TXF_TEXTURES)
		ok = texturex.writeTEXTURESData(tx_entry);
	else
		ok = texturex.writeTEXTUREXData(tx_entry, tx_editor->patchTable());

	// Redetect type and lock it up
	EntryType::detectEntryType(tx_entry);
	tx_entry->lock();

	// Set all textures to unmodified
	for (unsigned a = 0; a < texturex.nTextures(); a++)
		texturex.getTexture(a)->setState(0);
	list_textures->updateList();

	// Update variables
	modified = false;

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
		tex_current->setState(1);
		tx_editor->patchTable().updatePatchUsage(tex_current);
		list_textures->updateList();
		modified = true;
		texture_editor->openTexture(tex_current, &texturex);
	}
}

/* TextureXPanel::newTextureFromPatch
 * Creates a new texture called [name] from [patch]. The new texture
 * will be set to the dimensions of the patch, with the patch added
 * at 0,0
 *******************************************************************/
CTexture* TextureXPanel::newTextureFromPatch(string name, string patch) {
	// Create new texture
	CTexture* tex = new CTexture();
	tex->setName(name);
	tex->setState(2);

	// Setup texture scale
	if (texturex.getFormat() == TXF_TEXTURES) {
		tex->setScale(1, 1);
		tex->setExtended(true);
	}
	else
		tex->setScale(0, 0);

	// Add patch
	tex->addPatch(patch, 0, 0);

	// Load patch image (to determine dimensions)
	SImage image;
	tex->loadPatchImage(0, image);

	// Set dimensions
	tex->setWidth(image.getWidth());
	tex->setHeight(image.getHeight());

	// Update variables
	modified = true;

	// Return the new texture
	return tex;
}

/* TextureXPanel::newTexture
 * Creates a new, empty texture
 *******************************************************************/
void TextureXPanel::newTexture() {
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
	tex->setState(2);

	// Default size = 64x128
	tex->setWidth(64);
	tex->setHeight(128);

	// Setup texture scale
	if (texturex.getFormat() == TXF_TEXTURES) {
		tex->setScale(1, 1);
		tex->setExtended(true);
	}
	else
		tex->setScale(0, 0);

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

	// Update variables
	modified = true;
}

/* TextureXPanel::newTextureFromPatch
 * Creates a new texture from an existing patch
 *******************************************************************/
void TextureXPanel::newTextureFromPatch() {
	// Browse for patch
	string patch;
	if (texturex.getFormat() == TXF_TEXTURES)
		patch = tx_editor->browsePatchEntry();
	else
		patch = tx_editor->patchTable().patchName(tx_editor->browsePatchTable());

	if (!patch.IsEmpty()) {
		// Prompt for new texture name
		string name = wxGetTextFromUser("Enter a texture name:", "New Texture", patch);

		// Do nothing if no name entered
		if (name.IsEmpty())
			return;

		// Process name
		name = name.Upper().Truncate(8);

		// Create new texture from patch
		CTexture* tex = newTextureFromPatch(name, patch);

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
		tx_editor->patchTable().updatePatchUsage(tex);
	}
}

/* TextureXPanel::newTextureFromFile
 * Creates a new texture from an image file. The file will be
 * imported and added to the patch table if needed
 *******************************************************************/
void TextureXPanel::newTextureFromFile() {
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
	wxFileDialog dialog_open(this, "Choose file(s) to open", dir_last, wxEmptyString,
			ext_filter, wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		// Get file selection
		wxArrayString files;
		dialog_open.GetPaths(files);

		// Save 'dir_last'
		dir_last = dialog_open.GetDirectory();

		// Go through file selection
		for (unsigned a = 0; a < files.size(); a++) {
			// Load the file into a temporary ArchiveEntry
			ArchiveEntry* entry = new ArchiveEntry();
			entry->importFile(files[a]);

			// Determine type
			EntryType::detectEntryType(entry);

			// If it's not a valid image type, ignore this file
			if (!entry->getType()->extraProps().propertyExists("image")) {
				wxLogMessage("%s is not a valid image file", CHR(files[a]));
				continue;
			}

			// Ask for name for texture
			wxFileName fn(files[a]);
			string name = fn.GetName().Upper().Truncate(8);
			name = wxGetTextFromUser(S_FMT("Enter a texture name for %s:", CHR(fn.GetFullName())), "New Texture", name);
			name = name.Truncate(8);

			// Add patch to archive
			entry->setName(name);
			entry->setExtensionByType();
			tx_entry->getParent()->addEntry(entry, "patches");

			// Add patch to patch table if needed
			if (texturex.getFormat() != TXF_TEXTURES)
				tx_editor->patchTable().addPatch(name);


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
			tx_editor->patchTable().updatePatchUsage(tex);
		}
	}
}

/* TextureXPanel::removeTexture
 * Removes any selected textures
 *******************************************************************/
void TextureXPanel::removeTexture() {
	// Get selected textures
	vector<long> selection = list_textures->getSelection();

	// Go through selection backwards
	for (int a = selection.size() - 1; a >= 0; a--) {
		// Remove texture from patch table entries
		CTexture* tex = texturex.getTexture(selection[a]);
		for (unsigned p = 0; p < tex->nPatches(); p++)
			tx_editor->patchTable().patch(tex->getPatch(p)->getName()).removeTextureUsage(tex->getName());

		// Remove texture from list
		texturex.removeTexture(selection[a]);
	}

	// Clear selection & refresh
	list_textures->clearSelection();
	list_textures->updateList();
	texture_editor->clearTexture();

	// Update variables
	modified = true;
}

/* TextureXPanel::moveUp
 * Moves all selected textures up
 *******************************************************************/
void TextureXPanel::moveUp() {
	// Get selected textures
	vector<long> selection = list_textures->getSelection();

	// Do nothing if the first selected item is at the top of the list
	if (selection.size() > 0 && selection[0] == 0)
		return;

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		// Swap selected texture with the one above it
		texturex.swapTextures(selection[a], selection[a] - 1);
	}

	// Update selection
	list_textures->clearSelection();
	for (unsigned a = 0; a < selection.size(); a++)
		list_textures->selectItem(selection[a] - 1);

	// Refresh
	list_textures->updateList();

	// Update variables
	modified = true;
}

/* TextureXPanel::moveDown
 * Moves all selected textures down
 *******************************************************************/
void TextureXPanel::moveDown() {
	// Get selected textures
	vector<long> selection = list_textures->getSelection();

	// Do nothing if the last selected item is at the end of the list
	if (selection.size() > 0 && selection.back() == list_textures->GetItemCount()-1)
		return;

	// Go through selection backwards
	for (int a = selection.size()-1; a >= 0; a--) {
		// Swap selected texture with the one below it
		texturex.swapTextures(selection[a], selection[a] + 1);
	}

	// Update selection
	list_textures->clearSelection();
	for (unsigned a = 0; a < selection.size(); a++)
		list_textures->selectItem(selection[a] + 1);

	// Refresh
	list_textures->updateList();

	// Update variables
	modified = true;
}

/* TextureXPanel::copy
 * Copies any selected textures to the clipboard
 *******************************************************************/
void TextureXPanel::copy() {
	// Get selected textures
	vector<long> selection = list_textures->getSelection();

	// Do nothing if nothing selected
	if (selection.size() == 0)
		return;

	// Create list of textures to copy
	vector<ClipboardItem*> copy_items;
	for (unsigned a = 0; a < selection.size(); a++)
		copy_items.push_back(new TextureClipboardItem(texturex.getTexture(selection[a]), tx_editor->getArchive()));

	// Add list to clipboard
	theClipboard->addItems(copy_items);
}

/* TextureXPanel::paste
 * Pastes any textures on the clipboard after the last selected
 * texture
 *******************************************************************/
void TextureXPanel::paste() {
	// Check there is anything on the clipboard
	if (theClipboard->nItems() == 0)
		return;

	// Get last selected index
	int selected = list_textures->getLastSelected();
	if (selected == -1) selected = texturex.nTextures() - 1; // Add to end of the list if nothing selected

	// Go through clipboard items
	for (unsigned a = 0; a < theClipboard->nItems(); a++) {
		// Skip if not a texture clipboard item
		if (theClipboard->getItem(a)->getType() != CLIPBOARD_COMPOSITE_TEXTURE)
			continue;

		// Get texture item
		TextureClipboardItem* item = (TextureClipboardItem*)(theClipboard->getItem(a));

		// Add new texture after last selected item
		CTexture* ntex = new CTexture((texturex.getFormat() == TXF_TEXTURES));
		ntex->copyTexture(item->getTexture(), true);
		ntex->setState(2);
		texturex.addTexture(ntex, ++selected);

		// Deal with patches
		for (unsigned p = 0; p < ntex->nPatches(); p++) {
			CTPatch* patch = ntex->getPatch(p);

			// Update patch table if necessary
			if (texturex.getFormat() != TXF_TEXTURES)
				tx_editor->patchTable().addPatch(patch->getName());

			// Get the entry for this patch
			ArchiveEntry* entry = patch->getPatchEntry(tx_editor->getArchive());

			// If the entry wasn't found in any open archive, try copying it from the clipboard
			// (the user may have closed the archive the original patch was in)
			if (!entry) {
				entry = item->getPatchEntry(patch->getName());

				// Copy the copied patch entry over to this archive
				if (entry)
					tx_editor->getArchive()->addEntry(entry, "patches", true);
			}

			// If the entry exists in the base resource archive or this archive, do nothing
			else if (entry->getParent() == theArchiveManager->baseResourceArchive() ||
					entry->getParent() == tx_editor->getArchive())
				continue;

			// Otherwise, copy the entry over to this archive
			else
				tx_editor->getArchive()->addEntry(entry, "patches", true);
		}
	}

	// Refresh
	list_textures->updateList();

	// Update variables
	modified = true;
}

/* TextureXPanel::renameTexture
 * Create standalone image entries of any selected textures
 *******************************************************************/
void TextureXPanel::renameTexture() {
	// Get selected textures
	vector<long> selec_num = list_textures->getSelection();
	vector<CTexture*> selection;

	if (!tx_entry) return;

	saveTEXTUREX();

	Archive * archive = tx_entry->getParent();

	// Go through selection
	for (unsigned a = 0; a < selec_num.size(); ++a) {
		selection.push_back(texturex.getTexture(selec_num[a]));
	}

	// Check any are selected
	if (selection.size() == 1) {
		// If only one entry is selected, or "rename each" mode is desired, just do basic rename
		for (unsigned a = 0; a < selection.size(); a++) {

			// Prompt for a new name
			string new_name = wxGetTextFromUser("Enter new texture name: (* = unchanged)", "Rename", selection[a]->getName());
			if (wad_force_uppercase) new_name.MakeUpper();

			// Rename entry (if needed)
			if (!new_name.IsEmpty() && selection[a]->getName() != new_name)
				selection[a]->setName(new_name);
		}
	} else if (selection.size() > 1) {
		// Get a list of entry names
		wxArrayString names;
		for (unsigned a = 0; a < selection.size(); a++)
			names.push_back(selection[a]->getName());

		// Get filter string
		string filter = Misc::massRenameFilter(names);

		// Prompt for a new name
		string new_name = wxGetTextFromUser("Enter new texture name: (* = unchanged)", "Rename", filter);
		if (wad_force_uppercase) new_name.MakeUpper();

		// Apply mass rename to list of names
		if (!new_name.IsEmpty()) {
			Misc::doMassRename(names, new_name);

			// Go through the list
			for (size_t a = 0; a < selection.size(); a++) {
				// Rename the entry (if needed)
				if (selection[a]->getName() != names[a])
					selection[a]->setName(names[a]);		// Change name
			}
		}
	}
	Refresh();
}

/* TextureXPanel::exportTexture
 * Create standalone image entries of any selected textures
 *******************************************************************/
void TextureXPanel::exportTexture() {
	// Get selected textures
	vector<long> selec_num = list_textures->getSelection();
	vector<CTexture*> selection;

	if (!tx_entry) return;

	saveTEXTUREX();

	Archive * archive = tx_entry->getParent();
	bool force_rgba = texture_editor->getBlendRGBA();

	// Go through selection
	for (unsigned a = 0; a < selec_num.size(); ++a) {
		selection.push_back(texturex.getTexture(selec_num[a]));
	}

	// Create gfx conversion dialog
	GfxConvDialog gcd;

	// Send selection to the gcd
	gcd.openTextures(selection, texture_editor->getPalette(), archive, force_rgba);

	// Run the gcd
	gcd.ShowModal();

	// Show splash window
	theSplashWindow->show("Writing converted image data...", true);

	// Write any changes
	for (unsigned a = 0; a < selection.size(); a++) {
		// Update splash window
		theSplashWindow->setProgressMessage(selection[a]->getName());
		theSplashWindow->setProgress((float)a / (float)selection.size());

		// Skip if the image wasn't converted
		if (!gcd.itemModified(a))
			continue;

		// Get image and conversion info
		SImage* image = gcd.getItemImage(a);
		SIFormat* format = gcd.getItemFormat(a);
		
		// Write converted image back to entry
		MemChunk mc;
		format->saveImage(*image, mc, force_rgba ? NULL : gcd.getItemPalette(a));
		ArchiveEntry * lump = new ArchiveEntry;
		lump->importMemChunk(mc);
		lump->rename(selection[a]->getName());
		archive->addEntry(lump, "textures");
		EntryType::detectEntryType(lump);
		lump->setExtensionByType();
	}

	// Hide splash window
	theSplashWindow->hide();
}

/* TextureXPanel::handleAction
 * Handles the action [id]. Returns true if the action was handled,
 * false otherwise
 *******************************************************************/
bool TextureXPanel::handleAction(string id) {
	// Don't handle if hidden
	if (!tx_editor->IsShown() || !IsShown())
		return false;

	// Only interested in "txed_" events
	if (!id.StartsWith("txed_"))
		return false;

	// Handle action
	if (id == "txed_new")
		newTexture();
	else if (id == "txed_delete")
		removeTexture();
	else if (id == "txed_new_patch")
		newTextureFromPatch();
	else if (id == "txed_new_file")
		newTextureFromFile();
	else if (id == "txed_up")
		moveUp();
	else if (id == "txed_down")
		moveDown();
	else if (id == "txed_copy")
		copy();
	else if (id == "txed_cut") {
		copy();
		removeTexture();
	}
	else if (id == "txed_paste")
		paste();
	else if (id == "txed_export")
		exportTexture();
	else if (id == "txed_rename")
		renameTexture();
	else
		return false;	// Not handled here

	return true;
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
	texture_editor->openTexture(tex, &texturex);

	// Set current texture
	tex_current = tex;
}

/* TextureXPanel::onTextureListRightClick
 * Called when an item on the texture list is right clicked
 *******************************************************************/
void TextureXPanel::onTextureListRightClick(wxListEvent& e) {
	// Create context menu
	wxMenu context;
	theApp->getAction("txed_delete")->addToMenu(&context);
	context.AppendSeparator();
	theApp->getAction("txed_rename")->addToMenu(&context);
	theApp->getAction("txed_export")->addToMenu(&context);
	context.AppendSeparator();
	theApp->getAction("txed_copy")->addToMenu(&context);
	theApp->getAction("txed_cut")->addToMenu(&context);
	theApp->getAction("txed_paste")->addToMenu(&context);
	context.AppendSeparator();
	theApp->getAction("txed_up")->addToMenu(&context);
	theApp->getAction("txed_down")->addToMenu(&context);

	// Pop it up
	PopupMenu(&context);
}

/* TextureXPanel::onTextureListKeyDown
 * Called when a key is pressed in the texture list
 *******************************************************************/
void TextureXPanel::onTextureListKeyDown(wxKeyEvent& e) {
	// Move texture up (Ctrl+U or Ctrl+Up Arrow)
	if (e.GetModifiers() == wxMOD_CMD && (e.GetKeyCode() == 'U' || e.GetKeyCode() == WXK_UP))
		moveUp();

	// Move texture down (Ctrl+D or Ctrl+Down Arrow)
	else if (e.GetModifiers() == wxMOD_CMD && (e.GetKeyCode() == 'D' || e.GetKeyCode() == WXK_DOWN))
		moveDown();

	// Copy (Ctrl+C)
	else if (e.GetModifiers() == wxMOD_CMD && e.GetKeyCode() == 'C')
		copy();

	// Cut (Ctrl+X)
	else if (e.GetModifiers() == wxMOD_CMD && e.GetKeyCode() == 'X') {
		copy();
		removeTexture();
	}

	// Paste (Ctrl+V)
	else if (e.GetModifiers() == wxMOD_CMD && e.GetKeyCode() == 'V')
		paste();

	// New texture (Ctrl+N)
	else if (e.GetModifiers() == wxMOD_CMD && e.GetKeyCode() == 'N')
		newTexture();

	// New texture from patch (Ctrl+Shift+N)
	else if (e.GetModifiers() == (wxMOD_CMD|wxMOD_SHIFT) && e.GetKeyCode() == 'N')
		newTextureFromPatch();

	// New texture from file (Ctrl+Alt+N)
	else if (e.GetModifiers() == (wxMOD_CMD|wxMOD_ALT) && e.GetKeyCode() == 'N')
		newTextureFromFile();

	// Remove texture (Delete)
	else if (e.GetKeyCode() == WXK_DELETE)
		removeTexture();

	// Not handled here, send off to be handled by a parent window
	else
		e.Skip();
}

/* TextureXPanel::onBtnNewTexture
 * Called when the 'New Texture' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnNewTexture(wxCommandEvent& e) {
	newTexture();
}

/* TextureXPanel::onBtnNewTextureFromPatch
 * Called when the 'New Texture from Patch' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnNewTextureFromPatch(wxCommandEvent& e) {
	newTextureFromPatch();
}

/* TextureXPanel::onBtnNewTextureFromFile
 * Called when the 'New Texture from File' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnNewTextureFromFile(wxCommandEvent& e) {
	newTextureFromFile();
}

/* TextureXPanel::onBtnRemoveTexture
 * Called when the 'Remove Texture' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnRemoveTexture(wxCommandEvent& e) {
	removeTexture();
}

/* TextureXPanel::onBtnMoveUp
 * Called when the 'Move Up' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnMoveUp(wxCommandEvent& e) {
	moveUp();
}

/* TextureXPanel::onBtnMoveDown
 * Called when the 'Move Down' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnMoveDown(wxCommandEvent& e) {
	moveDown();
}

/* TextureXPanel::onBtnCopy
 * Called when the 'Copy' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnCopy(wxCommandEvent& e) {
	copy();
}

/* TextureXPanel::onBtnPaste
 * Called when the 'Paste' button is clicked
 *******************************************************************/
void TextureXPanel::onBtnPaste(wxCommandEvent& e) {
	paste();
}

/* TextureXPanel::onShow
 * Called when the panel is shown or hidden
 *******************************************************************/
void TextureXPanel::onShow(wxShowEvent& e) {
	tx_editor->showTextureMenu(e.IsShown());
}
