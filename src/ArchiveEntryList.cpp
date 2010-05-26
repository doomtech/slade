
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ArchiveEntryList.cpp
 * Description: A list widget that shows all entries in a list-based
 *              archive (eg WadArchive). Keeps in sync with it's
 *              associated archive automatically.
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
#include "ArchiveEntryList.h"
#include "Icons.h"
#include "ListView.h"
#include <wx/imaglist.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Int, elist_colname_width, 80, CVAR_SAVE)
CVAR(Int, elist_colsize_width, 64, CVAR_SAVE)
CVAR(Int, elist_coltype_width, 160, CVAR_SAVE)
CVAR(Bool, elist_colsize_show, true, CVAR_SAVE)
CVAR(Bool, elist_coltype_show, true, CVAR_SAVE)
CVAR(Bool, elist_hrules, false, CVAR_SAVE)
CVAR(Bool, elist_vrules, false, CVAR_SAVE)


/*******************************************************************
 * ARCHIVEENTRYLIST CLASS FUNCTIONS
 *******************************************************************/

/* ArchiveEntryList::ArchiveEntryList
 * ArchiveEntryList class constructor
 *******************************************************************/
ArchiveEntryList::ArchiveEntryList(wxWindow* parent) : wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL) {
	// Init variables
	this->archive = archive;
	item_attr = new wxListItemAttr();

	// Setup columns
	setupColumns();

	// Setup entry icons
	wxImageList* image_list = new wxImageList(16, 16, false, 0);

	wxArrayString et_icon_list = EntryType::getIconList();
	for (size_t a = 0; a < et_icon_list.size(); a++) {
		if (image_list->Add(getIcon(et_icon_list[a])) < 0)
			image_list->Add(getIcon("e_default"));
	}

	SetImageList(image_list, wxIMAGE_LIST_SMALL);

	// Bind events
	Bind(wxEVT_COMMAND_LIST_COL_RIGHT_CLICK, &ArchiveEntryList::onColumnHeaderRightClick, this);
	Bind(wxEVT_COMMAND_LIST_COL_END_DRAG, &ArchiveEntryList::onColumnResize, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ArchiveEntryList::onMenu, this);

	// Setup flags
	SetSingleStyle(wxLC_HRULES, elist_hrules);
	SetSingleStyle(wxLC_VRULES, elist_vrules);
}

/* ArchiveEntryList::~ArchiveEntryList
 * ArchiveEntryList class destructor
 *******************************************************************/
ArchiveEntryList::~ArchiveEntryList() {
}

/* ArchiveEntryList::setArchive
 * Sets the archive for this widget to handle (can be NULL for no
 * archive)
 *******************************************************************/
void ArchiveEntryList::setArchive(Archive* archive) {
	// Stop listening to current archive (if any)
	if (this->archive)
		stopListening(this->archive);

	// Set archive (allow null)
	this->archive = archive;

	// Init new archive if given
	if (archive) {
		listenTo(archive);						// Listen to it
		SetItemCount(archive->numEntries());	// Set list size to no. of entries
	}
}

/* ArchiveEntryList::setupColumns
 * Creates/sets the list columns depending on user options
 *******************************************************************/
void ArchiveEntryList::setupColumns() {
	// Remove existing columns
	while (GetColumnCount() > 0)
		DeleteColumn(0);

	// Create columns
	int col_num = 0;
	InsertColumn(col_num, "Name");
	SetColumnWidth(col_num++, elist_colname_width);
	if (elist_colsize_show) {
		InsertColumn(col_num, "Size");
		SetColumnWidth(col_num++, elist_colsize_width);
	}
	if (elist_coltype_show) {
		InsertColumn(col_num, "Type");
		SetColumnWidth(col_num++, elist_coltype_width);
	}
}

/* ArchiveEntryList::updateWidth
 * Updates the entry list's minimum requested width to allow the
 * widget to be shown with no horizontal scrollbar
 *******************************************************************/
void ArchiveEntryList::updateWidth() {
	// Get total column width
	int width = 8;
	for (int a = 0; a < GetColumnCount(); a++)
		width += GetColumnWidth(a);

	// Always leave room for the scrollbar (wxWidgets is silly)
	width += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);

	// Set widget size
	SetSizeHints(width, -1);
}

/* ArchiveEntryList::columnType
 * Returns the 'type' of column at [column] (name, size or type)
 *******************************************************************/
int ArchiveEntryList::columnType(int column) const {
	if (column == 0)
		return AEL_COLUMN_NAME;		// Name column is always 0
	else if (column == 1) {			// Column 1 can be either size or type
		if (elist_colsize_show)
			return AEL_COLUMN_SIZE;
		else
			return AEL_COLUMN_TYPE;
	}
	else if (column == 2)			// Column 2 is always type (if it exists)
		return AEL_COLUMN_TYPE;
	else
		return -1;					// Invalid column
}

/* ArchiveEntryList::updateList
 * Updates + refreshes the list
 *******************************************************************/
void ArchiveEntryList::updateList(bool clear) {
	// Clear all items if needed
	if (clear)
		DeleteAllItems();

	// Update list size
	SetItemCount(archive->numEntries());

	// Refresh
	Refresh();
}

