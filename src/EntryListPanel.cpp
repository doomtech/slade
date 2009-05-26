
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryListPanel.cpp
 * Description: EntryListPanel class (and related classes).
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
: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VRULES | wxLC_HRULES/*|wxLC_EDIT_LABELS*/) {
	this->parent = parent;
}

/* EntryList::~EntryList
 * EntryList class destructor
 *******************************************************************/
EntryList::~EntryList() {
}

/* EntryList::updateEntry
 * Updates the list item at index with it's associated archive
 * entry's information (name/size/type).
 * Returns false on invalid index or missing entry, true otherwise
 *******************************************************************/
bool EntryList::updateEntry(int index) {
	// Check that index is valid
	if (index < 0 || index >= this->GetItemCount())
		return false;

	// Get the lump associated with entry index
	ArchiveEntry* lump = (ArchiveEntry*) GetItemData(index);

	// Check that it exists
	if (!lump) {
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

	// Type
	li.SetText(lump->getTypeString());
	li.SetColumn(2);
	SetItem(li);
	SetColumnWidth(2, wxLIST_AUTOSIZE);

	// Set default text colour
	SetItemTextColour(index, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));

	// Set entry status text colour if needed
	if (lump->getState() == 2)
		SetItemTextColour(index, wxColour(0, 150, 0));
	else if (lump->getState() == 1)
		SetItemTextColour(index, wxColour(0, 80, 180));

	return true;
}

int EntryList::getWidth() {
	// For the moment. Kinda annoying I have to do this actually, it should be automatic >_<
	return GetColumnWidth(0) + GetColumnWidth(1) + GetColumnWidth(2) + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this) + 4;
}

/* EntryListPanel::EntryListPanel
 * EntryListPanel class constructor
 *******************************************************************/
EntryListPanel::EntryListPanel(wxWindow *parent, int id, Archive* archive)
: wxPanel(parent, id) {
	// Init variables
	this->archive = archive;

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Lumps"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	entry_list = new EntryList(this, ENTRY_LIST);
	framesizer->Add(entry_list, 1, wxEXPAND | wxALL, 4);

	Layout();
}

/* EntryListPanel::~EntryListPanel
 * EntryListPanel class destructor
 *******************************************************************/
EntryListPanel::~EntryListPanel() {
}

/* EntryListPanel::populateEntryList
 * Clears & populates the entry list with all the entries in
 * the archive
 *******************************************************************/
void EntryListPanel::populateEntryList() {
	// Clear the list
	entry_list->ClearAll();

	// Create the "Name" column
	entry_list->InsertColumn(0, _T("Name"));

	// Create the "Size" column
	entry_list->InsertColumn(1, _T("Size"));

	// Create the "Type" column
	entry_list->InsertColumn(2, _T("Type"));

	// Go through all lumps and add them to the list
	for (int a = 0; a < archive->numEntries(); a++) {
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

/* EntryListPanel::getFocusedEntry
 * Gets the archive entry associated with the currently focused list
 * item. Returns NULL if nothing is focused
 *******************************************************************/
ArchiveEntry* EntryListPanel::getFocusedEntry() {
	// Get the focus index
	int focus = getFocus();

	// Check that the focus index is invalid
	if (focus < 0 || focus > entry_list->GetItemCount())
		return NULL;

	// Return the focused archive entry
	return (ArchiveEntry*) (entry_list->GetItemData(focus));
}

/* EntryListPanel::getFocusedEntry
 * Gets the list index of the currently focused list item
 *******************************************************************/
int EntryListPanel::getFocus() {
	// Get the item in the list that is focused
	return entry_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
}

/* EntryListPanel::getSelectedEntries
 * Returns a vector of all selected archive entries
 *******************************************************************/
vector<ArchiveEntry*> EntryListPanel::getSelectedEntries() {
	// Init vector
	vector<ArchiveEntry*> ret;

	// Go through all entries
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = entry_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected entry to the vector
		ret.push_back((ArchiveEntry*)(entry_list->GetItemData(item)));
	}

	return ret;
}

/* EntryListPanel::getSelection
 * Returns a vector of all selected list item indices
 *******************************************************************/
vector<int> EntryListPanel::getSelection() {
	// Init vector
	vector<int> ret;

	// Go through all entries
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = entry_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected index to the vector
		ret.push_back(item);
	}

	return ret;
}

/* EntryListPanel::swapItems
 * Swaps two list items (including their focused/selected states)
 *******************************************************************/
