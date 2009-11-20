
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"
#include "GfxCanvas.h"
#include "PaletteChooser.h"

class GfxEntryPanel : public EntryPanel {
private:
	GfxCanvas*		gfx_canvas;
	wxSlider*		slider_zoom;
	wxStaticText*	text_current_zoom;
	PaletteChooser*	combo_palette;

public:
	enum {
		SLIDER_ZOOM,
		COMBO_PALETTE,
	};

	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	void	updateImagePalette();
	
	void	sliderZoomChanged(wxScrollEvent& e);
	void	paletteChanged(wxCommandEvent& e);

	DECLARE_EVENT_TABLE()
};

#endif //__GFXENTRYPANEL_H__
