
#ifndef __ZIPENTRYLISTPANEL_H__
#define	__ZIPENTRYLISTPANEL_H__

#include "EntryListPanel.h"

class ZipEntryListPanel : public EntryListPanel {
private:
	void*			cur_directory;
	ArchiveEntry*	dummy_folder_entry;
	int				entries_begin;

public:
	ZipEntryListPanel(wxWindow *parent, int id, Archive* archive);
	~ZipEntryListPanel();

	void	populateEntryList();
	bool	swapItems(int item1, int item2, ArchiveEntry* e1 = NULL, ArchiveEntry* e2 = NULL);
	bool	addEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	bool	updateEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	bool	removeEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	bool	addDirectory(wxUIntPtr zipdir_ptr);
	bool	removeDirectory(wxUIntPtr zipdir_ptr);

	ArchiveEntry*	newEntry();
	ArchiveEntry*	newEntryFromFile();
	bool			moveUp();
	bool			moveDown();

	void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void	onEntryListActivated(wxListEvent &event);
	void	onEntryListEditLabel(wxListEvent &event);
};

#endif //__ZIPENTRYLISTPANEL_H__
