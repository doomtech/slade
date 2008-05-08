
#ifndef __TTYPE_SELECT_H__
#define __TTYPE_SELECT_H__

#include "wx_stuff.h"
#include <wx/treectrl.h>

class TTypeSelectDialog : public wxDialog
{
private:
	wxTreeCtrl	*tree;
	int			selected_val;

public:
	enum
	{
		SD_TREE,
	};

	TTypeSelectDialog(int init);
	~TTypeSelectDialog();

	int get_type() { return selected_val; }

	void tree_selection_changed(wxTreeEvent &event);
	void tree_item_activated(wxTreeEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__TTYPE_SELECT_H__
