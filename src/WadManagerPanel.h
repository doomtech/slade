
#ifndef __WADMANAGERPANEL_H__
#define __WADMANAGERPANEL_H__

#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/dirctrl.h>

class WadManagerPanel : public wxPanel
{
private:
	wxNotebook*			notebook_tabs;
	wxTreeCtrl*			tree_wadlist;
	wxGenericDirCtrl*	file_browser;

public:
	WadManagerPanel(wxWindow *parent);
	~WadManagerPanel();

	void	openFiles(wxArrayString& files);
};

#endif //__WADMANAGERPANEL_H__
