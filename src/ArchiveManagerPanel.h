
#ifndef __ARCHIVEMANAGERPANEL_H__
#define __ARCHIVEMANAGERPANEL_H__

#include <wx/listctrl.h>
#include <wx/aui/auibook.h>
#include <wx/dirctrl.h>
#include <wx/listbox.h>
#include "ListenerAnnouncer.h"

class ArchiveManagerPanel;

class WMFileBrowser : public wxGenericDirCtrl {
private:

public:
	ArchiveManagerPanel*	parent;

	WMFileBrowser(wxWindow* parent, ArchiveManagerPanel* wm, int id);
	~WMFileBrowser();

	void onItemActivated(wxTreeEvent &e);
};

class ArchiveManagerPanel : public wxPanel, Listener {
private:
	wxAuiNotebook*		notebook_tabs;
	wxAuiNotebook*		notebook_archives;
	wxListCtrl*			list_archives;
	wxListCtrl*			list_maps;
	WMFileBrowser*		file_browser;
	wxButton*			btn_browser_open;
	wxMenu*				menu_context;

public:
	// wxWidgets ID table
	enum {
		LIST_OPENARCHIVES,
		LIST_MAPS,
		TREE_BROWSER,
		BTN_BROWSER_OPEN,

		MENU_SAVE,
		MENU_SAVEAS,
		MENU_CLOSE,
	};

	ArchiveManagerPanel(wxWindow *parent, wxAuiNotebook* nb_archives);
	~ArchiveManagerPanel();

	void	refreshArchiveList();
	bool	isArchivePanel(int tab_index);

	void	openFile(string filename);
	void	openFiles(wxArrayString& files);

	void	createNewArchive(BYTE type);

	// Selected archives in the list
	void	saveSelection();
	void	saveSelectionAs();
	void	closeSelection();

	// Currently opened tab
	void	saveCurrent();
	void	saveCurrentAs();
	void	closeCurrent();
	void	newEntry();
	void	newEntryFromFile();
	void	renameEntry();
	void	deleteEntry();
	void	importEntry();
	void	exportEntry();
	void	exportEntryWad();
	void	moveUp();
	void	moveDown();

	vector<int>	getSelectedArchives();

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Event handlers
	void	onListArchivesChanged(wxListEvent &e);
	void	onListArchivesActivated(wxListEvent &e);
	void	onListMapsChanged(wxCommandEvent &e);
	void	onListMapsActivated(wxListEvent &e);
	void	onBrowserItemActivated(wxTreeEvent &e);
	void	onListArchivesRightClick(wxListEvent &e);
	void	onMenuSave(wxCommandEvent &e);
	void	onMenuSaveAs(wxCommandEvent &e);
	void	onMenuClose(wxCommandEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__ARCHIVEMANAGERPANEL_H__
