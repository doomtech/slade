
#ifndef __LUMPLISTPANEL_H__
#define __LUMPLISTPANEL_H__

#include <wx/listctrl.h>

class LumpListPanel;
class Wad;

class LumpList : public wxListCtrl
{
private:
	LumpListPanel*	parent;

public:
	LumpList(LumpListPanel *parent, int id);
	~LumpList();

	bool updateEntry(int index);

	/*

	// Events
	void onKeyDown(wxKeyEvent &event);
	void onBeginLabelEdit(wxListEvent &event);
	void onEndLabelEdit(wxListEvent &event);

	DECLARE_EVENT_TABLE()
	*/
};


class LumpListPanel : public wxPanel
{
private:
	Wad*		wad;
	LumpList*	lump_list;

public:
	LumpListPanel(wxWindow *parent, int id, Wad* wad);
	~LumpListPanel();

	void	populateLumpList();
};

#endif //__LUMPLISTPANEL_H__
