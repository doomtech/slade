
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
	wxStaticBox* frame = new wxStaticBox(this, -1, _T("Patches (PNAMES)"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);
	list_patches = new ListView(this, -1);
	framesizer->Add(list_patches, 1, wxEXPAND|wxALL, 4);

	// Add editing controls
	frame = new wxStaticBox(this, -1, _T("Actions"));
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxALL, 4);

	// Add patch button
	btn_add_patch = new wxButton(this, -1, _T("Add Patch"));
	framesizer->Add(btn_add_patch, 0, wxEXPAND|wxALL, 4);

	// Remove patch button
	btn_remove_patch = new wxButton(this, -1, _T("Remove Patch"));
	framesizer->Add(btn_remove_patch, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Change patch button
	btn_change_patch = new wxButton(this, -1, _T("Change Patch"));
	framesizer->Add(btn_change_patch, 0, wxEXPAND|wxALL, 4);

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

void PatchTablePanel::updatePatchListItem(unsigned index) {
	// Check index
	if (index >= list_patches->GetItemCount())
		return;

	// Get patch
	patch_t& patch = patch_table->patch(index);

	// Get patch entry's parent archive
	string archive = _T("NOT FOUND");
	if (patch.entry)
		archive = patch.entry->getParent()->getFileName(false);

	// Update list item
	list_patches->setItemText(index, 0, s_fmt(_T("%04d"), index));
	list_patches->setItemText(index, 1, patch.name);
	list_patches->setItemText(index, 2, s_fmt(_T("%d"), patch.used));
	list_patches->setItemText(index, 3, archive);

	// Update item colour
	if (patch.entry)
		list_patches->setItemStatus(index, LV_STATUS_NORMAL);
	else
		list_patches->setItemStatus(index, LV_STATUS_ERROR);

	// Update list width
	list_patches->updateSize();
	list_patches->GetParent()->Layout();
}

/* PatchTablePanel::populatePatchList
 * Populated the patch list with details of all patches in the patch
 * table
 *******************************************************************/
void PatchTablePanel::populatePatchList() {
	// Clear current list
	list_patches->ClearAll();
	list_patches->Show(false);

	// Add columns
	list_patches->InsertColumn(0, _T("#"));
	list_patches->InsertColumn(1, _T("Patch Name"));
	list_patches->InsertColumn(2, _T("Use Count"));
	list_patches->InsertColumn(3, _T("In Archive"));

	// Add pnames entries to the list
	list_patches->enableSizeUpdate(false);
	for (uint32_t a = 0; a < patch_table->nPatches(); a++) {
		string name = patch_table->patchName(a);
		string archive = _T("NOT FOUND");

		// Get parent archive if any
		ArchiveEntry* entry = patch_table->patchEntry(a);
		if (entry)
			archive = entry->getParent()->getFileName(false);

		string cols[] = { s_fmt(_T("%04d"), a), name, s_fmt(_T("%d"), patch_table->patch(a).used), archive };
		list_patches->addItem(a, wxArrayString(4, cols));

		// Colour red if patch entry not found
		if (!entry)
			list_patches->setItemStatus(a, LV_STATUS_ERROR);
	}

	// Update list width
	list_patches->Show(true);
	list_patches->enableSizeUpdate(true);
	list_patches->updateSize();
	list_patches->GetParent()->Layout();
}


void PatchTablePanel::onBtnAddPatch(wxCommandEvent& e) {
	// Prompt for new patch name
	string patch = wxGetTextFromUser(_T("Enter patch entry name:"), _T("Add Patch"), wxEmptyString, this);

	// Add to patch table
	patch_table->addPatch(patch, parent->getArchive());

	// Add to patch list
	int index = list_patches->GetItemCount();
	list_patches->addItem(index, s_fmt(_T("%04d"), index));
	updatePatchListItem(index);
}

void PatchTablePanel::onBtnRemovePatch(wxCommandEvent& e) {
	// Go through patch list selection
	wxArrayInt selection = list_patches->selectedItems();
	for (int a = selection.size() - 1; a >= 0; a--) {
		// Check if patch is currently in use
		patch_t& patch = patch_table->patch(selection[a]);
		if (patch.used > 0) {
			// In use, ask if it's ok to remove the patch
			int answer = wxMessageBox(s_fmt(_T("The patch \"%s\" is currently used by %d texture(s), are you sure you wish to remove it?"), chr(patch.name), patch.used), _T("Confirm Remove Patch"), wxYES_NO|wxCANCEL, this);
			if (answer == wxYES) {
				// Answered yes, remove the patch
				parent->removePatch(selection[a]);
			}
		}
		else {
			// Not in use, just delete it
			parent->removePatch(selection[a]);
		}
	}

	// Delete selection from list
	list_patches->deleteItems(selection);

	// Update items after deleted ones (index # will be changed)
	list_patches->enableSizeUpdate(false);
	for (unsigned a = selection[0]; a < list_patches->GetItemCount(); a++)
		updatePatchListItem(a);

	// Update list width
	list_patches->enableSizeUpdate(true);
	list_patches->updateSize();
	list_patches->GetParent()->Layout();
}

void PatchTablePanel::onBtnChangePatch(wxCommandEvent& e) {
	// Go through patch list selection
	wxArrayInt selection = list_patches->selectedItems();
	for (unsigned a = 0; a < selection.size(); a++) {
		patch_t& patch = patch_table->patch(selection[a]);

		// Prompt for new patch name
		string newname = wxGetTextFromUser(_T("Enter new patch entry name:"), _T("Change Patch"), patch.name, this);

		// Update the patch
		patch_table->replacePatch(selection[a], newname, parent->getArchive());

		// Update the list entry
		updatePatchListItem(selection[a]);
	}
}
