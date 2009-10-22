
#ifndef __ARCHIVEPANEL_H__
#define __ARCHIVEPANEL_H__

#include "Archive.h"
#include "EntryListPanel.h"
#include "EntryPanel.h"
#include "ListenerAnnouncer.h"

class ArchivePanel : public wxPanel, Listener {
protected:
	Archive*		archive;
	EntryListPanel*	entry_list;
	wxMenu*			menu_entry;

	// Entry panels
	EntryPanel*		cur_area;
	EntryPanel*		entry_area;
	EntryPanel*		default_area;
	EntryPanel*		text_area;
	EntryPanel*		gfx_area;
	EntryPanel*		pal_area;

public:
	enum {
		ENTRY_LIST_PANEL,

		MENU_ENTRY_RENAME,
		MENU_ENTRY_DELETE,
		MENU_ENTRY_IMPORT,
		MENU_ENTRY_EXPORT,
		MENU_ENTRY_EXPORTWAD,
		MENU_ENTRY_MOVEUP,
		MENU_ENTRY_MOVEDOWN,

		MENU_ENTRY_END,
	};

	ArchivePanel(wxWindow *parent, Archive* archive);
	virtual ~ArchivePanel();

	virtual void	init();
	Archive*		getArchive() { return archive; }

	// Archive manipulation functions
	virtual void	save();
	virtual void	saveAs();

	// Entry manipulation functions
	virtual bool	newEntry();
	virtual bool	newEntryFromFile();
	virtual bool	renameEntry();
	virtual bool	deleteEntry();
	bool	importEntry();
	bool	exportEntry();
	bool	exportEntryWad();
	bool	moveUp();
	bool	moveDown();

	virtual void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void	onEntryListChange(wxListEvent &event);
	void	onEntryListRightClick(wxListEvent &event);
	void	onEntryMenuClick(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__ARCHIVEPANEL_H__
