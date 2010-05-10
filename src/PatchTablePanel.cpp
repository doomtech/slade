
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


/*******************************************************************
 * PATCHTABLEPANEL CLASS FUNCTIONS
 *******************************************************************/

/* PatchTablePanel::PatchTablePanel
 * PatchTablePanel class constructor
 *******************************************************************/
PatchTablePanel::PatchTablePanel(wxWindow* parent, PatchTable* patch_table) : wxPanel(parent, -1) {
	// Init variables
	this->patch_table = patch_table;

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
}

/* PatchTablePanel::~PatchTablePanel
 * PatchTablePanel class destructor
 *******************************************************************/
PatchTablePanel::~PatchTablePanel() {
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
