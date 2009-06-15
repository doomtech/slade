
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ArchiveManagerPanel.h"
#include <wx/aui/auibook.h>
#include <wx/html/htmlwin.h>

class MainWindow : public wxFrame {
private:
	ArchiveManagerPanel*	panel_archivemanager;
	wxAuiNotebook*			notebook_tabs;
	wxHtmlWindow*			html_startpage;

public:
	// wxWidgets ID table
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

		MENU_UNIMPLEMENTED,

		MENU_END,

		// Widget IDs
		HTML_WINDOW,
	};

	MainWindow();
	~MainWindow();

	void	setupLayout();
	void	setMockLayout();

	// Event handlers
	void	onMenuItemClicked(wxCommandEvent &e);
	void	onHTMLLinkClicked(wxHtmlLinkEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__MAINWINDOW_H__
