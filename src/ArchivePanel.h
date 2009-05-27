
#ifndef __ARCHIVEPANEL_H__
#define __ARCHIVEPANEL_H__

#include "Archive.h"
#include "EntryListPanel.h"
#include "EntryPanel.h"
#include "ListenerAnnouncer.h"

class ArchivePanel : public wxPanel, Listener {
private:
	Archive*		archive;
	EntryListPanel*	entry_list;

	// Entry panels
	EntryPanel*		cur_area;
	EntryPanel*		entry_area;
	TextEntryPanel*	text_area;

public:
	enum {
		ENTRY_LIST_PANEL,
	};

	ArchivePanel(wxWindow *parent, Archive* archive);
	~ArchivePanel();

	Archive*	getArchive() { return archive; }

	// Archive manipulationfunctions
	void		save();
	void		saveAs();

	// Entry manipulation functions
	bool	newEntry();
	bool	newEntryFromFile();
	bool	renameEntry();
	bool	deleteEntry();
	bool	importEntry();
	bool	exportEntry();
	bool	exportEntryWad();
	bool	moveUp();
	bool	moveDown();

	void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void	onEntryListChange(wxListEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__ARCHIVEPANEL_H__
