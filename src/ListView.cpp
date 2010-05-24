
#include "Main.h"
#include "WxStuff.h"
#include "ListView.h"


wxColor col_new(0, 150, 0);
wxColor col_modified(0, 80, 180);
wxColor col_locked(180, 50, 0);
wxColor col_error(230, 30, 0);


ListView::ListView(wxWindow* parent, int id, long style)
: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, style) {
	icons = true;
	update_width = true;
}

ListView::~ListView() {
}

bool ListView::addItem(int index, string text) {
	// Check index
	if (index < 0) index = 0;
	if (index > GetItemCount()) index = GetItemCount();

	// Add the item to the list
	wxListItem li;
	li.SetId(index);
	li.SetColumn(0);
	li.SetText(text);
	InsertItem(li);

	// Update list size
	updateSize();

	return true;
}

bool ListView::addItem(int index, wxArrayString text) {
	// Check index
	if (index < 0) index = 0;
	if (index > GetItemCount()) index = GetItemCount();

	// Add the item to the list
	wxListItem li;
	li.SetId(index);
	InsertItem(li);

	// Set item text for each column
	for (size_t a = 0; a < text.size(); a++)
		setItemText(index, a, text[a]);

	// Update list size
	updateSize();

	return true;
}

int cmp_int(int* a, int* b) {
	if (*a > *b) return 1;
	else if (*a < *b) return -1;
	else return 0;
}

bool ListView::deleteItems(wxArrayInt items) {
	// Sort items list
	items.Sort(cmp_int);

	// Go through list backwards and delete each item
	for (int a = items.size() - 1; a >= 0; a--)
		DeleteItem(items[a]);

	return true;
}

bool ListView::setItemStatus(int item, int status) {
	// Check item id is in range
	if (item >= GetItemCount())
		return false;

	// If given a negative item id, set all items in the list to the given status
	if (item < 0) {
		for (int a = 0; a < GetItemCount(); a++)
			setItemStatus(a, status);

		return true;
	}

	// Set item text colour to given status colour
	switch(status) {
		case LV_STATUS_NORMAL:
			SetItemTextColour(item, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
			break;
		case LV_STATUS_MODIFIED:
			SetItemTextColour(item, col_modified);
			break;
		case LV_STATUS_NEW:
			SetItemTextColour(item, col_new);
			break;
		case LV_STATUS_LOCKED:
			SetItemTextColour(item, col_locked);
			break;
		case LV_STATUS_ERROR:
			SetItemTextColour(item, col_error);
	}

	return true;
}

bool ListView::setItemText(int item, int column, string text) {
	// Check if column is in range
	if (column < 0 || column >= GetColumnCount())
		return false;

	// Check item id is in range
	if (item >= GetItemCount())
		return false;

	// If negative item given, apply to all items
	if (item < 0) {
		for (int a = 0; a < GetItemCount(); a++)
			setItemText(a, column, text);

		return true;
	}

	// Set the item text
	wxListItem li;
	li.SetId(item);
	li.SetColumn(column);
	li.SetText(text);
	SetItem(li);

	// Update widget size
	updateSize();

	return true;
}

void ListView::clearSelection() {
	for (int a = 0; a < GetItemCount(); a++)
		SetItemState(a, 0x0000, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

bool ListView::selectItem(int item, bool focus) {
	// Check item id is in range
	if (item >= GetItemCount())
		return false;

	// If negative item given, select all items
	if (item < 0) {
		for (int a = 0; a < GetItemCount(); a++)
			SetItemState(a, 0xFFFF, wxLIST_STATE_SELECTED);

		return true;
	}

	// Select the item (and focus if needed)
	if (focus)
		SetItemState(item, 0xFFFF, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	else
		SetItemState(item, 0xFFFF, wxLIST_STATE_SELECTED);

	return true;
}

bool ListView::deSelectItem(int item) {
	// Check item id is in range
	if (item >= GetItemCount())
		return false;

	// If negative item given, deselect all items
	if (item < 0) {
		clearSelection();
		return true;
	}

	// DeSelect the item
	SetItemState(item, 0x0000, wxLIST_STATE_SELECTED);

	return true;
}

wxArrayInt ListView::selectedItems() {
	// Init return array
	wxArrayInt ret;

	// Go through all items
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected index to the vector
		ret.Add(item);
	}

	return ret;
}

bool ListView::showItem(int item) {
	// Check item id is in range
	if (item < 0 || item >= GetItemCount())
		return false;

	EnsureVisible(item);

	return true;
}

bool ListView::swapItems(int item1, int item2) {
	// Check item id's are in range
	if (item1 < 0 || item1 >= GetItemCount() || item2 < 0 || item2 >= GetItemCount())
		return false;

	// Get item 1 info
	wxListItem i1;
	i1.SetId(item1);
	i1.SetMask(0xFFFF);
	GetItem(i1);
	long state1 = GetItemState(item1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

	// Get item 2 info
	wxListItem i2;
	i2.SetId(item2);
	i2.SetMask(0xFFFF);
	GetItem(i2);
	long state2 = GetItemState(item2, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

	// Swap the items
	i1.SetId(item2);
	i2.SetId(item1);
	SetItem(i1);
	SetItem(i2);
	SetItemState(item1, state2, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	SetItemState(item2, state1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

	return true;
}

bool ListView::updateSize() {
	// Update column widths if enabled
	if (update_width) {
		for (int a = 0; a < GetColumnCount(); a++) {
			// Get header width
			int maxsize = 0;
			SetColumnWidth(a, (a == GetColumnCount() -1 ? wxLIST_AUTOSIZE : wxLIST_AUTOSIZE_USEHEADER));
			maxsize = GetColumnWidth(a);

			// Autosize column
			SetColumnWidth(a, wxLIST_AUTOSIZE);

			// If header width is larger, use that
			if (maxsize > GetColumnWidth(a))
				SetColumnWidth(a, maxsize);

			// Minimum size of 24
			if (GetColumnWidth(a) < 24)
				SetColumnWidth(a, 24);
		}

		// Add extra width to the first column in linux as wxLIST_AUTOSIZE seems to ignore listitem images on wxGTK
		if (icons) {
			#ifdef __WXGTK__
			SetColumnWidth(0, GetColumnWidth(0) + 20);
			#endif
		}
	}

	// Get total column width
	int width = 8;
	for (int a = 0; a < GetColumnCount(); a++)
		width += GetColumnWidth(a);

	// Always leave room for the scrollbar (wxWidgets is silly)
	width += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);

	// Set widget size
	SetSizeHints(width, -1);

	return true;
}

wxColour ListView::colourModified() {
	return col_modified;
}

wxColour ListView::colourNew() {
	return col_new;
}

wxColour ListView::colourLocked() {
	return col_locked;
}

wxColour ListView::colourError() {
	return col_error;
}
