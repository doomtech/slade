
#ifndef __ENTRYLISTPANEL_H__
#define __ENTRYLISTPANEL_H__

#include <wx/listctrl.h>

class Archive;
class EntryListPanel;

class EntryList : public wxListCtrl
{
private:
	EntryListPanel*	parent;

public:

	EntryList(EntryListPanel *parent, int id);
	~EntryList();

	bool	updateEntry(int index);
	int		getWidth();
};

class EntryListPanel : public wxPanel
{
private:
	Archive*	archive;
	EntryList*	entry_list;

public:
	EntryListPanel(wxWindow *parent, int id, Archive* archive);
	~EntryListPanel();

	void	populateEntryList();
};

#endif //__ENTRYLISTPANEL_H__
