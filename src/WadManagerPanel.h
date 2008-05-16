
#ifndef __WADMANAGERPANEL_H__
#define __WADMANAGERPANEL_H__

#include <wx/treectrl.h>

class WadManagerPanel : public wxPanel
{
private:
	wxTreeCtrl*		tree_wadlist;

public:
	WadManagerPanel(wxWindow *parent);
	~WadManagerPanel();
};

#endif //__WADMANAGERPANEL_H__
