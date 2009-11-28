
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
#include "ArchiveManager.h"
#include "Icons.h"
#include <wx/imaglist.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
wxColor col_new(0, 150, 0);
wxColor col_modified(0, 80, 180);
CVAR(Bool, col_size, true, CVAR_SAVE);
CVAR(Bool, col_type, true, CVAR_SAVE);
CVAR(Bool, entry_list_monospace, true, CVAR_SAVE);

// No edit labels for now, causes too many problems
#define ENTRYLIST_FLAGS wxLC_REPORT|wxLC_VRULES|wxLC_HRULES


/*******************************************************************
 * ENTRYLIST CLASS FUNCTIONS
 *******************************************************************/

/* EntryList::EntryList
 * EntryList class constructor
 *******************************************************************/
EntryList::EntryList(EntryListPanel *parent, int id)
: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, ENTRYLIST_FLAGS) {
	this->parent = parent;

	// Set font to monospace if cvar set
	if (entry_list_monospace) {
		wxFont f = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
		SetFont(wxFont(f.GetPointSize(), wxFONTFAMILY_MODERN, f.GetStyle(), f.GetWeight()));
	}

	// Setup entry icons
	image_list = new wxImageList(16, 16, false, 0);
	image_list->Add(getIcon(_T("e_unknown")));
	image_list->Add(getIcon(_T("e_marker")));
	image_list->Add(getIcon(_T("e_text")));
	image_list->Add(getIcon(_T("e_patch")));
	image_list->Add(getIcon(_T("e_sprite")));
	image_list->Add(getIcon(_T("e_flat")));
	image_list->Add(getIcon(_T("e_gfx")));
	image_list->Add(getIcon(_T("e_gfx")));
	image_list->Add(getIcon(_T("e_png")));
	image_list->Add(getIcon(_T("e_gfx")));		// image
	image_list->Add(getIcon(_T("e_sound")));
	image_list->Add(getIcon(_T("e_sound")));	// wav
	image_list->Add(getIcon(_T("e_sound")));	// mp3
	image_list->Add(getIcon(_T("e_sound")));	// flac
	image_list->Add(getIcon(_T("e_music")));
	image_list->Add(getIcon(_T("e_music")));
	image_list->Add(getIcon(_T("e_music")));
	image_list->Add(getIcon(_T("e_texturex")));
	image_list->Add(getIcon(_T("e_pnames")));
	image_list->Add(getIcon(_T("e_map")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_map_data")));
	image_list->Add(getIcon(_T("e_default")));
	image_list->Add(getIcon(_T("e_default")));
	image_list->Add(getIcon(_T("e_wad")));
	image_list->Add(getIcon(_T("e_folder")));
	image_list->Add(getIcon(_T("e_upfolder")));
	SetImageList(image_list, wxIMAGE_LIST_SMALL);
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
bool EntryList::updateEntry(int index, bool update_colsize) {
	// Check that index is valid
	if (index < 0 || index >= this->GetItemCount())
		return false;

	// Get the Entry associated with entry index
	ArchiveEntry* entry = (ArchiveEntry*) GetItemData(index);

	// Check that it exists
	if (!entry) {
		wxLogMessage(_T("EntryList item %d has no associated archive entry!"), index);
		return false;
	}

	// Detect type
	entry->detectType(true, false);

	// -- Setup entry --
	// Name
	wxListItem li;
	li.SetId(index);
	li.SetText(entry->getName());
	SetItem(li);
	if (update_colsize) {
		SetColumnWidth(0, wxLIST_AUTOSIZE);
		// Add extra width in linux as wxLIST_AUTOSIZE seems to ignore listitem images on wxGTK
		#ifdef __WXGTK__
		SetColumnWidth(0, GetColumnWidth(0) + 20);
		#endif
		if (GetColumnWidth(0) < 32) SetColumnWidth(0, 32);
	}

	// Size
	int col = 1;
	if (col_size) {
		li.SetText(entry->getSizeString());
		li.SetColumn(col);
		SetItem(li);
		if (update_colsize) SetColumnWidth(col, wxLIST_AUTOSIZE);
		if (GetColumnWidth(col) < 32) SetColumnWidth(col, 32);
		col++;
	}

	// Type
	if (col_type) {
		li.SetText(entry->getTypeString());
		li.SetColumn(col);
		SetItem(li);
		if (update_colsize) SetColumnWidth(col, wxLIST_AUTOSIZE);
		if (GetColumnWidth(col) < 32) SetColumnWidth(col, 32);
	}

	// Set default text colour
	SetItemTextColour(index, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));

	// Set entry status text colour if needed
	if (entry->getState() == 2)
		SetItemTextColour(index, col_new);
	else if (entry->getState() == 1)
		SetItemTextColour(index, col_modified);

	SetItemImage(index, entry->getType());

	// Setup size
	if (update_colsize) {
		SetMinSize(wxSize(getWidth(), -1));
		wxTheApp->GetTopWindow()->Layout();
	}

	return true;
}

/* EntryList::getWidth
 * Gets the current minimum width of the entry list
 *******************************************************************/
int EntryList::getWidth() {
	// For the moment. Kinda annoying I have to do this actually, it should be automatic >_<
	int width = 0;
	for (int a = 0; a < GetColumnCount(); a++)
		width += GetColumnWidth(a);

	// Always leave room for the scrollbar (wxWidgets is silly)
	width += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);

	return width + 4;
}


/*******************************************************************
 * ENTRYLISTPANEL CLASS FUNCTIONS
 *******************************************************************/

/* EntryListPanel::EntryListPanel
 * EntryListPanel class constructor
 *******************************************************************/
EntryListPanel::EntryListPanel(wxWindow *parent, int id, Archive* archive)
: wxPanel(parent, id) {
	// Init variables
	this->archive = archive;
	this->col_update = true;

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Entries"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	entry_list = new EntryList(this, ENTRY_LIST);
	framesizer->Add(entry_list, 1, wxEXPAND | wxALL, 4);


	// Bind Events
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &EntryListPanel::onEntryListActivated, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_COL_END_DRAG, &EntryListPanel::onEntryListColResize, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_COL_RIGHT_CLICK, &EntryListPanel::onEntryListColRightClick, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EntryListPanel::onMenu, this, MENU_COLUMNS_SIZE, MENU_COLUMNS_TYPE);

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

	// Hide the list
	entry_list->Show(false);

	// Create the "Name" column
	entry_list->InsertColumn(0, _T("Name"));

	// Create the "Size" column
	int col = 1;
	if (col_size) {
		entry_list->InsertColumn(1, _T("Size"));
		col++;
	}

	// Create the "Type" column
	if (col_type) entry_list->InsertColumn(col, _T("Type"));

	// Go through all entries and add them to the list
	for (int a = 0; a < archive->numEntries(); a++) {
		// Setup new entry
		ArchiveEntry* entry = archive->getEntry(a);
		wxListItem li;
		li.SetId(a);
		li.SetData(entry);
		entry_list->InsertItem(li);
		entry_list->updateEntry(a, false);
	}

	// Setup column widths
	for (int a = 0; a < entry_list->GetColumnCount(); a++) {
		entry_list->SetColumnWidth(a, wxLIST_AUTOSIZE);

		// Minimum size of 32 for each column
		if (entry_list->GetColumnWidth(a) < 32)
			entry_list->SetColumnWidth(a, 32);
	}

	// Add extra width to the name column in linux as wxLIST_AUTOSIZE seems to ignore listitem images on wxGTK
	#ifdef __WXGTK__
	entry_list->SetColumnWidth(0, entry_list->GetColumnWidth(0) + 20);
	#endif

	// Show the list
	entry_list->Show();

	// Update list control width
	updateListWidth();
}

