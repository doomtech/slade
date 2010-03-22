
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ArchiveManagerPanel.h"
#include <wx/aui/auibook.h>
#include <wx/html/htmlwin.h>
#include <wx/aui/aui.h>

// The below is needed because, if I add a toolbar or dockable window to the main window,
// then a previously saved perspective is loaded from slade3.cfg, the new item won't show
// up at all. So when saving the perspective, add this number to the start of the string,
// and check against it when loading. In other words, wxAUI is silly.
#define MW_LAYOUT_VERS 001

class MainWindow : public wxFrame {
private:
	ArchiveManagerPanel*	panel_archivemanager;
	wxAuiNotebook*			notebook_tabs;
	wxHtmlWindow*			html_startpage;
	wxAuiManager*			m_mgr;

public:
	enum {
		// Menu item IDs
		MENU_START,

		MENU_FILE_NEW,
		MENU_FILE_NEWZIP,
		MENU_FILE_OPEN,
		MENU_FILE_SAVE,
		MENU_FILE_SAVEAS,
		MENU_FILE_SAVEALL,
		MENU_FILE_CLOSE,
		MENU_FILE_CLOSEALL,
		MENU_FILE_OPTIONS,
		MENU_FILE_QUIT,

		MENU_EDITOR_SET_BASE_RESOURCE,
		MENU_EDITOR_PREFERENCES,

		MENU_VIEW_MANAGER,
		MENU_VIEW_CONSOLE,

		MENU_ENTRY_NEW,
		MENU_ENTRY_NEWFROMFILE,
		MENU_ENTRY_NEWFOLDER,
		MENU_ENTRY_RENAME,
		MENU_ENTRY_DELETE,
		MENU_ENTRY_IMPORT,
		MENU_ENTRY_EXPORT,
		MENU_ENTRY_EXPORTWAD,
		MENU_ENTRY_MOVEUP,
		MENU_ENTRY_MOVEDOWN,

		MENU_HELP_ABOUT,

		MENU_UNIMPLEMENTED,

		MENU_END,
	};

	MainWindow();
	~MainWindow();

	void	setupLayout();

	// Events
	void	onMenuItemClicked(wxCommandEvent &e);
	void	onHTMLLinkClicked(wxHtmlLinkEvent &e);
	void	onClose(wxCloseEvent &e);
};

#endif //__MAINWINDOW_H__
