
#ifndef __PALETTEENTRYPANEL_H__
#define	__PALETTEENTRYPANEL_H__

#include "EntryPanel.h"
#include "PaletteCanvas.h"
#include "MainApp.h"

/* TODO:
 * - Ability to edit the palette (though might make a full-fledged PLAYPAL/COLORMAP editor)
 */
class PaletteEntryPanel : public EntryPanel, public SActionHandler {
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

	bool	loadEntry(ArchiveEntry* entry);
	string	statusString();
	
	bool	showPalette(uint32_t index);
	bool	addCustomPalette();
	bool	exportAs();

	// SAction handler
	bool	handleAction(string id);
	
	// Events
	void	onBtnNextPal(wxCommandEvent& e);
	void	onBtnPrevPal(wxCommandEvent& e);
	void	onPalCanvasMouseEvent(wxMouseEvent& e);
};

#endif //__PALETTEENTRYPANEL_H__
