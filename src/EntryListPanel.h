
#ifndef __ENTRYLISTPANEL_H__
#define __ENTRYLISTPANEL_H__

#include <wx/listctrl.h>

class Archive;
class EntryListPanel;

class EntryList : public wxListCtrl {
private:
	EntryListPanel*	parent;

public:

	EntryList(EntryListPanel *parent, int id);
	~EntryList();

	bool	updateEntry(int index);
	int		getWidth();
};

class EntryListPanel : public wxPanel {
private:
	Archive*	archive;
	EntryList*	entry_list;
	long		focus_index;

public:
	enum {
		ENTRY_LIST,
	};

	EntryListPanel(wxWindow *parent, int id, Archive* archive);
	~EntryListPanel();

	void					populateEntryList();
	ArchiveEntry*			getFocusedEntry();
	vector<ArchiveEntry*>	getSelectedEntries();
	vector<int>				getSelection();
	bool					swapItems(int item1, int item2);

	bool	moveUp();
	bool	moveDown();

	// Events
	void	onEntryListChange(wxListEvent &event);
	void	onEntryListActivated(wxListEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__ENTRYLISTPANEL_H__