/* EntryListPanel::updateListWidth
 * Updates the entrylist width and updates the layout to accommodate
 * it
 *******************************************************************/
void EntryListPanel::updateListWidth() {
	entry_list->SetMinSize(wxSize(entry_list->getWidth(), -1));
	GetParent()->Layout();
}

/* EntryListPanel::getFocusedEntry
 * Gets the archive entry associated with the currently focused list
 * item. Returns NULL if nothing is focused
 *******************************************************************/
ArchiveEntry* EntryListPanel::getFocusedEntry() {
	// Get the focus index
	int focus = getFocus();

	// Check that the focus index is valid
	if (focus < 0 || focus > entry_list->GetItemCount())
		return NULL;

	// Return the focused archive entry
	return (ArchiveEntry*) (entry_list->GetItemData(focus));
}

/* EntryListPanel::getFocus
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

	// Get selection
	vector<int> selection = getSelection();

	// Go through selection and add associated entries to the return vector
	for (size_t a = 0; a < selection.size(); a++)
		ret.push_back((ArchiveEntry*)(entry_list->GetItemData(selection[a])));

	return ret;
}

/* EntryListPanel::getSelection
 * Returns a vector of all selected list item indices
 *******************************************************************/
vector<int> EntryListPanel::getSelection() {
	// Init vector
	vector<int> ret;

	// Go through all items
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

/* EntryListPanel::getLastSelected
 * Gets the index of the last selected item in the list, or -1 if no
 * item is selected
 *******************************************************************/
int EntryListPanel::getLastSelected() {
	// Go through all items
	int item = -1;
	while (true) {
		// Get the next item in the list that is selected
		int index = entry_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// Exit loop if there is no selected item after the current
		if (index == -1)
			break;

		// Otherwise set the item to this index and keep going
		item = index;
	}

	return item;
}

/* EntryListPanel::getLastSelectedEntry
 * Gets the archive entry associated with the last selected item in
 * the list. Returns NULL if no item is selected
 *******************************************************************/
ArchiveEntry* EntryListPanel::getLastSelectedEntry() {
	int index = getLastSelected();

	if (index >= 0)
		return (ArchiveEntry*)(entry_list->GetItemData(index));
	else
		return NULL;
}

/* EntryListPanel::getListSize
 * Returns the number of items currently in the list
 *******************************************************************/
int EntryListPanel::getListSize() {
	return entry_list->GetItemCount();
}

/* EntryListPanel::swapItems
 * Swaps two list items (including their focused/selected states)
 *******************************************************************/
bool EntryListPanel::swapItems(int item1, int item2, ArchiveEntry* e1, ArchiveEntry* e2) {
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

	// Update items
	entry_list->updateEntry(item1, false);
	entry_list->updateEntry(item2, false);

	return true;
}

/* EntryListPanel::addEntry
 * Adds an entry to the list
 *******************************************************************/
bool EntryListPanel::addEntry(uint32_t archive_index, ArchiveEntry* e) {
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
	entry_list->updateEntry(archive_index, col_update);

	return true;
}

/* EntryListPanel::updateEntry
 * Updates an entry in the list
 *******************************************************************/
bool EntryListPanel::updateEntry(uint32_t archive_index, ArchiveEntry* e) {
	// Just update the list item corresponding with the entry index
	// (it's 1-1 in a normal EntryListPanel, will be different for zip archives)
	entry_list->updateEntry(archive_index, col_update);
}

/* EntryListPanel::removeEntry
 * Removes an entry from the list
 *******************************************************************/
bool EntryListPanel::removeEntry(uint32_t archive_index, ArchiveEntry* e) {
	// Just delete the list item corresponding with the entry index
	// (it's 1-1 in a normal EntryListPanel, will be different for zip archives)
	entry_list->DeleteItem(archive_index);
}

/* EntryListPanel::getEntryListItem
 * Gets the list item index of the given entry. Returns -1 if the
 * entry doesn't exist in the list, or the given entry was NULL
 *******************************************************************/
int EntryListPanel::getEntryListItem(ArchiveEntry* entry) {
	if (!entry)
		return -1;

	int count = entry_list->GetItemCount();
	for (int a = 0; a < count; a++) {
		if ((ArchiveEntry*)entry_list->GetItemData(a) == entry)
			return a;
	}

	return -1;
}

/* EntryListPanel::moveUp
 * Moves all selected entries up.
 * Returns false if the first selected item was at the top of the
 * list, true otherwise
 *******************************************************************/
bool EntryListPanel::moveUp() {
	// Get selection
	vector<int> selection = getSelection();

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

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

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

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

/* EntryListPanel::selectAll
 * Selects all items in the entry list
 *******************************************************************/
void EntryListPanel::selectAll() {
	for (int a = 0; a < entry_list->GetItemCount(); a++)
		entry_list->SetItemState(a, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}


/*******************************************************************
 * ENTRYLISTPANEL EVENTS
 *******************************************************************/

/* EntryListPanel::onEntryListActivated
 * Called when an item on the list is 'activated' (double-click or
 * enter)
 *******************************************************************/
void EntryListPanel::onEntryListActivated(wxListEvent& e) {
}

/* EntryListPanel::onEntryListEditLabel
 * Called when an 'edit label' operation finishes on a list entry
 *******************************************************************/
/*
void EntryListPanel::onEntryListEditLabel(wxListEvent& event) {
	ArchiveEntry* entry = getFocusedEntry();

	if (entry && !event.IsEditCancelled()) {
		archive->renameEntry(entry, event.GetLabel());
	}
}
*/

/* EntryListPanel::onEntryListColResize
 * Called when a column on the entry list is resized
 *******************************************************************/
void EntryListPanel::onEntryListColResize(wxListEvent& e) {
	updateListWidth();
}

/* EntryListPanel::onEntryListColRightClick
 * Called when an entrylist column header is right clicked
 *******************************************************************/
void EntryListPanel::onEntryListColRightClick(wxListEvent& e) {
	wxMenu* popup = new wxMenu();
	popup->AppendCheckItem(MENU_COLUMNS_SIZE, _T("Size"), _T("Show the size column"));
	popup->AppendCheckItem(MENU_COLUMNS_TYPE, _T("Type"), _T("Show the type column"));
	popup->Check(MENU_COLUMNS_SIZE, col_size);
	popup->Check(MENU_COLUMNS_TYPE, col_type);

	PopupMenu(popup);
}

/* EntryListPanel::onMenu
 * Called when a menu item is selected
 *******************************************************************/
void EntryListPanel::onMenu(wxCommandEvent& e) {
	switch (e.GetId()) {
		case MENU_COLUMNS_SIZE:
			col_size = !col_size;
			populateEntryList();
			break;
		case MENU_COLUMNS_TYPE:
			col_type = !col_type;
			populateEntryList();
			break;
	}

	e.Skip();
}
