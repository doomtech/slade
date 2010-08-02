
#ifndef __ARCHIVE_ENTRY_LIST_H__
#define __ARCHIVE_ENTRY_LIST_H__

#include "VirtualListView.h"
#include "ListenerAnnouncer.h"
#include "Archive.h"
#include <wx/listctrl.h>
#include <wx/textctrl.h>

class ArchiveEntryList : public VirtualListView, public Listener {
private:
	Archive*			archive;
	wxTextCtrl*			text_filter;
	vector<unsigned>	filter;
	bool				filter_active;
	ArchiveTreeNode*	current_dir;
	ArchiveEntry*		entry_dir_back;

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

	void	setArchive(Archive* archive);
	void	setFilterCtrl(wxTextCtrl* text_filter);

	void	setupColumns();
	int		columnType(int column) const;
	void	updateList();
	void	filterList(string filter);
	int		entriesBegin();

	ArchiveEntry*				getEntry(int index) const;
	ArchiveEntry*				getFocusedEntry();
	vector<ArchiveEntry*>		getSelectedEntries();
	ArchiveEntry*				getLastSelectedEntry();
	vector<ArchiveTreeNode*>	getSelectedDirectories();

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void	onColumnHeaderRightClick(wxListEvent& e);
	void	onColumnResize(wxListEvent& e);
	void	onMenu(wxCommandEvent& e);
	void	onFilterChanged(wxCommandEvent& e);
	void	onListItemActivated(wxListEvent& e);
};

#endif//__ARCHIVE_ENTRY_LIST_H__
