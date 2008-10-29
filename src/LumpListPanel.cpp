
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    LumpListPanel.cpp
 * Description: LumpListPanel class (and related classes).
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
#include "Wad.h"
#include "LumpListPanel.h"
#include "Lump.h"


/* LumpList::LumpList
 * LumpList class constructor
 *******************************************************************/
LumpList::LumpList(LumpListPanel *parent, int id)
:	wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES|wxLC_HRULES/*|wxLC_EDIT_LABELS*/)
{
	this->parent = parent;
}

/* LumpList::~LumpList
 * LumpList class destructor
 *******************************************************************/
LumpList::~LumpList()
{
}

/* LumpList::updateEntry
 * Updates the list entry at index with it's associated lump's
 * information (name/size/type).
 * Returns false on invalid index or missing lump, true otherwise
 *******************************************************************/
bool LumpList::updateEntry(int index)
{
	// Check that index is valid
	if (index < 0 || index >= this->GetItemCount())
		return false;

	// Get the lump associated with entry index
	Lump* lump = (Lump*)GetItemData(index);

	// Check that it exists
	if (!lump)
	{
		wxLogMessage(_T("LumpList entry at index %d has no associated lump!"), index);
		return false;
	}

	// Setup entry
	wxListItem li;
	li.SetId(index);
	li.SetText(lump->getName());
	SetItem(li);

	return true;
}





/* LumpListPanel::LumpListPanel
 * LumpListPanel class constructor
 *******************************************************************/
LumpListPanel::LumpListPanel(wxWindow *parent, int id, Wad* wad)
:	wxPanel(parent, id)
{
	// Init variables
	this->wad = wad;

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Lumps"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	lump_list = new LumpList(this, -1);
	framesizer->Add(lump_list, 1, wxEXPAND|wxALL, 4);
}

/* LumpListPanel::~LumpListPanel
 * LumpListPanel class destructor
 *******************************************************************/
LumpListPanel::~LumpListPanel()
{
}

/* LumpListPanel::populateLumpList
 * Clears & populates the lump list with all the lumps in the wadfile
 *******************************************************************/
void LumpListPanel::populateLumpList()
{
	// Clear the list
	lump_list->ClearAll();

	// Create the "Name" column
	lump_list->InsertColumn(0, _T("Name"));

	// Go through all lumps and add them to the list
	for (int a = 0; a < wad->numLumps(); a++)
	{
		// Setup new entry
		wxListItem li;
		li.SetId(a);
		li.SetData(wad->lumpAt(a));

		lump_list->InsertItem(li);
		lump_list->updateEntry(a);
	}
}
