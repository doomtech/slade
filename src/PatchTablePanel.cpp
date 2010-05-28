
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PatchTablePanel.cpp
 * Description: The UI for viewing/editing a patch table (PNAMES)
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
#include "PatchTablePanel.h"
#include "Archive.h"
#include "TextureXEditor.h"


PatchTableListView::PatchTableListView(wxWindow* parent, PatchTable* patch_table) : VirtualListView(parent) {
	// Init Variables
	this->patch_table = patch_table;

	// Add columns
	InsertColumn(0, "#");
	InsertColumn(1, "Patch Name");
	InsertColumn(2, "Use Count");
	InsertColumn(3, "In Archive");

	// Update list
	updateList();
}

PatchTableListView::~PatchTableListView() {
}

string PatchTableListView::getItemText(long item, long column) const {
	// Check patch table exists
	if (!patch_table)
		return "INVALID INDEX";

	// Check index is ok
	if (item < 0 || (unsigned)item > patch_table->nPatches())
		return "INVALID INDEX";

	// Get associated patch
	patch_t patch = patch_table->patch(item);

	if (column == 0)						// Index column
		return s_fmt("%04d", item);
	else if (column == 1)					// Name column
		return patch.name;
	else if (column == 2)					// Usage count column
		return s_fmt("%d", patch.used);
	else if (column == 3) {					// Archive column
		// Get patch entry's parent archive
		string archive = "NOT FOUND";
		if (patch.entry)
			archive = patch.entry->getParent()->getFileName(false);
		return archive;
	}
	else									// Invalid column
		return "INVALID COLUMN";
}

void PatchTableListView::updateItemAttr(long item) const {
	// Init attributes (to error colour)
	item_attr->SetTextColour(ListView::colourError());

	// Check patch table exists
	if (!patch_table)
		return;

	// Check index is ok
	if (item < 0 || (unsigned)item > patch_table->nPatches())
		return;

	// Get associated patch
	patch_t patch = patch_table->patch(item);

	// Set normal colour if patch is ok
	if (patch.entry)
		item_attr->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
}

void PatchTableListView::updateList(bool clear) {
	if (clear)
		ClearAll();

	// Set list size
	if (patch_table)
		SetItemCount(patch_table->nPatches());
	else
		SetItemCount(0);

	updateWidth();
	Refresh();
}

/*******************************************************************
 * PATCHTABLEPANEL CLASS FUNCTIONS
 *******************************************************************/

/* PatchTablePanel::PatchTablePanel
 * PatchTablePanel class constructor
 *******************************************************************/
PatchTablePanel::PatchTablePanel(wxWindow* parent, PatchTable* patch_table) : wxPanel(parent, -1) {
	// Init variables
	this->patch_table = patch_table;
	this->parent = (TextureXEditor*)parent;

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Add PNAMES list
	wxStaticBox* frame = new wxStaticBox(this, -1, "Patches (PNAMES)");
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);
	list_patches = new PatchTableListView(this, patch_table);
	framesizer->Add(list_patches, 1, wxEXPAND|wxALL, 4);

	// Add editing controls
	frame = new wxStaticBox(this, -1, "Actions");
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxALL, 4);

	// Add patch button
	btn_add_patch = new wxButton(this, -1, "New Patch");
	framesizer->Add(btn_add_patch, 0, wxEXPAND|wxALL, 4);

	// New patch from file button
	btn_patch_from_file = new wxButton(this, -1, "New Patch from File");
	framesizer->Add(btn_patch_from_file, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Remove patch button
	btn_remove_patch = new wxButton(this, -1, "Remove Patch");
	framesizer->Add(btn_remove_patch, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Change patch button
	btn_change_patch = new wxButton(this, -1, "Change Patch");
	framesizer->Add(btn_change_patch, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Bind events
	btn_add_patch->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PatchTablePanel::onBtnAddPatch, this);
	btn_remove_patch->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PatchTablePanel::onBtnRemovePatch, this);
	btn_change_patch->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PatchTablePanel::onBtnChangePatch, this);
}

/* PatchTablePanel::~PatchTablePanel
 * PatchTablePanel class destructor
 *******************************************************************/
PatchTablePanel::~PatchTablePanel() {
}

void PatchTablePanel::onBtnAddPatch(wxCommandEvent& e) {
	// Prompt for new patch name
	string patch = wxGetTextFromUser("Enter patch entry name:", "Add Patch", wxEmptyString, this);

	// Check something was entered
	if (patch.IsEmpty())
		return;

	// Add to patch table
	patch_table->addPatch(patch, parent->getArchive());

	// Update list
	list_patches->updateList();
}

void PatchTablePanel::onBtnRemovePatch(wxCommandEvent& e) {
	// Check anything is selected
	vector<long> selection = list_patches->getSelection();
	if (selection.size() == 0)
		return;

	// Go through patch list selection
	for (int a = selection.size() - 1; a >= 0; a--) {
		// Check if patch is currently in use
		patch_t& patch = patch_table->patch(selection[a]);
		if (patch.used > 0) {
			// In use, ask if it's ok to remove the patch
			int answer = wxMessageBox(s_fmt("The patch \"%s\" is currently used by %d texture(s), are you sure you wish to remove it?", chr(patch.name), patch.used), "Confirm Remove Patch", wxYES_NO|wxCANCEL, this);
			if (answer == wxYES) {
				// Answered yes, remove the patch
				parent->removePatch(selection[a]);

				// Deselect it
				list_patches->selectItem(selection[a], false);
			}
		}
		else {
			// Not in use, just delete it
			parent->removePatch(selection[a]);

			// Deselect it
			list_patches->selectItem(selection[a], false);
		}
	}

	// Update list
	list_patches->updateList();
}

void PatchTablePanel::onBtnChangePatch(wxCommandEvent& e) {
	// Check anything is selected
	vector<long> selection = list_patches->getSelection();
	if (selection.size() == 0)
		return;

	// Go through patch list selection
	for (unsigned a = 0; a < selection.size(); a++) {
		patch_t& patch = patch_table->patch(selection[a]);

		// Prompt for new patch name
		string newname = wxGetTextFromUser("Enter new patch entry name:", "Change Patch", patch.name, this);

		// Update the patch
		patch_table->replacePatch(selection[a], newname, parent->getArchive());

		// Update the list
		list_patches->updateList();
	}
}
