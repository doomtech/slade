
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"

class GfxEntryPanel : public EntryPanel
{
private:

public:
	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
};

#endif //__GFXENTRYPANEL_H__