/* ArchiveEntryList::getEntry
 * Returns the ArchiveEntry associated with the list item at [index].
 * Returns NULL if the index is out of bounds or no archive is open
 *******************************************************************/
ArchiveEntry* ArchiveEntryList::getEntry(int index) const {
	// Check index
	if (!archive || index < 0 || index >= (signed)archive->numEntries())
		return NULL;

	return archive->getEntry(index);
}

/* ArchiveEntryList::getFocusedEntry
 * Gets the archive entry associated with the currently focused list
 * item. Returns NULL if nothing is focused or no archive is open
 *******************************************************************/
ArchiveEntry* ArchiveEntryList::getFocusedEntry() {
	// Get the focus index
	int focus = getFocus();

	// Check that the focus index is valid
	if (focus < 0 || focus > GetItemCount())
		return NULL;

	// Return the focused archive entry
	if (archive)
		return getEntry(focus);
	else
		return NULL;
}

/* ArchiveEntryList::getFocus
 * Gets the list index of the currently focused list item
 *******************************************************************/
int ArchiveEntryList::getFocus() {
	// Get the item in the list that is focused
	return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
}

/* ArchiveEntryList::getSelectedEntries
 * Returns a vector of all selected archive entries
 *******************************************************************/
vector<ArchiveEntry*> ArchiveEntryList::getSelectedEntries() {
	// Init vector
	vector<ArchiveEntry*> ret;

	// Return empty if no archive open
	if (!archive)
		return ret;

	// Get selection
	vector<int> selection = getSelection();

	// Go through selection and add associated entries to the return vector
	for (size_t a = 0; a < selection.size(); a++)
		ret.push_back(getEntry(selection[a]));

	return ret;
}

/* ArchiveEntryList::getSelection
 * Returns a vector of all selected list item indices
 *******************************************************************/
vector<int> ArchiveEntryList::getSelection() {
	// Init vector
	vector<int> ret;

	// Go through all items
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected index to the vector
		ret.push_back(item);
	}

	return ret;
}

/* ArchiveEntryList::getLastSelected
 * Gets the index of the last selected item in the list, or -1 if no
 * item is selected
 *******************************************************************/
int ArchiveEntryList::getLastSelected() {
	// Go through all items
	int item = -1;
	while (true) {
		// Get the next item in the list that is selected
		int index = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// Exit loop if there is no selected item after the current
		if (index == -1)
			break;

		// Otherwise set the item to this index and keep going
		item = index;
	}

	return item;
}

/* ArchiveEntryList::getLastSelectedEntry
 * Gets the archive entry associated with the last selected item in
 * the list. Returns NULL if no item is selected
 *******************************************************************/
ArchiveEntry* ArchiveEntryList::getLastSelectedEntry() {
	int index = getLastSelected();

	if (index >= 0 && archive)
		return getEntry(index);
	else
		return NULL;
}

/* ArchiveEntryList::selectItem
 * Selects or deselects [item] depending on [select]
 *******************************************************************/
void ArchiveEntryList::selectItem(int item, bool select) {
	// Check item id is in range
	if (item >= GetItemCount())
		return;

	// Select/deselect the item
	if (select)
		SetItemState(item, 0xFFFF, wxLIST_STATE_SELECTED);
	else
		SetItemState(item, 0x0000, wxLIST_STATE_SELECTED);
}

/* ArchiveEntryList::selectAll
 * Selects all items in the list
 *******************************************************************/
