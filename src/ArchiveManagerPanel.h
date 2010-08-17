
#ifndef __ARCHIVEMANAGERPANEL_H__
#define __ARCHIVEMANAGERPANEL_H__

#include <wx/listctrl.h>
#include <wx/aui/auibook.h>
#include <wx/dirctrl.h>
#include <wx/listbox.h>
#include "ListenerAnnouncer.h"
#include "Archive.h"
#include "ListView.h"
#include "EntryPanel.h"

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
	ListView*			list_archives;
	ListView*			list_recent;
	ListView*			list_bookmarks;
	wxListCtrl*			list_maps;
	WMFileBrowser*		file_browser;
	wxButton*			btn_browser_open;
	wxMenu*				menu_context;
	wxMenu*				menu_recent;

public:
	// wxWidgets ID table
	enum {
		MENU_SAVE,
		MENU_SAVEAS,
		MENU_CLOSE,

		MENU_END,
	};

	ArchiveManagerPanel(wxWindow *parent, wxAuiNotebook* nb_archives);
	~ArchiveManagerPanel();

	wxMenu*			recentFilesMenu() { return menu_recent; }

	void			refreshArchiveList();
	void			refreshRecentFileList();
	void			populateMapList(Archive* archive);
	void			updateOpenListItem(int index);
	void			updateRecentListItem(int index);
	bool			isArchivePanel(int tab_index);
	int				currentTabIndex();
	Archive*		currentArchive();
	wxWindow*		currentPanel();
	EntryPanel*		currentArea();

	void	openTab(int archive_index);
	void	closeTab(int archive_index);
	void	openTextureTab(int archive_index);
	void	closeTextureTab(int archive_index);
	void	openFile(string filename);
	void	openFiles(wxArrayString& files);

	void	createNewArchive(uint8_t type);
	void	closeAll();
	void	saveAll();

	// Selected archives in the list
	void	saveSelection();
	void	saveSelectionAs();
	void	closeSelection();

	// Handler for MainWindow menu actions
	void	handleAction(int menu_id);

	vector<int>	getSelectedArchives();

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Event handlers
	void	onListArchivesChanged(wxListEvent& e);
	void	onListArchivesActivated(wxListEvent& e);
	void	onListArchivesRightClick(wxListEvent& e);
	void	onListRecentChanged(wxListEvent& e);
	void	onListRecentActivated(wxListEvent& e);
	void	onListRecentRightClick(wxListEvent& e);
	void	onListMapsChanged(wxCommandEvent& e);
	void	onListMapsActivated(wxListEvent& e);
	void	onMenu(wxCommandEvent& e);
	void	onTabChanged(wxAuiNotebookEvent& e);
	void	onTabClose(wxAuiNotebookEvent& e);
};

#endif //__ARCHIVEMANAGERPANEL_H__
