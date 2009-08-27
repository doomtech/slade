
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"
#include "GfxCanvas.h"
#include <wx/aui/auibar.h>

class GfxEntryPanel : public EntryPanel {
private:
	GfxCanvas*		gfx_canvas;
	wxAuiToolBar*	toolbar;

public:
	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
};

#endif //__GFXENTRYPANEL_H__
