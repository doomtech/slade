
#ifndef __ARCHIVE_ENTRY_LIST_H__
#define __ARCHIVE_ENTRY_LIST_H__

#include "VirtualListView.h"
#include "ListenerAnnouncer.h"
#include "Archive.h"
#include <wx/listctrl.h>
#include <wx/textctrl.h>

wxDECLARE_EVENT(EVT_AEL_DIR_CHANGED, wxCommandEvent);

class ArchiveEntryList : public VirtualListView, public Listener {
private:
	Archive*			archive;
	vector<unsigned>	filter;
	bool				filter_active;
	string				filter_name;
	string				filter_category;
	ArchiveTreeNode*	current_dir;
	ArchiveEntry*		entry_dir_back;
	bool				show_dir_back;

	enum {
		AEL_COLUMN_NAME,
		AEL_COLUMN_SIZE,
		AEL_COLUMN_TYPE,
		AEL_HRULES,
		AEL_VRULES,
	};

protected:
	// Virtual wxListCtrl overrides
	string	getItemText(long item, long column) const;
	int		getItemIcon(long item) const;
	void	updateItemAttr(long item) const;

public:
	ArchiveEntryList(wxWindow* parent);
	~ArchiveEntryList();

	ArchiveTreeNode*	getCurrentDir() { return current_dir; }

	bool	showDirBack() { return show_dir_back; }
	void	showDirBack(bool db) { show_dir_back = db; }

	void	setArchive(Archive* archive);

	void	setupColumns();
	int		columnType(int column) const;
	void	updateList();
	int		entriesBegin();

	void	filterList(string filter = "", string category = "");
	void	applyFilter();
	bool	goUpDir();
	bool	setDir(ArchiveTreeNode* dir);

	ArchiveEntry*				getEntry(int index) const;
	int							getEntryIndex(int index);
	ArchiveEntry*				getFocusedEntry();
	vector<ArchiveEntry*>		getSelectedEntries();
	ArchiveEntry*				getLastSelectedEntry();
	vector<ArchiveTreeNode*>	getSelectedDirectories();

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void	onColumnHeaderRightClick(wxListEvent& e);
	void	onColumnResize(wxListEvent& e);
	void	onMenu(wxCommandEvent& e);
	void	onListItemActivated(wxListEvent& e);
};

#endif//__ARCHIVE_ENTRY_LIST_H__
