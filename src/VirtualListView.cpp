
#include "Main.h"
#include "WxStuff.h"
#include "VirtualListView.h"
#include "ListView.h"
#include "Console.h"


wxDEFINE_EVENT(EVT_VLV_SELECTION_CHANGED, wxCommandEvent);


VirtualListView::VirtualListView(wxWindow* parent)
: wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL) {
	item_attr = new wxListItemAttr();
	last_focus = 0;

	// Bind events
#ifdef __WXGTK__
	Bind(wxEVT_LEFT_DOWN, &VirtualListView::onMouseLeftDown, this);
	Bind(wxEVT_KEY_DOWN, &VirtualListView::onKeyDown, this);
#endif
	Bind(wxEVT_COMMAND_LIST_COL_END_DRAG, &VirtualListView::onColumnResize, this);
}

VirtualListView::~VirtualListView() {
	delete item_attr;
}

void VirtualListView::sendSelectionChangedEvent() {
	wxCommandEvent evt(EVT_VLV_SELECTION_CHANGED, GetId());
	ProcessWindowEvent(evt);
}

/* VirtualListView::updateWidth
 * Updates the list's minimum requested width to allow the
 * widget to be shown with no horizontal scrollbar
 *******************************************************************/
void VirtualListView::updateWidth() {
	// Get total column width
	int width = 8;
	for (int a = 0; a < GetColumnCount(); a++)
		width += GetColumnWidth(a);

	// Always leave room for the scrollbar (wxWidgets is silly)
	width += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);

	// Set widget size
	SetSizeHints(width, -1);
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

void VirtualListView::selectItems(long start, long end, bool select) {
	// Check/correct indices
	if (start > end) {
		long temp = start;
		start = end;
		end = temp;
	}
	if (start < 0) start = 0;
	if (end >= GetItemCount()) end = GetItemCount() - 1;

	// Go through range
	for (long a = start; a <= end; a++) {
		// Select/deselect the item
		if (select)
			SetItemState(a, 0xFFFF, wxLIST_STATE_SELECTED);
		else
			SetItemState(a, 0x0000, wxLIST_STATE_SELECTED);
	}
}

void VirtualListView::selectAll() {
	for (int a = 0; a < GetItemCount(); a++)
		SetItemState(a, 0xFFFF, wxLIST_STATE_SELECTED);

	sendSelectionChangedEvent();
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

void VirtualListView::focusItem(long item, bool focus) {
	// Check item id is in range
	if (item >= GetItemCount())
		return;

	// Select/deselect the item
	if (focus) {
		SetItemState(item, 0xFFFF, wxLIST_STATE_FOCUSED);
		last_focus = item;
	}
	else
		SetItemState(item, 0x0000, wxLIST_STATE_FOCUSED);
}

long VirtualListView::getFocus() {
	return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
}



/* VirtualListView::onColumnResize
 * Called when a column is resized
 *******************************************************************/
void VirtualListView::onColumnResize(wxListEvent& e) {
	// Update width etc
	updateWidth();
	if (GetParent())
		GetParent()->Layout();
}

void VirtualListView::onMouseLeftDown(wxMouseEvent& e) {
	// Get item at click position
	int flags = 0;
	long item = this->HitTest(wxPoint(e.GetX(), e.GetY()), flags);
	if (flags & wxLIST_HITTEST_ONITEM) {
		if (e.ShiftDown()) {
			// Shift+left click: Add all items between the focused item and the item that was clicked to the selection
			long focus = getFocus();
			if (focus < 0) focus = last_focus;		// If no current focus, go with last focused item
			selectItems(item, focus);
			focusItem(item);
			sendSelectionChangedEvent();
		}
		else if (e.ControlDown()) {
			// Ctrl+left click: Toggle the selection status of the clicked item
			bool selected = !!(GetItemState(item, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED);
			selectItem(item, !selected);
			focusItem(item, !selected);
			sendSelectionChangedEvent();
		}
		else {
			// Just a left click, select+focus the item
			clearSelection();
			selectItem(item);
			focusItem(item);
			sendSelectionChangedEvent();
		}
	}
}

void VirtualListView::onKeyDown(wxKeyEvent& e) {
	// up/down - sets selection+focus to item above/below current focus
	// shift+up/down - same as up/down but doesn't clear previous selection
	// ctrl+up/down - does nothing
	if (e.GetKeyCode() == WXK_UP) {
		if (e.ShiftDown()) {
			long focus = getFocus();
			if (focus < 0) focus = last_focus;		// If no current focus, go with last focused item
			if (focus > 0) {
				focusItem(focus, false);
				selectItem(focus-1);
				focusItem(focus-1);
				EnsureVisible(focus-1);
				sendSelectionChangedEvent();
			}
		}
		else if (!e.AltDown() && !e.ControlDown()) {
			long focus = getFocus();
			if (focus < 0) focus = last_focus;		// If no current focus, go with last focused item
			if (focus > 0) {
				clearSelection();
				focusItem(focus, false);
				selectItem(focus-1);
				focusItem(focus-1);
				EnsureVisible(focus-1);
				sendSelectionChangedEvent();
			}
		}
	}
	else if (e.GetKeyCode() == WXK_DOWN) {
		if (e.ShiftDown()) {
			long focus = getFocus();
			if (focus < 0) focus = last_focus;		// If no current focus, go with last focused item
			if (focus < GetItemCount() - 1) {
				focusItem(focus, false);
				selectItem(focus+1);
				focusItem(focus+1);
				EnsureVisible(focus+1);
				sendSelectionChangedEvent();
			}
		}
		else if (!e.AltDown() && !e.ControlDown()) {
			long focus = getFocus();
			if (focus < 0) focus = last_focus;		// If no current focus, go with last focused item
			if (focus < GetItemCount() - 1) {
				clearSelection();
				focusItem(focus, false);
				selectItem(focus+1);
				focusItem(focus+1);
				EnsureVisible(focus+1);
				sendSelectionChangedEvent();
			}
		}
	}
	else
		e.Skip();
}




void c_test_vlv(vector<string> args) {
	wxDialog dlg(NULL, -1, "VLV!", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	dlg.SetSizer(hbox);

	VirtualListView* vlv = new VirtualListView(&dlg);
	hbox->Add(vlv, 1, wxEXPAND|wxALL, 4);
	vlv->SetItemCount(100);
	vlv->InsertColumn(0, "Test");

	dlg.ShowModal();
}
ConsoleCommand test_vlv("test_vlv", &c_test_vlv, 0);
