
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "WadManagerPanel.h"
#include <wx/aui/auibook.h>
#include <wx/html/htmlwin.h>

class MainWindow : public wxFrame
{
private:
	WadManagerPanel*		panel_wadmanager;
	wxAuiNotebook*			notebook_tabs;
	wxHtmlWindow*			html_startpage;

public:
	// wxWidgets ID table
	enum
	{
		MENU_FILE_NEW,
		MENU_FILE_NEWZIP,
		MENU_FILE_OPEN,
		MENU_FILE_SAVEALL,
		MENU_FILE_CLOSEALL,
		MENU_FILE_OPTIONS,
		MENU_FILE_QUIT,

		MENU_VIEW_WADMANAGER,
		MENU_VIEW_CONSOLE,

		MENU_WAD_SAVE,
		MENU_WAD_SAVEAS,
		MENU_WAD_CLOSE,

		HTML_WINDOW,
	};

	MainWindow();
	~MainWindow();

	void	setupLayout();
	void	setMockLayout();

	// Event handlers
	void	onFileOpen(wxCommandEvent &e);
	void	onFileQuit(wxCommandEvent &e);

	void	onViewWadManager(wxCommandEvent &e);
	void	onViewConsole(wxCommandEvent &e);

	void	onHTMLLinkClicked(wxHtmlLinkEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__MAINWINDOW_H__
