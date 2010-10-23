
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ArchiveManagerPanel.h"
#include <wx/aui/auibook.h>
#include <wx/html/htmlwin.h>
#include <wx/aui/aui.h>
#include "PaletteChooser.h"

// The below is needed because, if I add a toolbar or dockable window to the main window,
// then a previously saved perspective is loaded from slade3.cfg, the new item won't show
// up at all. So when saving the perspective, add this number to the start of the string,
// and check against it when loading. In other words, wxAUI is silly.
#define MW_LAYOUT_VERS 005

class MainWindow : public wxFrame {
private:
	ArchiveManagerPanel*	panel_archivemanager;
	wxAuiNotebook*			notebook_tabs;
	wxHtmlWindow*			html_startpage;
	wxAuiManager*			m_mgr;
	int						lasttipindex;
	PaletteChooser*			palette_chooser;

	// Singleton instance
	static MainWindow*		instance;

public:
	enum {
		MENU_START,

		MENU_FILE_NEWWAD,
		MENU_FILE_NEWZIP,
		MENU_FILE_OPEN,
		MENU_FILE_SAVE,
		MENU_FILE_SAVEAS,
		MENU_FILE_SAVEALL,
		MENU_FILE_CLOSE,
		MENU_FILE_CLOSEALL,
		MENU_FILE_EXIT,

		MENU_RECENT_1,
		MENU_RECENT_2,
		MENU_RECENT_3,
		MENU_RECENT_4,
		MENU_RECENT_5,
		MENU_RECENT_6,
		MENU_RECENT_7,
		MENU_RECENT_8,

		MENU_EDITOR_SETBASERESOURCE,
		MENU_EDITOR_PREFERENCES,

		MENU_ARCHIVE_NEWENTRY,
		MENU_ARCHIVE_NEWDIRECTORY,
		MENU_ARCHIVE_IMPORTFILES,
		MENU_ARCHIVE_CONVERTTO,
		MENU_ARCHIVE_CLEAN_PATCHES,
		MENU_ARCHIVE_TEXEDITOR,

		MENU_ENTRY_RENAME,
		MENU_ENTRY_DELETE,
		MENU_ENTRY_REVERT,
		MENU_ENTRY_CUT,
		MENU_ENTRY_COPY,
		MENU_ENTRY_PASTE,
		MENU_ENTRY_MOVEUP,
		MENU_ENTRY_MOVEDOWN,
		MENU_ENTRY_BOOKMARK,
		MENU_ENTRY_CONVERTTO,
		MENU_ENTRY_IMPORT,
		MENU_ENTRY_EXPORT,
		MENU_ENTRY_EXPORTAS,

		MENU_VIEW_ARCHIVEMANAGER,
		MENU_VIEW_CONSOLE,

		MENU_HELP_ONLINEDOCUMENTATION,
		MENU_HELP_ABOUT,

		MENU_END,
	};

	MainWindow();
	~MainWindow();

	// Singleton implementation
	static MainWindow* getInstance() {
		if (!instance)
			instance = new MainWindow();

		return instance;
	}

	void	setupLayout();
	void	createStartPage();
	bool	exitProgram();

	ArchiveManagerPanel*	getArchiveManagerPanel() { return panel_archivemanager; }
	PaletteChooser*			getPaletteChooser() { return palette_chooser; }

	Archive*				getCurrentArchive();
	ArchiveEntry*			getCurrentEntry();
	vector<ArchiveEntry*>	getCurrentEntrySelection();

	void	openTextureEditor(Archive* archive);
	void	openEntry(ArchiveEntry* entry);

	// Events
	void	onMenuItemClicked(wxCommandEvent &e);
	void	onHTMLLinkClicked(wxHtmlLinkEvent &e);
	void	onClose(wxCloseEvent &e);
	void	onTabChanged(wxAuiNotebookEvent& e);
};

// Define for less cumbersome MainWindow::getInstance()
#define theMainWindow MainWindow::getInstance()

#endif //__MAINWINDOW_H__
