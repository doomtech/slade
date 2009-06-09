
#ifndef __ZIPENTRYLISTPANEL_H__
#define	__ZIPENTRYLISTPANEL_H__

#include "EntryListPanel.h"
#include "ZipArchive.h"

class ZipEntryListPanel : public EntryListPanel {
private:
	zipdir_t*		cur_directory;
	ArchiveEntry*	dummy_folder_entry;
	int				entries_begin;

public:
	ZipEntryListPanel(wxWindow *parent, int id, Archive* archive);
	~ZipEntryListPanel();

	zipdir_t*			getCurrentDir() { return cur_directory; }
	vector<zipdir_t*>	getSelectedDirectories();
	void				updateDirectoryEntry(int index);
	int					entriesBegin();

	void	populateEntryList();
	bool	swapItems(int item1, int item2, ArchiveEntry* e1 = NULL, ArchiveEntry* e2 = NULL);
	bool	addEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	bool	updateEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	bool	removeEntry(DWORD archive_index, ArchiveEntry* e = NULL);
	bool	addDirectory(wxUIntPtr zipdir_ptr);
	bool	removeDirectory(wxUIntPtr zipdir_ptr);
	bool	updateDirectory(wxUIntPtr zipdir_ptr);

	bool	moveUp();
	bool	moveDown();

	// Events
	void	onEntryListActivated(wxListEvent &event);
	void	onEntryListEditLabel(wxListEvent &event);
};

#endif //__ZIPENTRYLISTPANEL_H__
