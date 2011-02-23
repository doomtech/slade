
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
#include "MainWindow.h"
#include "WxStuff.h"
#include "TextureXEditor.h"
#include "ArchiveManager.h"
#include "Console.h"
#include "SplashWindow.h"
#include "ExtMessageDialog.h"
#include <wx/dialog.h>
#include <wx/radiobut.h>


/*******************************************************************
 * CREATETEXTUREXDIALOG CLASS
 *******************************************************************
 A simple dialog that provides options to create new (empty) texture
 definitions or import them from the base resource archive. Also has
 options for the desired texture format (doom, strife or TEXTURES)
 */
class CreateTextureXDialog : public wxDialog {
private:
	wxRadioButton*	rb_format_doom;
	wxRadioButton*	rb_format_strife;
	wxRadioButton*	rb_format_textures;

	wxRadioButton*	rb_new;
	wxRadioButton*	rb_import_bra;
	wxRadioButton*	rb_import_archive;

public:
	CreateTextureXDialog(wxWindow* parent) : wxDialog(parent, -1, "Create Texture Definitions") {
		// Setup layout
		wxBoxSizer* m_vbox = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_vbox);

		// --- Format options ---
		wxStaticBox* frame = new wxStaticBox(this, -1, "Format");
		wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
		m_vbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);

		// Doom format
		rb_format_doom = new wxRadioButton(this, -1, "Doom (TEXTURE1 + PNAMES)", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		framesizer->Add(rb_format_doom, 0, wxEXPAND|wxALL, 4);

		// Strife format
		rb_format_strife = new wxRadioButton(this, -1, "Strife (TEXTURE1 + PNAMES)");
		framesizer->Add(rb_format_strife, 0, wxEXPAND|wxALL, 4);

		// ZDoom TEXTURES format
		rb_format_textures = new wxRadioButton(this, -1, "ZDoom (TEXTURES)");
		framesizer->Add(rb_format_textures, 0, wxEXPAND|wxALL, 4);


		// --- Source options ---
		frame = new wxStaticBox(this, -1, "Source");
		framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
		m_vbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);

		// New list
		rb_new = new wxRadioButton(this, -1, "Create New (Empty)", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		framesizer->Add(rb_new, 0, wxEXPAND|wxALL, 4);

		// Import from Base Resource Archive
		rb_import_bra = new wxRadioButton(this, -1, "Import from Base Resource Archive:");
		framesizer->Add(rb_import_bra, 0, wxEXPAND|wxALL, 4);

		// Add buttons
		m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

		// Bind events
		rb_new->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &CreateTextureXDialog::onRadioNewSelected, this);
		rb_import_bra->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &CreateTextureXDialog::onRadioNewSelected, this);

		SetInitialSize(wxSize(-1, -1));
		Layout();
	}

	~CreateTextureXDialog() {}

	int	getSelectedFormat() {
		if (rb_format_doom->GetValue())
			return TXF_NORMAL;
		else if (rb_format_strife->GetValue())
			return TXF_STRIFE11;
		else if (rb_format_textures->GetValue())
			return TXF_TEXTURES;
		else
			return -1;
	}

	bool createNewSelected() {
		return rb_new->GetValue();
	}

	void onRadioNewSelected(wxCommandEvent& e) {
		// Enable/Disable format selection depending on rb_new state
		if (rb_new->GetValue()) {
			rb_format_doom->Enable(true);
			rb_format_strife->Enable(true);
			rb_format_textures->Enable(true);
		}
		else {
			rb_format_doom->Enable(false);
			rb_format_strife->Enable(false);
			rb_format_textures->Enable(false);
		}
	}
};


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

	// Create patch browser
	patch_browser = new PatchBrowser(this);
	patch_browser->Show(false);

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Add tabs
	tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP|wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|wxNO_BORDER);
	sizer->Add(tabs, 1, wxEXPAND|wxALL, 4);

	// Bottom bar
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

	// Add save changes button
	btn_save = new wxButton(this, -1, "Save Changes");
	hbox->AddStretchSpacer();
	hbox->Add(btn_save, 0, wxEXPAND|wxALL, 4);

	// Bind events
	btn_save->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureXEditor::onSaveClicked, this);

	// Palette chooser
	listenTo(theMainWindow->getPaletteChooser());
	updateTexturePalette();

	// Listen to patch table
	listenTo(&patch_table);

	// Update+ layout
	Layout();
	Show();
}

/* TextureXEditor::~TextureXEditor
 * TextureXEditor class destructor
 *******************************************************************/
TextureXEditor::~TextureXEditor() {
	if (pnames)
		pnames->unlock();
}

/* TextureXEditor::openArchive
 * Opens an archive to manage. Opens all texture related entries in
 * the archive and sorts a bunch of stuff out. If no texture related
 * entries exist in the archive, give the user options to create or
 * import them
 *******************************************************************/
