
#ifndef __WADPANEL_H__
#define __WADPANEL_H__

#include "LumpListPanel.h"
#include "Wad.h"
#include "LumpArea.h"

class WadPanel : public wxPanel
{
private:
	Wad*			wad;
	LumpListPanel*	lump_list;
	LumpArea*		lump_area;

public:
	WadPanel(wxWindow *parent, Wad* wad);
	~WadPanel();
};

#endif //__WADPANEL_H__
