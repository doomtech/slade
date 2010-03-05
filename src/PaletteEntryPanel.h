
#ifndef __PALETTEENTRYPANEL_H__
#define	__PALETTEENTRYPANEL_H__

#include "EntryPanel.h"
#include "PaletteCanvas.h"

/* TODO:
 * - Ability to export to custom palettes directory
 * - Ability to edit the palette (though might make a full-fledged PLAYPAL/COLORMAP editor)
 */
class PaletteEntryPanel : public EntryPanel {
private:
	PaletteCanvas*			pal_canvas;
	vector<Palette8bit*>	palettes;
	uint32_t				cur_palette;
	
	wxButton*				btn_nextpal;
	wxButton*				btn_prevpal;
	wxStaticText*			text_curpal;

public:
	PaletteEntryPanel(wxWindow* parent);
	~PaletteEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
	
	bool	showPalette(uint32_t index);
	
	// Events
	void	onBtnNextPal(wxCommandEvent& e);
	void	onBtnPrevPal(wxCommandEvent& e);
};

#endif //__PALETTEENTRYPANEL_H__
