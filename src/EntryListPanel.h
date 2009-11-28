
#ifndef __ENTRYLISTPANEL_H__
#define __ENTRYLISTPANEL_H__

#include <wx/listctrl.h>

class Archive;
class EntryListPanel;

/* TODO:
 * - Implement my own automatic resize (current way is too slow)
 */
class EntryList : public wxListCtrl {
private:
	EntryListPanel*		parent;
	wxImageList*		image_list;

public:
	EntryList(EntryListPanel *parent, int id);
	~EntryList();

	bool	updateEntry(int index, bool update_colsize = true);
	int		getWidth();
};

class EntryListPanel : public wxPanel {
protected:
	Archive*	archive;
	EntryList*	entry_list;
	bool		col_update;

public:
	enum {
		ENTRY_LIST,
		MENU_COLUMNS_SIZE,
		MENU_COLUMNS_TYPE,
	};

	EntryListPanel(wxWindow *parent, int id, Archive* archive);
	virtual ~EntryListPanel();

	virtual void			populateEntryList();
	void					updateListWidth();
	bool					columnsUpdate() { return col_update; }
	void					columnsUpdate(bool update) { col_update = update; }

	ArchiveEntry*			getFocusedEntry();
	int						getFocus();
	vector<ArchiveEntry*>	getSelectedEntries();
	virtual vector<int>		getSelection();
	int						getLastSelected();
	ArchiveEntry*			getLastSelectedEntry();
	int						getListSize();
	virtual bool			swapItems(int item1, int item2, ArchiveEntry* e1 = NULL, ArchiveEntry* e2 = NULL);
	virtual bool			addEntry(uint32_t archive_index, ArchiveEntry* e = NULL);
	virtual bool			updateEntry(uint32_t archive_index, ArchiveEntry* e = NULL);
	virtual bool			removeEntry(uint32_t archive_index, ArchiveEntry* e = NULL);
	int						getEntryListItem(ArchiveEntry* entry);

	void					selectAll();
	virtual bool			moveUp();
	virtual bool			moveDown();

	// Events
	virtual void	onEntryListActivated(wxListEvent& e);
	virtual void	onEntryListColResize(wxListEvent& e);
	virtual void	onEntryListColRightClick(wxListEvent& e);
	virtual void	onMenu(wxCommandEvent& e);
};

#endif //__ENTRYLISTPANEL_H__
