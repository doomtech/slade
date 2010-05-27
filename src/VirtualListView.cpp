
#include "Main.h"
#include "VirtualListView.h"
#include "ListView.h"


wxDEFINE_EVENT(EVT_VLV_SELECTION_CHANGED, wxCommandEvent);


VirtualListView::VirtualListView(wxWindow* parent)
: wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL) {
	item_attr = new wxListItemAttr();
}

VirtualListView::~VirtualListView() {
	delete item_attr;
}

void VirtualListView::sendSelectionChangedEvent() {
	wxCommandEvent evt(EVT_VLV_SELECTION_CHANGED, GetId());
	ProcessWindowEvent(evt);
}

void VirtualListView::selectItem(long item, bool select) {
	// Check item id is in range
	if (item >= GetItemCount())
		return;

	// Select/deselect the item
	if (select)
		SetItemState(item, 0xFFFF, wxLIST_STATE_SELECTED);
	else
		SetItemState(item, 0x0000, wxLIST_STATE_SELECTED);
}

void VirtualListView::selectAll() {
	for (int a = 0; a < GetItemCount(); a++)
		SetItemState(a, 0xFFFF, wxLIST_STATE_SELECTED);
}

void VirtualListView::clearSelection() {
	for (int a = 0; a < GetItemCount(); a++)
		SetItemState(a, 0x0000, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

vector<long> VirtualListView::getSelection() {
	// Init return array
	vector<long> ret;

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

long VirtualListView::getFirstSelected() {
	return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

long VirtualListView::getLastSelected() {
	// Go through all items
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		long index = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// Exit loop if there is no selected item after the current
		if (index == -1)
			break;

		// Otherwise set the item to this index and keep going
		item = index;
	}

	return item;
}




void VirtualListView::onMouseLeftDown(wxMouseEvent& e) {
	// Shift+click - seems to select all between the focus and what was clicked, sets new focus to clicked item
	// Ctrl+click - toggles selection on clicked item, clears focus if deselected (and sets if selected)
	// no focus acts as if topmost item is focused

	// Get item at click position
	int flags = 0;
	long item = this->HitTest(wxPoint(e.GetX(), e.GetY()), flags);
	if (flags & wxLIST_HITTEST_ONITEM) {
		if (e.ShiftDown()) {

		}
		else if (e.ControlDown()) {

		}
		else {

		}
	}
}

void VirtualListView::onKeyDown(wxKeyEvent& e) {
	// up/down - sets selection+focus to item above/below current focus
	// shift+up/down - same as up/down but doesn't clear previous selection
	// ctrl+up/down - does nothing
	e.Skip();
}
