
#ifndef __WADPANEL_H__
#define __WADPANEL_H__

#include "LumpListPanel.h"

class WadPanel : public wxPanel
{
private:
	LumpListPanel	*lump_list;

public:
	WadPanel(wxWindow *parent);
	~WadPanel();
};

#endif //__WADPANEL_H__
