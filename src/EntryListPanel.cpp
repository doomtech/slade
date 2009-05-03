
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
#include "Archive.h"
#include "EntryListPanel.h"
#include "ArchiveEntry.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/


/* EntryList::EntryList
 * EntryList class constructor
 *******************************************************************/
EntryList::EntryList(EntryListPanel *parent, int id)
:	wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES|wxLC_HRULES/*|wxLC_EDIT_LABELS*/)
{
	this->parent = parent;
}

/* EntryList::~EntryList
 * EntryList class destructor
 *******************************************************************/
EntryList::~EntryList()
{
}

/* EntryList::updateEntry
 * Updates the list entry at index with it's associated archive
 * entry's information (name/size/type).
 * Returns false on invalid index or missing entry, true otherwise
 *******************************************************************/
bool EntryList::updateEntry(int index)
{
	// Check that index is valid
	if (index < 0 || index >= this->GetItemCount())
		return false;

	// Get the lump associated with entry index
	ArchiveEntry* lump = (ArchiveEntry*)GetItemData(index);

	// Check that it exists
	if (!lump)
	{
		wxLogMessage(_T("EntryList entry at index %d has no associated archive entry!"), index);
		return false;
	}

	// -- Setup entry --
	// Name
	wxListItem li;
	li.SetId(index);
	li.SetText(lump->getName());
	SetItem(li);
	SetColumnWidth(0, wxLIST_AUTOSIZE);

	// Size
	li.SetText(s_fmt(_T("%d"), lump->getSize()));
	li.SetColumn(1);
	SetItem(li);
	SetColumnWidth(1, wxLIST_AUTOSIZE);

	return true;
}

int EntryList::getWidth()
{
	// For the moment. Kinda annoying I have to do this actually, it should be automatic >_<
	return GetColumnWidth(0) + GetColumnWidth(1) + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);
}




/* EntryListPanel::EntryListPanel
 * EntryListPanel class constructor
 *******************************************************************/
EntryListPanel::EntryListPanel(wxWindow *parent, int id, Archive* archive)
:	wxPanel(parent, id)
{
	// Init variables
	this->archive = archive;

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Lumps"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	entry_list = new EntryList(this, -1);
	framesizer->Add(entry_list, 1, wxEXPAND|wxALL, 4);

	Layout();
}

/* EntryListPanel::~EntryListPanel
 * EntryListPanel class destructor
 *******************************************************************/
EntryListPanel::~EntryListPanel()
{
}

/* EntryListPanel::populateEntryList
 * Clears & populates the entry list with all the entries in
 * the archive
 *******************************************************************/
void EntryListPanel::populateEntryList()
{
	// Clear the list
	entry_list->ClearAll();

	// Create the "Name" column
	entry_list->InsertColumn(0, _T("Name"));

	// Create the "Size" column
	entry_list->InsertColumn(1, _T("Size"));

	// Go through all lumps and add them to the list
	for (int a = 0; a < archive->numEntries(); a++)
	{
		// Setup new entry
		wxListItem li;
		li.SetId(a);
		li.SetData(archive->getEntry(a));

		entry_list->InsertItem(li);
		entry_list->updateEntry(a);
	}

	// Setup size
	entry_list->SetMinSize(wxSize(entry_list->getWidth(), -1));
}