void ArchiveEntryList::selectAll() {
	for (int a = 0; a < GetItemCount(); a++)
		SetItemState(a, 0xFFFF, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

/* ArchiveEntryList::clearSelection
 * Deselects all items in the list
 *******************************************************************/
void ArchiveEntryList::clearSelection() {
	for (int a = 0; a < GetItemCount(); a++)
		SetItemState(a, 0x0000, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

/* ArchiveEntryList::OnGetItemText
 * Called when the widget requests the text for [item] at [column]
 *******************************************************************/
string ArchiveEntryList::OnGetItemText(long item, long column) const {
	ArchiveEntry* entry = getEntry(item);

	// If entry doesn't exist, return invalid string
	if (!entry)
		return "INVALID INDEX";

	// Determine what column we want
	int col = columnType(column);

	if (col == AEL_COLUMN_NAME)
		return entry->getName();		// Name column
	else if (col == AEL_COLUMN_SIZE)
		return entry->getSizeString();	// Size column
	else if (col == AEL_COLUMN_TYPE)
		return entry->getTypeString();	// Type column
	else
		return "INVALID COLUMN";		// Invalid column
}

/* ArchiveEntryList::OnGetItemImage
 * Called when the widget requests the image for [item]
 *******************************************************************/
int ArchiveEntryList::OnGetItemImage(long item) const {
	// Get associated entry
	ArchiveEntry* entry = getEntry(item);

	// If entry doesn't exist, return invalid image
	if (!entry)
		return -1;

	return entry->getType()->getIndex();
}

/* ArchiveEntryList::OnGetItemAttr
 * Called when widget requests the attributes (text colour /
 * background colour / font) for [item]
 *******************************************************************/
wxListItemAttr* ArchiveEntryList::OnGetItemAttr(long item) const {
	// Get associated entry
	ArchiveEntry* entry = getEntry(item);

	// Init attributes
	item_attr->SetTextColour(ListView::colourError());

	// If entry doesn't exist, return error colour
	if (!entry)
		return item_attr;

	// Set colour depending on entry state
	switch (entry->getState()) {
	case 1:
		item_attr->SetTextColour(ListView::colourModified());
		break;
	case 2:
		item_attr->SetTextColour(ListView::colourNew());
		break;
	default:
		item_attr->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
		break;
	};

	// Locked state overrides others
	if (entry->isLocked())
		item_attr->SetTextColour(ListView::colourLocked());

	return item_attr;
}

/* ArchiveEntryList::onAnnouncement
 * Called when an announcement is recieved from the archive being
 * managed
 *******************************************************************/
void ArchiveEntryList::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Archive entries were swapped
	if (announcer == archive && event_name == "entries_swapped") {
		int i1, i2;
		wxUIntPtr e1, e2;

		// Check all event data is there
		if (!event_data.read(&i1, sizeof(int)) || !event_data.read(&i2, sizeof(int)) ||
			!event_data.read(&e1, sizeof(wxUIntPtr)) || !event_data.read(&e2, sizeof(wxUIntPtr)))
			return;

		// Swap selection
		long state1 = GetItemState(i1, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);
		long state2 = GetItemState(i2, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);
		SetItemState(i1, state2, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);
		SetItemState(i2, state1, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);

		// Update affected items
		RefreshItem(i1);
		RefreshItem(i2);
	}

	// An entry was added to the archive
	if (announcer == archive && event_name == "entry_added") {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Update item count
		SetItemCount(archive->numEntries());

		// Refresh
		if (index >= (unsigned)GetTopItem())
			Refresh();
	}

	// An entry in the archive was modified
	if (announcer == archive && event_name == "entry_modified") {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Update affected item
		RefreshItem(index);
	}

	// An entry in the archive was removed
	if (announcer == archive && event_name == "entry_removed") {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Deselect item
		SetItemState(index, 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

		// Update item count
		SetItemCount(archive->numEntries());

		// Refresh
		if (index >= (unsigned)GetTopItem())
			Refresh();
	}
}


/*******************************************************************
 * ARCHIVEENTRYLIST EVENTS
 *******************************************************************/

/* ArchiveEntryList::onColumnHeaderRightClick
 * Called when a column header is right clicked
 *******************************************************************/
void ArchiveEntryList::onColumnHeaderRightClick(wxListEvent& e) {
	// Create simple popup menu with options to toggle columns
	wxMenu* popup = new wxMenu();
	popup->AppendCheckItem(AEL_COLUMN_SIZE, "Size", "Show the size column");
	popup->AppendCheckItem(AEL_COLUMN_TYPE, "Type", "Show the type column");
	popup->AppendCheckItem(AEL_HRULES, "Horizontal Rules", "Show horizontal rules between entries");
	popup->AppendCheckItem(AEL_VRULES, "Vertical Rules", "Show vertical rules between columns");
	popup->Check(AEL_COLUMN_SIZE, elist_colsize_show);
	popup->Check(AEL_COLUMN_TYPE, elist_coltype_show);
	popup->Check(AEL_HRULES, elist_hrules);
	popup->Check(AEL_VRULES, elist_vrules);

	// Pop it up
	PopupMenu(popup);
}

/* ArchiveEntryList::onColumnResize
 * Called when a column is resized
 *******************************************************************/
void ArchiveEntryList::onColumnResize(wxListEvent& e) {
	// Save column widths
	int col = 0;
	elist_colname_width = GetColumnWidth(col++);
	if (elist_colsize_show)
		elist_colsize_width = GetColumnWidth(col++);
	if (elist_coltype_show)
		elist_coltype_width = GetColumnWidth(col++);

	// Update width etc
	updateWidth();
	if (GetParent())
		GetParent()->Layout();
}

/* ArchiveEntryList::onMenu
 * Called when a menu item is clicked (from the column header context
 * menu)
 *******************************************************************/
void ArchiveEntryList::onMenu(wxCommandEvent& e) {
	if (e.GetId() == AEL_COLUMN_SIZE) {
		elist_colsize_show = !elist_colsize_show;
		setupColumns();
		updateWidth();
		if (GetParent())
			GetParent()->Layout();
	}
	else if (e.GetId() == AEL_COLUMN_TYPE) {
		elist_coltype_show = !elist_coltype_show;
		setupColumns();
		updateWidth();
		if (GetParent())
			GetParent()->Layout();
	}
	else if (e.GetId() == AEL_HRULES) {
		elist_hrules = !elist_hrules;
		SetSingleStyle(wxLC_HRULES, elist_hrules);
		Refresh();
	}
	else if (e.GetId() == AEL_VRULES) {
		elist_vrules = !elist_vrules;
		SetSingleStyle(wxLC_VRULES, elist_vrules);
		Refresh();
	}
}
