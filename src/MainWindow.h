
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ArchiveManagerPanel.h"
#include <wx/aui/auibook.h>
#include <wx/html/htmlwin.h>
#include <wx/aui/aui.h>
#include "PaletteChooser.h"
#include "MainApp.h"

// The below is needed because, if I add a toolbar or dockable window to the main window,
// then a previously saved perspective is loaded from slade3.cfg, the new item won't show
// up at all. So when saving the perspective, add this number to the start of the string,
// and check against it when loading. In other words, wxAUI is silly.
#define MW_LAYOUT_VERS 007

class MainWindow : public wxFrame, SActionHandler {
private:
	ArchiveManagerPanel*	panel_archivemanager;
	wxAuiNotebook*			notebook_tabs;
	wxHtmlWindow*			html_startpage;
	wxAuiManager*			m_mgr;
	int						lasttipindex;
	PaletteChooser*			palette_chooser;

	// Singleton instance
	static MainWindow*		instance;

	// Action handling
	bool	handleAction(string id);

public:
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

	// Custom menu
	void	addCustomMenu(wxMenu* menu, string title);
	void	removeCustomMenu(string title);

	// Events
	void	onMenuItemClicked(wxCommandEvent &e);
	void	onHTMLLinkClicked(wxHtmlLinkEvent &e);
	void	onClose(wxCloseEvent& e);
	void	onTabChanged(wxAuiNotebookEvent& e);
	void	onSize(wxSizeEvent& e);
	void	onMove(wxMoveEvent& e);
};

// Define for less cumbersome MainWindow::getInstance()
#define theMainWindow MainWindow::getInstance()

#endif //__MAINWINDOW_H__
