
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MultiEntryPanel.cpp
 * Description: This UI panel is shown if multiple entries are
 *              selected in the entry list. Shows the selection size,
 *              and some batch 'actions' that can be performed on the
 *              selected entries.
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
#include "MultiEntryPanel.h"
#include "Misc.h"
#include "WadArchive.h"
#include "GfxConvDialog.h"
#include "ModifyOffsetsDialog.h"
#include "EntryOperations.h"


/*******************************************************************
 * MULTIENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* MultiEntryPanel::MultiEntryPanel
 * MultiEntryPanel class constructor
 *******************************************************************/
MultiEntryPanel::MultiEntryPanel(wxWindow* parent)
: EntryPanel(parent, "multi") {
	// Init variables
	gfx_selected = false;

	// Init widgets
	label_entries = new wxStaticText(this, -1, "0 selected entries");
	label_size = new wxStaticText(this, -1, "Total size: 0");
	btn_export_archive = new wxButton(this, -1, "Export as Wad");
	btn_convert_gfx = new wxButton(this, -1, "Convert Gfx to...");
	btn_modify_offsets = new wxButton(this, -1, "Modify Gfx Offsets");
	btn_add_trns_chunk = new wxButton(this, -1, "Add tRNS chunks to PNG images");

	// Bind Events
	btn_export_archive->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MultiEntryPanel::onBtnExportArchive, this);
	btn_convert_gfx->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MultiEntryPanel::onBtnConvertGfx, this);
	btn_modify_offsets->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MultiEntryPanel::onBtnModifyOffsets, this);
	btn_add_trns_chunk->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MultiEntryPanel::onBtnAddtRNSChunks, this);

	// Update panel layout
	updateLayout();

	// Disable default entry buttons
	btn_save->Enable(false);
	btn_revert->Enable(false);
	btn_edit_ext->Enable(false);
}

/* MultiEntryPanel::~MultiEntryPanel
 * MultiEntryPanel class destructor
 *******************************************************************/
MultiEntryPanel::~MultiEntryPanel() {
}

/* MultiEntryPanel::loadEntries
 * Loads the given entries into the panel
 *******************************************************************/
bool MultiEntryPanel::loadEntries(vector<ArchiveEntry*>& list) {
	uint32_t total_size = 0;
	gfx_selected = false;
	png_selected = false;

	// Clear current list of entries
	entries.clear();

	// Get selected entries
	for (size_t a = 0; a < list.size(); a++) {
		entries.push_back(list[a]);

		// Add entry size to total
		total_size += list[a]->getSize();

		// Check for gfx entry
		if (!gfx_selected) {
			if (list[a]->getType()->getEditor() == "gfx")
				gfx_selected = true;
		}
		// Check for png entry
		if (!png_selected) {
			if (list[a]->getType()->getFormat() == "img_png")
				png_selected = true;
		}
	}

	// Update labels
	label_entries->SetLabel(s_fmt("%d selected entries", entries.size()));
	label_size->SetLabel(s_fmt("Total size: %s", Misc::sizeAsString(total_size).c_str()));

	// Update panel layout
	updateLayout();

	return true;
}

/* MultiEntryPanel::updateLayout
 * Updates the panel layout, including hiding/showing different
 * action buttons depending on what is selected
 *******************************************************************/
void MultiEntryPanel::updateLayout() {
	// Clear current layout
	sizer_main->Clear();

	// Setup layout
	sizer_main->AddStretchSpacer();

	// Count/size labels
	sizer_main->Add(label_entries, 0, wxALIGN_CENTER|wxALL, 4);
	sizer_main->Add(label_size, 0, wxALIGN_CENTER|wxALL, 4);
	sizer_main->AddSpacer(8);

	// Actions frame
	wxStaticBox* frame_actions = new wxStaticBox(this, -1, "Actions");
	wxStaticBoxSizer* frame_sizer = new wxStaticBoxSizer(frame_actions, wxVERTICAL);
	frame_sizer->Add(btn_export_archive, 0, wxEXPAND|wxALL, 4);

	// Check if any gfx entries are selected
	if (gfx_selected) {
		btn_convert_gfx->Show(true);
		btn_modify_offsets->Show(true);
		frame_sizer->Add(btn_convert_gfx, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
		frame_sizer->Add(btn_modify_offsets, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	}
	else {
		btn_convert_gfx->Show(false);
		btn_modify_offsets->Show(false);
	}

	// Check if any png entries are selected
	if (png_selected) {
		btn_add_trns_chunk->Show(true);
		frame_sizer->Add(btn_add_trns_chunk, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	}
	else
		btn_add_trns_chunk->Show(false);

	sizer_main->Add(frame_sizer, 0, wxALIGN_CENTER|wxALL, 4);

	sizer_main->AddStretchSpacer();


	// Apply layout
	Layout();
}


/*******************************************************************
 * MULTIENTRYPANEL EVENTS
 *******************************************************************/

/* MultiEntryPanel::btnExportArchiveClicked
 * Called when the 'Export as Wad' button is clicked
 *******************************************************************/
void MultiEntryPanel::onBtnExportArchive(wxCommandEvent& e) {
	// Create save file dialog
	wxFileDialog dialog_save(this, "", wxEmptyString, wxEmptyString,
								"Doom Wad File (*.wad)|*.wad", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_save.ShowModal() == wxID_OK) {
		// If a filename was selected, export the selected entries to a wad archive
		WadArchive::exportEntriesAsWad(dialog_save.GetPath(), entries);
	}
}

/* MultiEntryPanel::btnConvertGfxClicked
 * Called when the 'Convert Gfx to...' button is clicked
 *******************************************************************/
void MultiEntryPanel::onBtnConvertGfx(wxCommandEvent& e) {
	// Create gfx conversion dialog
	GfxConvDialog gcd;

	// Send entries to the gcd
	gcd.openEntries(entries);

	// Run the gcd
	gcd.ShowModal();
}

/* MultiEntryPanel::btnAddtRNSChunksClicked
 * Called when the 'Add tRNS chunks to PNG images' button is clicked
 *******************************************************************/
void MultiEntryPanel::onBtnAddtRNSChunks(wxCommandEvent& e) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (EntryOperations::modifytRNSChunk(entries[a], true))
			entries[a]->setState(1);
	}
}

/* MultiEntryPanel::btnModifyOffsetsClicked
 * Called when the 'Modify Gfx Offsets' button is clicked
 *******************************************************************/
void MultiEntryPanel::onBtnModifyOffsets(wxCommandEvent& e) {
	// Create and run modify offsets dialog
	ModifyOffsetsDialog mod;
	if (mod.ShowModal() == wxID_CANCEL)
		return;

	// Apply offsets to selected entries
	for (uint32_t a = 0; a < entries.size(); a++)
		EntryOperations::modifyGfxOffsets(entries[a], mod.getAlignType(), mod.getOffset(), mod.xOffChange(), mod.yOffChange(), mod.relativeOffset());
}
