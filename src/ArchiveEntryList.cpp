
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ArchiveEntryList.cpp
 * Description: A list widget that shows all entries in an archive.
 *              Keeps in sync with its associated archive
 *              automatically.
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
CVAR(Bool, elist_filter_dirs, false, CVAR_SAVE)
wxDEFINE_EVENT(EVT_AEL_DIR_CHANGED, wxCommandEvent);


/*******************************************************************
 * ARCHIVEENTRYLIST CLASS FUNCTIONS
 *******************************************************************/

/* ArchiveEntryList::ArchiveEntryList
 * ArchiveEntryList class constructor
 *******************************************************************/
ArchiveEntryList::ArchiveEntryList(wxWindow* parent) : VirtualListView(parent) {
	// Init variables
	archive = NULL;
	filter_active = false;
	filter_name = "";
	filter_category = "";
	current_dir = false;
	show_dir_back = false;

	// Create dummy 'up folder' entry
	entry_dir_back = new ArchiveEntry();
	entry_dir_back->setType(EntryType::folderType());
	entry_dir_back->setState(0);
	entry_dir_back->setName("..");

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
	Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ArchiveEntryList::onListItemActivated, this);

	// Setup flags
	SetSingleStyle(wxLC_HRULES, elist_hrules);
	SetSingleStyle(wxLC_VRULES, elist_vrules);
}

/* ArchiveEntryList::~ArchiveEntryList
 * ArchiveEntryList class destructor
 *******************************************************************/
ArchiveEntryList::~ArchiveEntryList() {
}

/* ArchiveEntryList::getItemText
 * Called when the widget requests the text for [item] at [column]
 *******************************************************************/
string ArchiveEntryList::getItemText(long item, long column) const {
	// Get entry
	ArchiveEntry* entry = getEntry(item);

	// Check entry
	if (!entry)
		return "INVALID INDEX";

	// Determine what column we want
	int col = columnType(column);

	if (col == AEL_COLUMN_NAME)
		return entry->getName();	// Name column
	else if (col == AEL_COLUMN_SIZE) {
		// Size column

		if (entry->getType() == EntryType::folderType()) {
			// Entry is a folder, return the number of entries+subdirectories in it
			ArchiveTreeNode* dir = NULL;

			// Get selected directory
			if (entry == entry_dir_back)
				dir = (ArchiveTreeNode*)current_dir->getParent();	// If it's the 'back directory', get the current dir's parent
			else
				dir = dir = archive->getDir(entry->getName(), current_dir);

			// If it's null, return error
			if (!dir)
				return "INVALID DIRECTORY";

			// Return the number of items in the directory
			return s_fmt("%d entries", dir->numEntries() + dir->nChildren());
		}
		else
			return entry->getSizeString();	// Not a folder, just return the normal size string
	}
	else if (col == AEL_COLUMN_TYPE)
		return entry->getTypeString();	// Type column
	else
		return "INVALID COLUMN";		// Invalid column
}

/* ArchiveEntryList::getItemIcon
 * Called when the widget requests the icon for [item]
 *******************************************************************/
int ArchiveEntryList::getItemIcon(long item) const {
	// Get associated entry
	ArchiveEntry* entry = getEntry(item);

	// If entry doesn't exist, return invalid image
	if (!entry)
		return -1;

	return entry->getType()->getIndex();
}

/* ArchiveEntryList::updateItemAttr
 * Called when widget requests the attributes (text colour /
 * background colour / font) for [item]
 *******************************************************************/
