
#ifndef __WADMANAGERPANEL_H__
#define __WADMANAGERPANEL_H__

#include <wx/listctrl.h>
#include <wx/aui/auibook.h>
#include <wx/dirctrl.h>
#include <wx/listbox.h>
#include "ListenerAnnouncer.h"

class WadManagerPanel;

class WMFileBrowser : public wxGenericDirCtrl {
private:

public:
	WadManagerPanel*	parent;

	WMFileBrowser(wxWindow* parent, WadManagerPanel* wm, int id);
	~WMFileBrowser();

	void onItemActivated(wxTreeEvent &e);
};

class WadManagerPanel : public wxPanel, Listener {
private:
	wxAuiNotebook*		notebook_tabs;
	wxAuiNotebook*		notebook_wads;
	wxListCtrl*			list_openwads;
	wxListCtrl*			list_maps;
	WMFileBrowser*		file_browser;
	wxButton*			btn_browser_open;
	wxMenu*				menu_wadcontext;

public:
	// wxWidgets ID table
	enum {
		LIST_OPENWADS,
		LIST_MAPS,
		TREE_BROWSER,
		BTN_BROWSER_OPEN,

		MENU_SAVE,
		MENU_SAVEAS,
		MENU_CLOSE,
	};

	WadManagerPanel(wxWindow *parent, wxAuiNotebook* nb_wads);
	~WadManagerPanel();

	void	refreshWadList();

	void	openFile(string filename);
	void	openFiles(wxArrayString& files);

	// Selected archives in the list
	void	saveSelection();
	void	saveSelectionAs();
	void	closeSelection();

	// Currently opened tab
	void	saveCurrent();
	void	saveCurrentAs();
	void	closeCurrent();
	void	moveUp();
	void	moveDown();

	vector<int>	getSelectedWads();

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Event handlers
	void	onListWadsChanged(wxListEvent &e);
	void	onListWadsActivated(wxListEvent &e);
	void	onListMapsChanged(wxCommandEvent &e);
	void	onListMapsActivated(wxListEvent &e);
	void	onBrowserItemActivated(wxTreeEvent &e);
	void	onListWadsRightClick(wxListEvent &e);
	void	onMenuSave(wxCommandEvent &e);
	void	onMenuSaveAs(wxCommandEvent &e);
	void	onMenuClose(wxCommandEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__WADMANAGERPANEL_H__
