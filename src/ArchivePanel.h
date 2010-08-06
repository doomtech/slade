
#ifndef __ARCHIVEPANEL_H__
#define __ARCHIVEPANEL_H__

#include "Archive.h"
#include "EntryPanel.h"
#include "ListenerAnnouncer.h"
#include "ArchiveEntryList.h"

class ArchivePanel : public wxPanel, public Listener {
protected:
	Archive*			archive;
	ArchiveEntryList*	entry_list;

	// Entry panels
	EntryPanel*	cur_area;
	EntryPanel*	entry_area;
	EntryPanel*	default_area;
	EntryPanel*	text_area;
	EntryPanel*	gfx_area;
	EntryPanel*	pal_area;
	EntryPanel*	multi_area;
	EntryPanel* texturex_area;
	EntryPanel* animated_area;
	EntryPanel* switches_area;
	EntryPanel* pnames_area;

public:
	ArchivePanel(wxWindow *parent, Archive* archive);
	~ArchivePanel();

	Archive*	getArchive() { return archive; }
	bool		saveEntryChanges();

	// Editing actions - return success

	// Archive manipulation actions
	bool	save();
	bool	saveAs();
	bool	newEntry();
	bool	newDirectory();
	bool	importFiles();
	bool	convertArchiveTo();
	bool	cleanupArchive();

	// Entry manipulation actions
	bool	renameEntry();
	bool	deleteEntry();
	bool	moveUp();
	bool	moveDown();
	bool	convertEntryTo();
	bool	importEntry();
	bool	exportEntry();
	bool	exportEntryAs();
	bool	copyEntry();
	bool	cutEntry();
	bool	pasteEntry();

	// Other entry actions
	bool	gfxConvert();
	bool	gfxModifyOffsets();
	bool	basConvert();
	bool	palConvert();
	bool	reloadCurrentPanel();

	// Needed for some console commands
	EntryPanel *	currentArea() { return cur_area;}
	ArchiveEntry*	currentEntry();

	// UI related
	bool	openEntry(ArchiveEntry* entry, bool force = false);
	bool	showEntryPanel(EntryPanel* new_area, bool ask_save = true);
	void	focusEntryList() { entry_list->SetFocus(); }

	// Function to handle menu actions from MainWindow
	void	handleAction(int menu_id);

	virtual void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void			onEntryListSelectionChange(wxCommandEvent& e);
	void			onEntryListFocusChange(wxListEvent& e);
	void			onEntryListRightClick(wxListEvent& e);
	void			onEntryMenuClick(wxCommandEvent& e);
	void			onEntryListKeyDown(wxKeyEvent& e);
	virtual void	onEntryListActivated(wxListEvent& e);
};

#endif //__ARCHIVEPANEL_H__