void ArchiveEntryList::updateItemAttr(long item) const {
	// Get associated entry
	ArchiveEntry* entry = getEntry(item);

	// Init attributes
	item_attr->SetTextColour(ListView::colourError());

	// If entry doesn't exist, return error colour
	if (!entry)
		return;

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
		// Listen to it
		listenTo(archive);

		// Open root directory
		current_dir = archive->getRoot();
		updateList();
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
void ArchiveEntryList::updateList() {
	// If no current directory, set size to 0
	if (!current_dir) {
		SetItemCount(0);
		Refresh();
		return;
	}

	// Determine if we need a 'back folder' entry
	int back_folder = 0;
	if (show_dir_back && current_dir->getParent())
		back_folder = 1;

	// Update list
	if (filter_active)
		SetItemCount(filter.size());
	else
		SetItemCount(current_dir->numEntries() + current_dir->nChildren() + back_folder);

	Refresh();
}

/* ArchiveEntryList::filterList
 * Filters the list to only entries and directories with names
 * matching [filter], and with type categories matching [category].
 *******************************************************************/
void ArchiveEntryList::filterList(string filter, string category) {
	// Update variables
	filter_name = filter;
	filter_category = category;

	applyFilter();
}

/* ArchiveEntryList::applyFilter
 * Applies the current filter(s) to the list
 *******************************************************************/
void ArchiveEntryList::applyFilter() {
	// Get current selection
	vector<ArchiveEntry*> selection = getSelectedEntries();
	clearSelection();

	// Disable filter initially
	filter_active = false;

	// Check if any filters were given
	if (filter_name.IsEmpty() && filter_category.IsEmpty()) {
		// No filter, just refresh the list
		updateList();

		// Update selection
		for (unsigned a = 0; a < current_dir->numEntries(); a++) {
			for (unsigned b = 0; b < selection.size(); b++) {
				if (current_dir->getEntry(a) == selection[b]) {
					selectItem(a);
					break;
				}
			}
		}

		return;
	}

	// Clear current filter list
	filter.clear();

	// Filter by category
	unsigned index = 0;
	ArchiveEntry* entry = getEntry(index);
	while (entry) {
		if (filter_category.IsEmpty() || entry->getType() == EntryType::folderType())
			filter.push_back(index);	// If no category specified, just add all entries to the filter
		else {
			// Check for category match
			if (s_cmpnocase(entry->getType()->getCategory(), filter_category))
				filter.push_back(index);
		}

		entry = getEntry(++index);
	}

	// Now filter by name if needed
	if (!filter_name.IsEmpty()) {
		// Convert filter to lowercase (to avoid case-sensitivity)
		string filterstring = filter_name.Lower() + "*";

		// Go through filtered list
		for (unsigned a = 0; a < filter.size(); a++) {
			entry = getEntry(filter[a]);

			// Don't filter folders if !elist_filter_dirs
			if (!elist_filter_dirs && entry->getType() == EntryType::folderType())
				continue;

			// Check for name match with filter
			if (entry == entry_dir_back || entry->getName().Lower().Matches(filterstring))
				continue;

			// No match, remove from filtered list
			filter.erase(filter.begin() + a);
			a--;
		}
	}

	// Enable the filter
	filter_active = true;

	// Update the list
	updateList();

	// Update selection
	for (unsigned a = 0; a < filter.size(); a++) {
		for (unsigned b = 0; b < selection.size(); b++) {
			if (getEntry(a) == selection[b]) {
				selectItem(a);
				break;
			}
		}
	}
}


/* ArchiveEntryList::setDir
 * Opens the given directory (if it exists)
 *******************************************************************/
bool ArchiveEntryList::setDir(ArchiveTreeNode* dir) {
	// If it doesn't exist, do nothing
	if (!dir)
		return false;

	// Set current dir
	current_dir = dir;

	// Clear current selection
	clearSelection();

	// Update filter
	applyFilter();

	// Update list
	updateList();

	// Fire event
	wxCommandEvent evt(EVT_AEL_DIR_CHANGED, GetId());
	ProcessWindowEvent(evt);

	return true;
}

/* ArchiveEntryList::goUpDir
 * Opens the parent directory of the current directory (if it exists)
 *******************************************************************/
bool ArchiveEntryList::goUpDir() {
	// Get parent directory
	return (setDir((ArchiveTreeNode*)current_dir->getParent()));
}

/* ArchiveEntryList::entriesBegin
 * Returns the index of the first list item that is an entry (rather
 * than a directory), or -1 if no directory/archive is open)
 *******************************************************************/
int ArchiveEntryList::entriesBegin() {
	// Check directory is open
	if (!current_dir)
		return -1;

	// Determine first entry index
	int index = 0;
	if (show_dir_back && current_dir->getParent())		// Offset if '..' item exists
		index++;
	index += current_dir->nChildren();					// Offset by number of subdirs

	return index;
}

/* ArchiveEntryList::getEntry
 * Returns the ArchiveEntry associated with the list item at [index].
 * Returns NULL if the index is out of bounds or no archive is open
 *******************************************************************/
ArchiveEntry* ArchiveEntryList::getEntry(int index) const {
	// Check index & archive
	if (index < 0 || !archive)
		return NULL;

	// Check if filtering is active
	if (filter_active) {
		// If it is, modify index for filtered list
		if (index < 0 || (unsigned) index >= filter.size())
			return NULL;

		index = filter[index];
	}

	// Index modifier if 'up folder' entry exists
	if (show_dir_back && current_dir->getParent()) {
		if (index == 0)
			return entry_dir_back;
		else
			index--;
	}

	// Subdirectories
	int subdirs = current_dir->nChildren();
	if (index < subdirs)
		return ((ArchiveTreeNode*)(current_dir->getChild(index)))->getDirEntry();

	// Entries
	if ((unsigned)index < subdirs + current_dir->numEntries())
		return current_dir->getEntry(index - subdirs);

	// Out of bounds
	return NULL;
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
	vector<long> selection = getSelection();

	// Go through selection and add associated entries to the return vector
	ArchiveEntry* entry = NULL;
	for (size_t a = 0; a < selection.size(); a++) {
		entry = getEntry(selection[a]);
		if (entry && entry->getType() != EntryType::folderType())
			ret.push_back(entry);
	}

	return ret;
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

/* ArchiveEntryList::getSelectedDirectories
 * Returns a vector of all currently selected directories
 *******************************************************************/
vector<ArchiveTreeNode*> ArchiveEntryList::getSelectedDirectories() {
	vector<ArchiveTreeNode*> ret;

	// Get all selected items
	vector<long> selection = getSelection();

	// Go through the selection
	for (size_t a = 0; a < selection.size(); a++) {
		ArchiveEntry* entry = getEntry(selection[a]);

		// If the selected entry is the 'back folder', ignore it
		if (entry == entry_dir_back)
			continue;
		else if (entry->getType() == EntryType::folderType()) {
			// If the entry is a folder type, get its ArchiveTreeNode counterpart
			ArchiveTreeNode* dir = archive->getDir(entry->getName(), current_dir);

			// Add it to the return list
			if (dir)
				ret.push_back(dir);
		}
	}

	return ret;
}

/* ArchiveEntryList::onAnnouncement
 * Called when an announcement is recieved from the archive being
 * managed
 *******************************************************************/
void ArchiveEntryList::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {

	if (announcer == archive) {
		// Since refreshing the list is relatively fast, just refresh it on any change
		updateList();
		applyFilter();
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

	e.Skip();
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

/* ArchiveEntryList::onListItemActivated
 * Called when a list item is 'activated' (double-click or enter)
 *******************************************************************/
void ArchiveEntryList::onListItemActivated(wxListEvent& e) {
	// Get item entry
	ArchiveEntry* entry = getEntry(e.GetIndex());

	// Do nothing if NULL (shouldn't be)
	if (!entry)
		return;

	// If it's a folder, open it
	if (entry->getType() == EntryType::folderType()) {
		// Get directory to open
		ArchiveTreeNode* dir = NULL;
		if (entry == entry_dir_back)
			dir = (ArchiveTreeNode*)current_dir->getParent();	// 'Back directory' entry, open current dir's parent
		else
			dir = archive->getDir(entry->getName(), current_dir);

		// Check it exists (really should)
		if (!dir) {
			wxLogMessage("Error: Trying to open nonexistant directory");
			return;
		}

		// Set current dir
		setDir(dir);
	}
	else
		e.Skip();
}
