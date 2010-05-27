
#ifndef __ARCHIVE_ENTRY_LIST_H__
#define __ARCHIVE_ENTRY_LIST_H__

#include "VirtualListView.h"
#include "ListenerAnnouncer.h"
#include "Archive.h"
#include <wx/listctrl.h>
#include <wx/textctrl.h>

class ArchiveEntryList : public VirtualListView, public Listener {
protected:
	Archive*			archive;
	//wxListItemAttr*		item_attr;
	wxTextCtrl*			text_filter;
	vector<unsigned>	filter;
	bool				filter_active;

	enum {
		AEL_COLUMN_NAME,
		AEL_COLUMN_SIZE,
		AEL_COLUMN_TYPE,
		AEL_HRULES,
		AEL_VRULES,
	};

	// Virtual wxListCtrl overrides
	string	getItemText(long item, long column) const;
	int		getItemIcon(long item) const;
	void	updateItemAttr(long item) const;
	//string			OnGetItemText(long item, long column) const;
	//int				OnGetItemImage(long item) const;
	//wxListItemAttr*	OnGetItemAttr(long item) const;

public:
	ArchiveEntryList(wxWindow* parent);
	~ArchiveEntryList();

	virtual void	setArchive(Archive* archive);
	void			setFilterCtrl(wxTextCtrl* text_filter);
	void			setupColumns();
	void			updateWidth();
	int				columnType(int column) const;
	virtual void	updateList(bool clear = false);
	virtual void	filterList(string filter);

	virtual ArchiveEntry*	getEntry(int index) const;
	ArchiveEntry*			getFocusedEntry();
	//int						getFocus();
	vector<ArchiveEntry*>	getSelectedEntries();
	//virtual vector<int>		getSelection();
	//int						getLastSelected();
	ArchiveEntry*			getLastSelectedEntry();

	//void	selectItem(int index, bool select = true);
	//void	clearSelection();
	//void	selectAll();

	virtual void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void			onColumnHeaderRightClick(wxListEvent& e);
	void			onColumnResize(wxListEvent& e);
	void			onMenu(wxCommandEvent& e);
	void			onFilterChanged(wxCommandEvent& e);
};

#endif//__ARCHIVE_ENTRY_LIST_H__
