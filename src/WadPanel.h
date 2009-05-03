
#ifndef __WADPANEL_H__
#define __WADPANEL_H__

#include "EntryListPanel.h"
#include "Archive.h"
#include "LumpArea.h"

class WadPanel : public wxPanel
{
private:
	Archive*		wad;
	EntryListPanel*	lump_list;
	LumpArea*		lump_area;

public:
	WadPanel(wxWindow *parent, Archive* wad);
	~WadPanel();

	Archive*	getWad() { return wad; }
};

#endif //__WADPANEL_H__
