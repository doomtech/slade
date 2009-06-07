
#ifndef __ENTRYLISTPANEL_H__
#define __ENTRYLISTPANEL_H__

#include <wx/listctrl.h>
#include "ListenerAnnouncer.h"

class Archive;
class EntryListPanel;

class EntryList : public wxListCtrl {
private:
	EntryListPanel*	parent;

public:
	EntryList(EntryListPanel *parent, int id);
	~EntryList();

	bool	updateEntry(int index, bool update_colsize = true);
	int		getWidth();
};

class EntryListPanel : public wxPanel, Listener {
protected:
	Archive*	archive;
	EntryList*	entry_list;

public:
	enum {
		ENTRY_LIST,
	};

	EntryListPanel(wxWindow *parent, int id, Archive* archive);
	virtual ~EntryListPanel();

	virtual void			populateEntryList();
	ArchiveEntry*			getFocusedEntry();
	int						getFocus();
	vector<ArchiveEntry*>	getSelectedEntries();
	vector<int>				getSelection();
	int						getLastSelected();
	ArchiveEntry*			getLastSelectedEntry();
	virtual bool			swapItems(int item1, int item2, ArchiveEntry* e1 = NULL, ArchiveEntry* e2 = NULL);
	virtual bool			addEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	virtual bool			updateEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	virtual bool			removeEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	int						getEntryListItem(ArchiveEntry* entry);

	virtual ArchiveEntry*	newEntry();
	virtual ArchiveEntry*	newEntryFromFile();
	virtual bool			moveUp();
	virtual bool			moveDown();

	virtual void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {}

	// Events
	virtual void	onEntryListChange(wxListEvent &event);
	virtual void	onEntryListActivated(wxListEvent &event);
	virtual void	onEntryListEditLabel(wxListEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__ENTRYLISTPANEL_H__