bool TextureXEditor::openArchive(Archive* archive) {
	// Check any archive was given
	if (!archive)
		return false;

	// Setup archive texture entries
	if (!setupTextureEntries(archive))
		return false;

	// Search archive for any texture-related entries
	Archive::search_options_t options;
	options.match_type = EntryType::getType("texturex");
	vector<ArchiveEntry*> tx_entries = archive->findAll(options);	// Find all TEXTUREx entries
	options.match_type = EntryType::getType("pnames");
	ArchiveEntry* entry_pnames = archive->findLast(options);		// Find last PNAMES entry

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
		TextureXPanel* tx_panel = new TextureXPanel(this, this);

		// Init texture panel
		tx_panel->Show(false);

		// Open TEXTUREX entry
		if (tx_panel->openTEXTUREX(tx_entries[a])) {
			// Set palette
			tx_panel->setPalette(theMainWindow->getPaletteChooser()->getSelectedPalette());
			// Lock entry
			tx_entries[a]->lock();

			// Add it to the list of editors, and a tab
			texture_editors.push_back(tx_panel);
			tabs->AddPage(tx_panel, tx_entries[a]->getName());
		}

		tx_panel->Show(true);
	}

	// Open patch table tab if needed
	if (pnames) {
		PatchTablePanel* ptp = new PatchTablePanel(this, &patch_table);
		tabs->AddPage(ptp, "Patch Table (PNAMES)");
	}

	// Search archive for TEXTURES entries
	options.match_type = EntryType::getType("zdtextures");
	vector<ArchiveEntry*> ztx_entries = archive->findAll(options);

	// Open texture editor tabs
	for (unsigned a = 0; a < ztx_entries.size(); a++) {
		TextureXPanel* tx_panel = new TextureXPanel(this, this);

		// Init texture panel
		tx_panel->Show(false);

		// Open TEXTURES entry
		if (tx_panel->openTEXTUREX(ztx_entries[a])) {
			// Set palette
			tx_panel->setPalette(theMainWindow->getPaletteChooser()->getSelectedPalette());
			// Lock entry
			ztx_entries[a]->lock();

			// Add it to the list of editors, and a tab
			texture_editors.push_back(tx_panel);
			tabs->AddPage(tx_panel, ztx_entries[a]->getName());
		}

		tx_panel->Show(true);
	}

	// Update layout
	Layout();

	// Update variables
	this->archive = archive;

	// Lock pnames entry if it exists
	if (pnames)
		pnames->lock();

	// Set global palette
	theMainWindow->getPaletteChooser()->setGlobalFromArchive(archive);

	// Find patch entries (this really needs to be faster)
	theSplashWindow->show("Opening Texture List...", true);
	theSplashWindow->setProgressMessage("Searching for patches");
	float np = (float)patch_table.nPatches();
	for (unsigned a = 0; a < patch_table.nPatches(); a++) {
		theSplashWindow->setProgress((float)a / np);
		patch_table.updatePatchEntry(a);
	}
	theSplashWindow->hide();

	// Setup patch browser
	patch_browser->openPatchTable(&patch_table);

	return true;
}

/* TextureXEditor::removePatch
 * Removes the patch at [index] on the patch table from any textures
 * that contain it (and from the patch table itself)
 *******************************************************************/
bool TextureXEditor::removePatch(unsigned index, bool delete_entry) {
	// Get patch we're removing
	patch_t& p = patch_table.patch(index);

	// Update TEXTUREx lists
	for (unsigned a = 0; a < texture_editors.size(); a++)
		texture_editors[a]->txList().removePatch(p.name);

	// Delete patch entry if it's part of this archive (and delete_entry is true)
	if (delete_entry && p.entry && p.entry->getParent() == archive)
		archive->removeEntry(p.entry);

	// Remove patch from patch table
	patch_table.removePatch(index);

	return true;
}

/* TextureXEditor::browsePatch
 * Opens the patch browser. Returns the selected patch index, or -1
 * if no patch was selected
 *******************************************************************/
int TextureXEditor::browsePatch() {
	patch_browser->updateItems();

	if (patch_browser->ShowModal() == wxID_OK)
		return patch_browser->getSelectedPatch();
	else
		return -1;
}

/* TextureXEditor::checkTextures
 * Checks all texture definitions for problems and alerts the user
 * if any are found. Returns true if any problems were found, false
 * otherwise
 *******************************************************************/
