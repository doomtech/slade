
#ifndef __SPECIAL_SELECT_H__
#define __SPECIAL_SELECT_H__

#include "wx_stuff.h"
#include <wx/treectrl.h>

class SpecialSelectDialog : public wxDialog
{
private:
	wxTreeCtrl	*tree;
	int			selected_val;

public:
	enum
	{
		SD_TREE,
	};

	SpecialSelectDialog(int init);
	~SpecialSelectDialog();

	int get_special() { return selected_val; }

	void tree_selection_changed(wxTreeEvent &event);
	void tree_item_activated(wxTreeEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__SPECIAL_SELECT_H__
