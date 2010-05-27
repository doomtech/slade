
#ifndef __VIRTUAL_LIST_VIEW_H__
#define __VIRTUAL_LIST_VIEW_H__

#include <wx/listctrl.h>

// An event to indicate when the selection has changed
wxDECLARE_EVENT(EVT_VLV_SELECTION_CHANGED, wxCommandEvent);

class VirtualListView : public wxListCtrl {
private:
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

	// Selection
	void			selectItem(long item, bool select = true);
	void			selectAll();
	void			clearSelection();
	vector<long>	getSelection();
	long			getFirstSelected();
	long			getLastSelected();

	// Events
	void	onMouseEvent(wxMouseEvent& e);
	void	onMouseLeftDown(wxMouseEvent& e);
	void	onKeyDown(wxKeyEvent& e);
};

#endif//__VIRTUAL_LIST_VIEW_H__
