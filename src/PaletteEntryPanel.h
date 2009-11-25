
#ifndef __PALETTEENTRYPANEL_H__
#define	__PALETTEENTRYPANEL_H__

#include "EntryPanel.h"
#include "PaletteCanvas.h"

/* TODO:
 * - Ability to scroll through multiple palettes (if the entry contains more than 1)
 * - Ability to export to custom palettes directory
 * - Ability to edit the palette (though might make a full-fledged PLAYPAL/COLORMAP editor)
 */
class PaletteEntryPanel : public EntryPanel {
private:
	PaletteCanvas*	pal_canvas;

public:
	PaletteEntryPanel(wxWindow* parent);
	~PaletteEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
};

#endif //__PALETTEENTRYPANEL_H__
