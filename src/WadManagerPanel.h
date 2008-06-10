
#ifndef __WADMANAGERPANEL_H__
#define __WADMANAGERPANEL_H__

#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/dirctrl.h>
#include <wx/listbox.h>

class WadManagerPanel : public wxPanel
{
private:
	wxNotebook*			notebook_tabs;
	wxNotebook*			notebook_wads;
	wxListBox*			list_openwads;
	wxListCtrl*			list_maps;
	wxGenericDirCtrl*	file_browser;
	wxButton*			btn_browser_open;

public:
	// wxWidgets ID table
	enum
	{
		LIST_OPENWADS,
		LIST_MAPS,
		TREE_BROWSER,
		BTN_BROWSER_OPEN,
	};

	WadManagerPanel(wxWindow *parent, wxNotebook* nb_wads);
	~WadManagerPanel();

	void	openFile(string filename);
	void	openFiles(wxArrayString& files);

	// Event handlers
	void	onListWadsChanged(wxCommandEvent &e);
	void	onListWadsActivated(wxCommandEvent &e);
	void	onListMapsChanged(wxListEvent &e);
	void	onListMapsActivated(wxListEvent &e);
	void	onBrowserItemActivated(wxTreeEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__WADMANAGERPANEL_H__
