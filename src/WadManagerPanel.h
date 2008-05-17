
#ifndef __WADMANAGERPANEL_H__
#define __WADMANAGERPANEL_H__

#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/dirctrl.h>

class WadManagerPanel : public wxPanel
{
private:
	wxNotebook*			notebook_tabs;
	wxNotebook*			notebook_wads;
	wxTreeCtrl*			tree_wadlist;
	wxGenericDirCtrl*	file_browser;

public:
	// wxWidgets ID table
	enum
	{
		TREE_WADLIST,
		TREE_BROWSER,
	};

	WadManagerPanel(wxWindow *parent, wxNotebook* nb_wads);
	~WadManagerPanel();

	void	openFile(string filename);
	void	openFiles(wxArrayString& files);

	// Event handlers
	void	onBrowserItemActivated(wxTreeEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__WADMANAGERPANEL_H__
