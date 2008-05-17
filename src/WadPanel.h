
#ifndef __WADPANEL_H__
#define __WADPANEL_H__

#include "LumpListPanel.h"
#include "Wad.h"

class WadPanel : public wxPanel
{
private:
	Wad*			wad;
	LumpListPanel*	lump_list;

public:
	WadPanel(wxWindow *parent, Wad* wad);
	~WadPanel();
};

#endif //__WADPANEL_H__
