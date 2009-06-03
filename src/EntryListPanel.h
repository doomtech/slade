
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
	virtual bool			swapItems(int item1, int item2);
	virtual bool			addEntry(DWORD archive_index);
	virtual bool			updateEntry(DWORD archive_index);
	virtual bool			removeEntry(DWORD archive_index);
	int						getEntryListItem(ArchiveEntry* entry);

	virtual ArchiveEntry*	newEntry(string name);
	virtual ArchiveEntry*	newEntryFromFile(string name, string filename);
	bool					moveUp();
	bool					moveDown();

	// Events
	virtual void	onEntryListChange(wxListEvent &event);
	virtual void	onEntryListActivated(wxListEvent &event);
	virtual void	onEntryListEditLabel(wxListEvent &event);

	DECLARE_EVENT_TABLE()
};

class ZipEntryListPanel : public EntryListPanel {
private:
	void*			cur_directory;
	ArchiveEntry*	dummy_folder_entry;

public:
	ZipEntryListPanel(wxWindow *parent, int id, Archive* archive);
	~ZipEntryListPanel();

	void	populateEntryList();
	bool	swapItems(int item1, int item2);
	bool	addEntry(DWORD archive_index);
	bool	updateEntry(DWORD archive_index);
	bool	removeEntry(DWORD archive_index);

	ArchiveEntry*	newEntry(string name);
	ArchiveEntry*	newEntryFromFile(string name, string filename);

	// Events
	void	onEntryListActivated(wxListEvent &event);
	void	onEntryListEditLabel(wxListEvent &event);
};

#endif //__ENTRYLISTPANEL_H__