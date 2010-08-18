
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    DefaultEntryPanel.cpp
 * Description: DefaultEntryPanel class. Used for entries that don't
 *              have their own specific editor, or entries of an
 *              unknown type. Has the option to open/edit the entry
 *              as text.
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
#include "DefaultEntryPanel.h"
#include "Misc.h"
#include "GfxConvDialog.h"
#include "ModifyOffsetsDialog.h"
#include "EntryOperations.h"
#include "ArchiveManager.h"
#include "HexEditorPanel.h"


/*******************************************************************
 * DEFAULTENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* DefaultEntryPanel::DefaultEntryPanel
 * DefaultEntryPanel class constructor
 *******************************************************************/
DefaultEntryPanel::DefaultEntryPanel(wxWindow* parent)
: EntryPanel(parent, "default") {
	sizer_main->AddStretchSpacer(1);

	// Add type label
	label_type = new wxStaticText(this, -1, "Type");
	sizer_main->Add(label_type, 0, wxALL|wxALIGN_CENTER, 4);

	// Add size label
	label_size = new wxStaticText(this, -1, "Size");
	sizer_main->Add(label_size, 0, wxALL|wxALIGN_CENTER, 4);

	// Add actions frame
	frame_actions = new wxStaticBox(this, -1, "Actions");
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame_actions, wxVERTICAL);
	sizer_main->Add(framesizer, 0, wxALL|wxALIGN_CENTER, 4);

	// Add 'Convert Gfx' button
	btn_gfx_convert = new wxButton(this, -1, "Convert Gfx To...");
	framesizer->AddSpacer(4);
	framesizer->Add(btn_gfx_convert, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Add 'Modify Gfx Offsets' button
	btn_gfx_modify_offsets = new wxButton(this, -1, "Modify Gfx Offsets");
	framesizer->Add(btn_gfx_modify_offsets, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Add 'Edit Textures' button
	btn_texture_edit = new wxButton(this, -1, "Edit Textures");
	framesizer->Add(btn_texture_edit, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	sizer_main->AddStretchSpacer(1);

	// Hide default buttons
	btn_save->Enable(false);
	btn_revert->Enable(false);

	// Add 'Edit as Text' button
	btn_edit_text = new wxButton(this, -1, "Edit as Text");
	sizer_bottom->AddStretchSpacer(1);
	sizer_bottom->Add(btn_edit_text, 0, wxEXPAND, 0);

	// Add 'View as Hex' button
	btn_view_hex = new wxButton(this, -1, "View as Hex");
	sizer_bottom->Add(btn_view_hex, 0, wxEXPAND|wxLEFT, 4);

	// Bind events
	btn_gfx_convert->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DefaultEntryPanel::onBtnGfxConvert, this);
	btn_gfx_modify_offsets->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DefaultEntryPanel::onBtnGfxModifyOffsets, this);
	btn_texture_edit->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DefaultEntryPanel::onBtnTextureEdit, this);
	//btn_view_hex->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DefaultEntryPanel::onBtnViewHex, this);

	Layout();
}

DefaultEntryPanel::~DefaultEntryPanel() {
}

bool DefaultEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Update labels
	label_type->SetLabel(s_fmt("Entry Type: %s", chr(entry->getTypeString())));
	label_size->SetLabel(s_fmt("Entry Size: %d bytes", entry->getSize()));

	// Show edit buttons
	btn_edit_text->Show(true);

	// Setup actions frame
	btn_gfx_convert->Show(false);
	btn_gfx_modify_offsets->Show(false);
	btn_texture_edit->Show(false);
	frame_actions->Show(false);

	// Check for gfx entry
	if (entry->getType()->extraProps().propertyExists("image")) {
		frame_actions->Show(true);
		btn_gfx_convert->Show(true);
		btn_gfx_modify_offsets->Show(true);
	}

	// Check for TEXTUREx related entry
	if (entry->getType()->getId() == "texturex" || entry->getType()->getId() == "pnames") {
		frame_actions->Show(true);
		btn_texture_edit->Show(true);
	}

	// Update layout
	Layout();

	return true;
}

bool DefaultEntryPanel::loadEntries(vector<ArchiveEntry*>& entries) {
	// Update labels
	label_type->SetLabel(s_fmt("%d selected entries", entries.size()));
	unsigned size = 0;
	for (unsigned a = 0; a < entries.size(); a++)
		size += entries[a]->getSize();
	label_size->SetLabel(s_fmt("Total Size: %s", chr(Misc::sizeAsString(size))));

	// Hide edit buttons
	btn_edit_text->Show(false);

	// Setup actions frame
	btn_gfx_convert->Show(false);
	btn_gfx_modify_offsets->Show(false);
	btn_texture_edit->Show(false);
	frame_actions->Show(false);

	bool gfx = false;
	bool texture = false;
	this->entries.clear();
	for (unsigned a = 0; a < entries.size(); a++) {
		// Check for gfx entry
		if (entries[a]->getType()->extraProps().propertyExists("image"))
			gfx = true;

		// Check for TEXTUREx related entry
		if (entries[a]->getType()->getId() == "texturex" || entries[a]->getType()->getId() == "pnames")
			texture = true;

		this->entries.push_back(entries[a]);
	}
	if (gfx) {
		frame_actions->Show(true);
		btn_gfx_convert->Show(true);
		btn_gfx_modify_offsets->Show(true);
	}
	if (texture) {
		frame_actions->Show(true);
		btn_texture_edit->Show(true);
	}

	// Update layout
	Layout();

	return true;
}

bool DefaultEntryPanel::saveEntry() {
	return true;
}


void DefaultEntryPanel::onBtnGfxConvert(wxCommandEvent& e) {
	// Create gfx conversion dialog
	GfxConvDialog gcd;

	// Send entries to the gcd
	gcd.openEntries(entries);

	// Run the gcd
	gcd.ShowModal();
}

void DefaultEntryPanel::onBtnGfxModifyOffsets(wxCommandEvent& e) {
// Create and run modify offsets dialog
	ModifyOffsetsDialog mod;
	if (mod.ShowModal() == wxID_CANCEL)
		return;

	// Apply offsets to selected entries
	for (uint32_t a = 0; a < entries.size(); a++)
		EntryOperations::modifyGfxOffsets(entries[a], mod.getAlignType(), mod.getOffset(),
											mod.xOffChange(), mod.yOffChange(), mod.relativeOffset());
}

void DefaultEntryPanel::onBtnTextureEdit(wxCommandEvent& e) {
	theArchiveManager->openTextureEditor(theArchiveManager->archiveIndex(entry->getParent()));
}

/*
void DefaultEntryPanel::onBtnViewHex(wxCommandEvent& e) {
	wxDialog dlg(NULL, -1, "Hex Editor (temp dialog)", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	dlg.SetSizer(hbox);

	HexEditorPanel* hex = new HexEditorPanel(&dlg);
	hbox->Add(hex, 1, wxEXPAND|wxALL, 4);

	if (entry)
		hex->loadData(entry->getMCData());

	dlg.Layout();
	dlg.SetInitialSize(wxSize(800, 500));
	dlg.ShowModal();
}
*/
