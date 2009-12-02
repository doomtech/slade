
#include "Main.h"
#include "WxStuff.h"
#include "ListView.h"


wxColor col_new(0, 150, 0);
wxColor col_modified(0, 80, 180);
wxColor col_locked(180, 50, 0);


ListView::ListView(wxWindow* parent, int id)
: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT) {
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

bool ListView::updateSize() {
	// Update column widths if enabled
	if (update_width) {
		for (int a = 0; a < GetColumnCount(); a++) {
			// Autosize column
			SetColumnWidth(a, wxLIST_AUTOSIZE);

			// Minimum size of 32
			if (GetColumnWidth(a) < 32)
				SetColumnWidth(a, 32);
		}

		// Add extra width to the first column in linux as wxLIST_AUTOSIZE seems to ignore listitem images on wxGTK
		if (icons) {
			#ifdef __WXGTK__
			SetColumnWidth(0, GetColumnWidth(0) + 20);
			#endif
		}
	}

	// Get total column width
	int width = 4;
	for (int a = 0; a < GetColumnCount(); a++)
		width += GetColumnWidth(a);

	// Always leave room for the scrollbar (wxWidgets is silly)
	width += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);

	// Set widget size
	SetSizeHints(width, -1);

	return true;
}
