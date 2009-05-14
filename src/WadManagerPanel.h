
#ifndef __WADMANAGERPANEL_H__
#define __WADMANAGERPANEL_H__

#include <wx/listctrl.h>
#include <wx/aui/auibook.h>
#include <wx/dirctrl.h>
#include <wx/listbox.h>

class WadManagerPanel;

class WMFileBrowser : public wxGenericDirCtrl {
private:

public:
	WadManagerPanel*	parent;

	WMFileBrowser(wxWindow* parent, WadManagerPanel* wm, int id);
	~WMFileBrowser();

	void onItemActivated(wxTreeEvent &e);
};

class WadManagerPanel : public wxPanel {
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
	};

	WadManagerPanel(wxWindow *parent, wxAuiNotebook* nb_wads);
	~WadManagerPanel();

	void	openFile(string filename);
	void	openFiles(wxArrayString& files);

	void	saveSelection();
	void	saveSelectionAs();
	void	saveCurrent();
	void	saveCurrentAs();

	vector<int>	getSelectedWads();

	// Event handlers
	void	onListWadsChanged(wxListEvent &e);
	void	onListWadsActivated(wxListEvent &e);
	void	onListMapsChanged(wxCommandEvent &e);
	void	onListMapsActivated(wxListEvent &e);
	void	onBrowserItemActivated(wxTreeEvent &e);
	void	onListWadsRightClick(wxListEvent &e);
	void	onMenuSave(wxCommandEvent &e);
	void	onMenuSaveAs(wxCommandEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__WADMANAGERPANEL_H__
