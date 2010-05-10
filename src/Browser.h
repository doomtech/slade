
#ifndef __BROWSER_H__
#define __BROWSER_H__

#include "BrowserCanvas.h"
#include <wx/treectrl.h>

class Browser : public wxPanel {
private:
	BrowserCanvas*	browser_canvas;
	wxTreeCtrl*		tree_items;

public:
	Browser(wxWindow* parent);
	~Browser();
};

#endif//__BROWSER_H__