bool EntryListPanel::swapItems(int item1, int item2) {
	// Check item 1
	if (item1 < 0 || item1 > entry_list->GetItemCount())
		return false;

	// Check item 2
	if (item2 < 0 || item2 > entry_list->GetItemCount())
		return false;

	// Get item 1 info
	wxListItem i1;
	i1.SetId(item1);
	i1.SetMask(0xFFFF);
	entry_list->GetItem(i1);
	long state1 = entry_list->GetItemState(item1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED); // Not sure why I have to get/set states separately :/

	// Get item 2 info
	wxListItem i2;
	i2.SetId(item2);
	i2.SetMask(0xFFFF);
	entry_list->GetItem(i2);
	long state2 = entry_list->GetItemState(item2, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

	// Swap the items
	i1.SetId(item2);
	i2.SetId(item1);
	entry_list->SetItem(i1);
	entry_list->SetItem(i2);
	entry_list->SetItemState(item1, state2, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	entry_list->SetItemState(item2, state1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

	// Update the items
	entry_list->updateEntry(item1);
	entry_list->updateEntry(item2);

	return true;
}

/* EntryListPanel::addEntry
 * Adds an entry to the list
 *******************************************************************/
bool EntryListPanel::addEntry(DWORD archive_index) {
	// Get the entry to add
	ArchiveEntry* entry = archive->getEntry(archive_index);

	// Check it
	if (!entry)
		return false;

	// Setup new entry
	wxListItem li;
	li.SetId(archive_index);
	li.SetData(entry);

	// Add it to the list
	entry_list->InsertItem(li);
	entry_list->updateEntry(archive_index);

	return true;
}

/* EntryListPanel::updateEntry
 * Updates an entry in the list
 *******************************************************************/
bool EntryListPanel::updateEntry(DWORD archive_index) {
	// Just update the list item corresponding with the entry index
	// (it's 1-1 in a normal EntryListPanel, will be different for zip archives)
	entry_list->updateEntry(archive_index);
}

bool EntryListPanel::removeEntry(DWORD archive_index) {
	// Just delete the list item corresponding with the entry index
	// (it's 1-1 in a normal EntryListPanel, will be different for zip archives)
	entry_list->DeleteItem(archive_index);
}

/* EntryListPanel::moveUp
 * Moves all selected entries up.
 * Returns false if the first selected item was at the top of the
 * list, true otherwise
 *******************************************************************/
bool EntryListPanel::moveUp() {
	// Get selection
	vector<int> selection = getSelection();

	// If the first selected item is at the top of the list
	// then don't move anything up
	if (selection[0] == 0)
		return false;

	// Move each one up by swapping it with the entry above it
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the entries to swap
		ArchiveEntry* entry = (ArchiveEntry*)(entry_list->GetItemData(selection[a]));
		ArchiveEntry* above = (ArchiveEntry*)(entry_list->GetItemData(selection[a]-1));

		// Swap them in the archive
		archive->swapEntries(entry, above);
	}

	// Return success
	return true;
}

/* EntryListPanel::moveDown
 * Moves all selected entries down.
 * Returns false if the last selected item was at the end of the
 * list, true otherwise
 *******************************************************************/
bool EntryListPanel::moveDown() {
	// Get selection
	vector<int> selection = getSelection();

	// If the last selected item is at the end of the list
	// then don't move anything down
	if (selection.back() == entry_list->GetItemCount()-1)
		return false;

	// Move each one down by swapping it with the entry below it
	for (int a = selection.size()-1; a >= 0; a--) {
		// Get the entries to swap
		ArchiveEntry* entry = (ArchiveEntry*)(entry_list->GetItemData(selection[a]));
		ArchiveEntry* below = (ArchiveEntry*)(entry_list->GetItemData(selection[a]+1));

		// Swap them in the archive
		archive->swapEntries(entry, below);
	}

	// Return success
	return true;
}

BEGIN_EVENT_TABLE(EntryListPanel, wxPanel)
EVT_LIST_ITEM_FOCUSED(EntryListPanel::ENTRY_LIST, EntryListPanel::onEntryListChange)
END_EVENT_TABLE()

/* EntryListPanel::onEntryListChange
 * Called when the current focus on the entry list control is changed
 *******************************************************************/
void EntryListPanel::onEntryListChange(wxListEvent& event) {
	// Set the currently focused item index
	//focus_index = event.GetIndex();

	// Pass the event up to the parent window, as the wad panel should deal with it,
	// not this panel.
	event.Skip();
}

void EntryListPanel::onEntryListActivated(wxListEvent& event) {

}




ZipEntryListPanel::ZipEntryListPanel(wxWindow* parent, int id, Archive* archive)
: EntryListPanel(parent, id, archive) {
	// Check the archive type
	if (archive->getType() != ARCHIVE_ZIP) {
		wxMessageBox(_T("Error: Attempt to open a non-zip archive with a zip entry list, this shouldn't happen"), _T("Error"));
		archive = NULL;
	}
}

ZipEntryListPanel::~ZipEntryListPanel() {
}