bool TextureXEditor::checkTextures() {
	string problems = wxEmptyString;

	// Go through all texturex lists
	for (unsigned a = 0; a < texture_editors.size(); a++) {
		// Go through all textures
		for (unsigned t = 0; t < texture_editors[a]->txList().nTextures(); t++) {
			// Get texture
			CTexture* tex = texture_editors[a]->txList().getTexture(t);

			// Check it's patches are all valid
			for (unsigned p = 0; p < tex->nPatches(); p++) {
				if (patch_table.patchIndex(tex->getPatch(p)->getName()) == -1) {
					problems += s_fmt("Texture %s contains invalid/unknown patch %s\n", chr(tex->getName()), chr(tex->getPatch(p)->getName()));
				}
			}
		}
	}

	// Go through patch table
	for (unsigned a = 0; a < patch_table.nPatches(); a++) {
		// Check patch entry is valid
		patch_table.updatePatchEntry(a);
		patch_t& patch = patch_table.patch(a);

		if (!patch.entry) {
			problems += s_fmt("Patch %s cannot be found in any open archive\n", chr(patch.name));
		}
		else {
			// Check patch entry type
			if (patch.entry->getType() == EntryType::unknownType())
				EntryType::detectEntryType(patch.entry);
			EntryType* type = patch.entry->getType();

			if (!type->extraProps().propertyExists("patch"))
				problems += s_fmt("Patch %s is of type \"%s\", which is not a valid gfx format for patches. Convert it to either Doom Gfx or PNG\n", chr(patch.name), chr(type->getName()));
		}
	}

	// Display a message box with any problems found
	if (!problems.IsEmpty()) {
		ExtMessageDialog dlg(this, "Problems Found");
		dlg.setMessage("The following problems were found:");
		dlg.setExt(problems);
		dlg.ShowModal();

		return true;
	}
	else
		return false;
}


/*******************************************************************
 * TEXTUREXEDITOR EVENTS
 *******************************************************************/

/* TextureXEditor::updateTexturePalette
 * Sets the texture panels' palettes to what is selected in the
 * palette chooser
 *******************************************************************/
void TextureXEditor::updateTexturePalette() {
	// Get palette
	Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();

	// Send to whatever needs it
	for (size_t a = 0; a < texture_editors.size(); a++)
		texture_editors[a]->setPalette(pal);
}

/* TextureXEditor::onAnnouncement
 * Handles any announcements from the current texture
 *******************************************************************/
void TextureXEditor::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	if (announcer == theMainWindow->getPaletteChooser() && event_name == "main_palette_changed") {
		updateTexturePalette();
	}

	if (announcer == &patch_table && event_name == "modified") {
		patch_browser->openPatchTable(&patch_table);
	}
}

/* TextureXEditor::onSaveClicked
 * Called when the 'Save Changes' button is clicked
 *******************************************************************/
void TextureXEditor::onSaveClicked(wxCommandEvent& e) {
	// Check for problems
	checkTextures();

	// Save TEXTUREx entries
	for (unsigned a = 0; a < texture_editors.size(); a++)
		texture_editors[a]->saveTEXTUREX();

	// Save PNAMES if it exists
	if (patch_table.nPatches() > 0) {
		if (!pnames) {
			// If no PNAMES entry exists in the archive, create one
			int index = archive->entryIndex(texture_editors.back()->txEntry()) + 1;
			pnames = archive->addNewEntry("PNAMES", index);
			pnames->setType(EntryType::getType("pnames"));
			pnames->setExtensionByType();
		}

		pnames->unlock();	// Have to unlock it to write
		patch_table.writePNAMES(pnames);
		pnames->lock();
	}
}


/* TextureXEditor::setupTextureEntries
 * Static function to check if an archive has sufficient texture
 * related entries, and if not, prompts the user to either create
 * or import them. Returns true if the entries exist, false otherwise
 *******************************************************************/
