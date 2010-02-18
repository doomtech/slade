
#ifndef __LISTVIEW_H__
#define __LISTVIEW_H__

#include <wx/listctrl.h>

enum {
	LV_STATUS_NORMAL,
	LV_STATUS_MODIFIED,
	LV_STATUS_NEW,
	LV_STATUS_LOCKED,
};

class ListView : public wxListCtrl {
private:
	bool		icons;
	bool		update_width;

public:
	ListView(wxWindow* parent, int id);
	~ListView();

	bool	showIcons() { return icons; }
	void	showIcons(bool show) { icons = show; }
	bool	enableSizeUpdate() { return update_width; }
	void	enableSizeUpdate(bool update) { update_width = update; }

	bool	addItem(int index, string text);
	bool	addItem(int index, wxArrayString text);

	bool	setItemStatus(int item, int status);
	bool	setItemText(int item, int column, string text);

	void	clearSelection();
	bool	selectItem(int item, bool focus = true);

	bool	showItem(int item);

	bool	updateSize();
};

#endif//__LISTVIEW_H__
