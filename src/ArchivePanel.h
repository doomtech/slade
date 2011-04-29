
#ifndef __ARCHIVEPANEL_H__
#define __ARCHIVEPANEL_H__

#include "Archive.h"
#include "EntryPanel.h"
#include "ListenerAnnouncer.h"
#include "ArchiveEntryList.h"
#include "MainApp.h"
#include <wx/textctrl.h>
#include <wx/choice.h>

class ArchivePanel : public wxPanel, public Listener, SActionHandler {
protected:
	Archive*			archive;
	ArchiveEntryList*	entry_list;
	wxTextCtrl*			text_filter;
	wxChoice*			choice_category;
	wxStaticText*		label_path;
	wxBitmapButton*		btn_updir;
	wxSizer*			sizer_path_controls;

	// Entry panels
	EntryPanel*	cur_area;
	EntryPanel*	entry_area;
	EntryPanel*	default_area;
	EntryPanel*	text_area;
	EntryPanel*	ansi_area;
	EntryPanel*	gfx_area;
	EntryPanel*	pal_area;
	EntryPanel* texturex_area;
	EntryPanel* animated_area;
	EntryPanel* switches_area;
	EntryPanel* pnames_area;
	EntryPanel* hex_area;
	EntryPanel* map_area;
	EntryPanel* audio_area;

public:
	ArchivePanel(wxWindow *parent, Archive* archive);
	~ArchivePanel();

	Archive*	getArchive() { return archive; }
	bool		saveEntryChanges();
	void		addMenus();
	void		removeMenus();

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
	bool	renameEntry(bool each = false);
	bool	deleteEntry();
	bool	revertEntry();
	bool	moveUp();
	bool	moveDown();
	bool	bookmark();
	bool	convertEntryTo();
	bool	importEntry();
	bool	exportEntry();
	bool	exportEntryAs();
	bool	copyEntry();
	bool	cutEntry();
	bool	pasteEntry();

	// Other entry actions
	bool	gfxConvert();
	bool	gfxRemap();
	bool	gfxModifyOffsets();
	bool	gfxExportPNG();
	bool	basConvert();
	bool	palConvert();
	bool	reloadCurrentPanel();
	bool	wavDSndConvert();
	bool	dSndWavConvert();
	bool	musMidiConvert();
	bool	optimizePNG();
	bool	compileACS(bool hexen = false);
	bool	convertTextures();

	// Needed for some console commands
	EntryPanel *			currentArea() { return cur_area;}
	ArchiveEntry*			currentEntry();
	vector<ArchiveEntry*>	currentEntries();

	// UI related
	bool	openEntry(ArchiveEntry* entry, bool force = false);
	bool	openEntryAsText(ArchiveEntry* entry);
	bool	openEntryAsHex(ArchiveEntry* entry);
	bool	showEntryPanel(EntryPanel* new_area, bool ask_save = true);
	void	focusOnEntry(ArchiveEntry* entry);
	void	focusEntryList() { entry_list->SetFocus(); }
	void	refreshPanel();

	// SAction handler
	bool	handleAction(string id);

	virtual void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void			onEntryListSelectionChange(wxCommandEvent& e);
	void			onEntryListFocusChange(wxListEvent& e);
	void			onEntryListRightClick(wxListEvent& e);
	void			onEntryListKeyDown(wxKeyEvent& e);
	virtual void	onEntryListActivated(wxListEvent& e);
	void			onDEPEditAsText(wxCommandEvent& e);
	void			onDEPViewAsHex(wxCommandEvent& e);
	void			onTextFilterChanged(wxCommandEvent& e);
	void			onChoiceCategoryChanged(wxCommandEvent& e);
	void			onDirChanged(wxCommandEvent& e);
	void			onBtnUpDir(wxCommandEvent& e);
	void			onShow(wxShowEvent& e);
};

#endif //__ARCHIVEPANEL_H__