bool TextureXEditor::setupTextureEntries(Archive* archive) {
	// Check any archive was given
	if (!archive)
		return false;

	// Search archive for any ZDoom TEXTURES entries
	Archive::search_options_t options;
	options.match_type = EntryType::getType("zdtextures");
	ArchiveEntry* entry_tx = archive->findFirst(options);	// Find any TEXTURES entry

	// If it's found, we're done
	if (entry_tx)
		return true;


	// Search archive for any texture-related entries
	options.match_type = EntryType::getType("texturex");
	entry_tx = archive->findFirst(options);						// Find any TEXTUREx entry
	options.match_type = EntryType::getType("pnames");
	ArchiveEntry* entry_pnames = archive->findFirst(options);	// Find any PNAMES entry

	// If both exist, we're done
	if (entry_tx && entry_pnames)
		return true;

	// If no TEXTUREx entry exists
	if (!entry_tx) {
		// No TEXTUREx entries found, so ask if the user wishes to create one
		wxMessageDialog dlg(NULL, "The archive does not contain any texture definitions (TEXTURE1/2 or TEXTURES). Do you wish to create or import a texture definition list?", "No Texture Definitions Found", wxYES_NO);

		if (dlg.ShowModal() == wxID_YES) {
			CreateTextureXDialog ctxd(NULL);

			while (1) {
				// Check if cancelled
				if (ctxd.ShowModal() == wxID_CANCEL)
					return false;

				if (ctxd.createNewSelected()) {
					// User selected to create a new TEXTUREx list
					ArchiveEntry* texturex = NULL;

					int format = ctxd.getSelectedFormat();
					if (ctxd.getSelectedFormat() == TXF_NORMAL || ctxd.getSelectedFormat() == TXF_STRIFE11) {	// Doom or Strife TEXTUREx
						// Create texture list
						TextureXList txlist;
						txlist.setFormat(ctxd.getSelectedFormat());

						// Create patch table
						PatchTable ptt;

						// Create dummy patch
						ArchiveEntry* dpatch = theArchiveManager->programResourceArchive()->entryAtPath("s3dummy.lmp");
						dpatch = archive->addEntry(dpatch, "patches", true);
						ptt.addPatch("S3DUMMY");

						// Create dummy texture
						CTexture* dummytex = new CTexture();
						dummytex->setName("S3DUMMY");
						dummytex->addPatch("S3DUMMY", 0, 0, dpatch);
						dummytex->setWidth(128);
						dummytex->setHeight(128);
						dummytex->setScale(0, 0);

						// Add dummy texture to list
						// (this serves two purposes - supplies the special 'invalid' texture by default,
						//   and allows the texturex format to be detected)
						txlist.addTexture(dummytex);

						// Add empty PNAMES entry to archive
						entry_pnames = archive->addNewEntry("PNAMES");
						ptt.writePNAMES(entry_pnames);
						entry_pnames->setType(EntryType::getType("pnames"));
						entry_pnames->setExtensionByType();

						// Add empty TEXTURE1 entry to archive
						texturex = archive->addNewEntry("TEXTURE1");
						txlist.writeTEXTUREXData(texturex, ptt);
						texturex->setType(EntryType::getType("texturex"));
						texturex->setExtensionByType();
					}
					else if (ctxd.getSelectedFormat() == TXF_TEXTURES) {
						// Create texture list
						TextureXList txlist;
						txlist.setFormat(TXF_TEXTURES);

						// Add empty TEXTURES entry to archive
						texturex = archive->addNewEntry("TEXTURES");
						texturex->setType(EntryType::getType("zdtextures"));
						texturex->setExtensionByType();

						return false;
					}

					if (!texturex)
						return false;
				}
				else {
					// User selected to import texture definitions from the base resource archive
					Archive* bra = theArchiveManager->baseResourceArchive();

					if (!bra) {
						wxMessageBox("No Base Resource Archive is opened, please select/open one", "Error", wxICON_ERROR);
						continue;
					}

					// Find all relevant entries in the base resource archive
					Archive::search_options_t options;
					options.match_type = EntryType::getType("texturex");
					vector<ArchiveEntry*> import_tx = bra->findAll(options);	// Find all TEXTUREx entries
					options.match_type = EntryType::getType("pnames");
					ArchiveEntry* import_pnames = bra->findLast(options);		// Find last PNAMES entry

					// Check enough entries exist
					if (import_tx.size() == 0 || !import_pnames) {
						wxMessageBox("The selected Base Resource Archive does not contain sufficient texture definition entries", "Error", wxICON_ERROR);
						continue;
					}

					// Copy TEXTUREx entries over to current archive
					for (unsigned a = 0; a < import_tx.size(); a++) {
						ArchiveEntry* texturex = archive->addEntry(import_tx[a], "global", true);
						texturex->setType(EntryType::getType("texturex"));
						texturex->setExtensionByType();
					}

					// Copy PNAMES entry over to current archive
					entry_pnames = archive->addEntry(import_pnames, "global", true);
					entry_pnames->setType(EntryType::getType("pnames"));
					entry_pnames->setExtensionByType();
				}

				break;
			}

			return true;
		}

		// 'No' clicked
		return false;
	}
	else {	// TEXTUREx entry exists
		// TODO: Probably a better idea here to get the user to select an archive to import the patch table from
		// If no PNAMES entry was found, search resource archives
		if (!entry_pnames) {
			Archive::search_options_t options;
			options.match_type = EntryType::getType("pnames");
			entry_pnames = theArchiveManager->findResourceEntry(options, archive);
		}

		// If no PNAMES entry is found at all, show an error and abort
		// TODO: ask user to select appropriate base resource archive
		if (!entry_pnames) {
			wxMessageBox("PNAMES entry not found!", wxMessageBoxCaptionStr, wxICON_ERROR);
			return false;
		}

		return true;
	}

	return false;
}
