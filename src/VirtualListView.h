
#ifndef __VIRTUAL_LIST_VIEW_H__
#define __VIRTUAL_LIST_VIEW_H__

#include <wx/listctrl.h>

// An event to indicate when the selection has changed
wxDECLARE_EVENT(EVT_VLV_SELECTION_CHANGED, wxCommandEvent);

class VirtualListView : public wxListCtrl {
private:
	long	last_focus;
	string	search;
	int		col_search;

	void	sendSelectionChangedEvent();

protected:
	wxListItemAttr*	item_attr;

	virtual string	getItemText(long item, long column) const { return "UNDEFINED"; }
	virtual int		getItemIcon(long item) const { return -1; }
	virtual void	updateItemAttr(long item) const {}

	// Virtual wxListCtrl overrides
	string			OnGetItemText(long item, long column) const { return getItemText(item, column); }
	int				OnGetItemImage(long item) const { return getItemIcon(item); }
	wxListItemAttr*	OnGetItemAttr(long item) const { updateItemAttr(item); return item_attr; }

public:
	VirtualListView(wxWindow* parent);
	~VirtualListView();

	void	setSearchColumn(int col) { col_search = col; }

	// Selection
	void			selectItem(long item, bool select = true);
	void			selectItems(long start, long end, bool select = true);
	void			selectAll();
	void			clearSelection();
	vector<long>	getSelection();
	long			getFirstSelected();
	long			getLastSelected();

	// Focus
	void			focusItem(long item, bool focus = true);
	long			getFocus();
	bool			lookForSearchEntryFrom(int focus);

	// Layout
	void			updateWidth();
	virtual void	updateList(bool clear = false) { SetItemCount(0); }

	// Events
	void	onColumnResize(wxListEvent& e);
	void	onMouseLeftDown(wxMouseEvent& e);
	void	onKeyDown(wxKeyEvent& e);
	void	onKeyChar(wxKeyEvent& e);
};

#endif//__VIRTUAL_LIST_VIEW_H__
