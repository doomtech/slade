
#ifndef __PALETTEENTRYPANEL_H__
#define	__PALETTEENTRYPANEL_H__

#include "EntryPanel.h"
#include "PaletteCanvas.h"

class PaletteEntryPanel : public EntryPanel {
private:
	PaletteCanvas*	pal_canvas;

public:
	PaletteEntryPanel(wxWindow* parent);
	~PaletteEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
};

#endif //__PALETTEENTRYPANEL_H__
