
#ifndef __ZIP_ARCHIVE_ENTRY_LIST_H__
#define __ZIP_ARCHIVE_ENTRY_LIST_H__

#include "ArchiveEntryList.h"
#include "ZipArchive.h"

class ZipArchiveEntryList : public ArchiveEntryList {
protected:
	// Virtual wxListCtrl overrides
	string			OnGetItemText(long item, long column) const;
	//int				OnGetItemImage(long item) const;
	//wxListItemAttr*	OnGetItemAttr(long item) const;

private:
	zipdir_t*		current_dir;
	ArchiveEntry*	entry_folder_back;

public:
	ZipArchiveEntryList(wxWindow* parent, Archive* archive);
	~ZipArchiveEntryList();

	zipdir_t*	getCurrentDir() { return current_dir; }

	void	setArchive(Archive* archive);
	void	updateList();

	bool				isFolder(int index);
	bool				isEntry(int index);
	ArchiveEntry*		getEntry(int index) const;
	vector<zipdir_t*>	getSelectedDirectories();

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void	onListItemActivated(wxListEvent& e);
};

#endif//__ZIP_ARCHIVE_ENTRY_LIST_H__
