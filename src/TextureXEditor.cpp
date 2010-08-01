
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextureXEditor.cpp
 * Description: The UI for managing/viewing/editing all composite
 *              textures in an archive.
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
#include "TextureXEditor.h"
#include "ArchiveManager.h"
#include "Console.h"


/*******************************************************************
 * TEXTUREXEDITOR CLASS FUNCTIONS
 *******************************************************************/

/* TextureXEditor::TextureXEditor
 * TextureXEditor class constructor
 *******************************************************************/
TextureXEditor::TextureXEditor(wxWindow* parent) : wxPanel(parent, -1) {
	// Init variables
	this->archive = archive;
	this->pnames = NULL;

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Add tabs
	tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP|wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|wxNO_BORDER);
	sizer->Add(tabs, 1, wxEXPAND|wxALL, 4);

	// Bottom bar
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

	// Add palette chooser
	pal_chooser = new PaletteChooser(this, -1);
	hbox->Add(new wxStaticText(this, -1, "Palette:"), 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
	hbox->Add(pal_chooser, 0, wxALIGN_CENTER|wxALL, 4);

	// Add save changes button
	btn_save = new wxButton(this, -1, "Save Changes");
	hbox->AddStretchSpacer();
	hbox->Add(btn_save, 0, wxEXPAND|wxALL, 4);

	// Bind events
	pal_chooser->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &TextureXEditor::onPaletteChanged, this);
	btn_save->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXEditor::onSaveClicked, this);

	// Update+ layout
	Layout();
	Show();
}

/* TextureXEditor::~TextureXEditor
 * TextureXEditor class destructor
 *******************************************************************/
TextureXEditor::~TextureXEditor() {
	// FIXME: Can cause crash if archive is closed before this
	if (pnames)
		pnames->unlock();
}

/* TextureXEditor::openArchive
 * Opens an archive to manage. Opens all texture related entries in
 * the archive and sorts a bunch of stuff out
 *******************************************************************/
bool TextureXEditor::openArchive(Archive* archive) {
	// Check any archive was given
	if (!archive)
		return false;

	// Set global palette
	pal_chooser->setGlobalFromArchive(archive);

	// Search archive for any texture-related entries
	Archive::search_options_t options;
	options.match_type = EntryType::getType("texturex");
	vector<ArchiveEntry*> tx_entries = archive->findAll(options);
	options.match_type = EntryType::getType("pnames");
	ArchiveEntry* entry_pnames = archive->findLast(options);

	// If any TEXTURE1/2 entries were found, setup patch table stuff
	if (tx_entries.size() > 0) {
		// TODO: Probably a better idea here to get the user to select an archive to import the patch table from
		// If no PNAMES entry was found, search resource archives
		if (!entry_pnames) {
			Archive::search_options_t options;
			options.match_type = EntryType::getType("pnames");
			entry_pnames = theArchiveManager->findResourceEntry(options, archive);
		}
		else
			pnames = entry_pnames;	// If PNAMES was found in the archive, set the class variable so it is written to if modified

		// If no PNAMES entry is found at all, show an error and abort
		// TODO: ask user to select appropriate base resource archive
		if (!entry_pnames) {
			wxMessageBox("PNAMES entry not found!", wxMessageBoxCaptionStr, wxICON_ERROR);
			return false;
		}

		// Load patch table
		patch_table.loadPNAMES(entry_pnames, archive);
	}

	// Open texture editor tabs
	for (size_t a = 0; a < tx_entries.size(); a++) {
		TextureXPanel* tx_panel = new TextureXPanel(this, &patch_table);

		// Init texture panel
		tx_panel->Show(false);

		// Open TEXTUREX entry
		if (tx_panel->openTEXTUREX(tx_entries[a])) {
			//tx_panel->populateTextureList();							// Populate texture list
			tx_panel->setPalette(pal_chooser->getSelectedPalette());	// Set palette
			tx_entries[a]->lock();										// Lock entry

			// Add it to the list of editors, and a tab
			texture_editors.push_back(tx_panel);
			tabs->AddPage(tx_panel, tx_entries[a]->getName());
		}

		tx_panel->Show(true);
	}

	// Open patch table tab if needed
	if (entry_pnames) {
		PatchTablePanel* ptp = new PatchTablePanel(this, &patch_table);
		tabs->AddPage(ptp, "Patch Table (PNAMES)");
	}

	// Update layout
	Layout();

	// Update variables
	this->archive = archive;

	// Lock pnames entry if it exists
	if (pnames)
		pnames->lock();

	return true;
}

bool TextureXEditor::removePatch(unsigned index) {
	// Update TEXTUREx lists
	for (unsigned a = 0; a < texture_editors.size(); a++)
		texture_editors[a]->txList().removePatch(patch_table.patchName(index));

	// Remove patch from patch table
	patch_table.removePatch(index);

	return true;
}


/*******************************************************************
 * TEXTUREXEDITOR EVENTS
 *******************************************************************/

/* TextureXEditor::onPaletteChanged
 * Called when a palette is selected in the palette chooser
 *******************************************************************/
void TextureXEditor::onPaletteChanged(wxCommandEvent& e) {
	// Get palette
	Palette8bit* pal = pal_chooser->getSelectedPalette();

	// Send to whatever needs it
	for (size_t a = 0; a < texture_editors.size(); a++)
		texture_editors[a]->setPalette(pal);
}

void TextureXEditor::onSaveClicked(wxCommandEvent& e) {

}





/*******************************************************************
 * CONSOLE COMMANDS
 *******************************************************************/

// Command to test the texture editor
/*
void c_text_txeditor(vector<string> args) {
	wxDialog dialog(NULL, -1, "Texture Editor!", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
	TextureXEditor txed(&dialog);
	if (!txed.openArchive(theArchiveManager->getArchive(0))) {
		wxLogMessage("No archive open");
		return;
	}

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(&txed, 1, wxEXPAND|wxALL, 4);

	dialog.SetSizer(sizer);
	dialog.Layout();
	dialog.SetInitialSize(wxSize(1024, 768));
	dialog.ShowModal();
}
ConsoleCommand test_txeditor("test_txeditor", &c_text_txeditor, 0);
*/
