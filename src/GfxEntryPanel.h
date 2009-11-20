
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"
#include "GfxCanvas.h"
#include "PaletteChooser.h"

class GfxEntryPanel : public EntryPanel {
private:
	GfxCanvas*		gfx_canvas;
	wxSlider*		slider_zoom;
	wxStaticText*	label_current_zoom;
	PaletteChooser*	combo_palette;
	wxTextCtrl*		text_xoffset;
	wxTextCtrl*		text_yoffset;

public:
	enum {
		SLIDER_ZOOM,
		COMBO_PALETTE,
		TEXT_XOFFSET,
		TEXT_YOFFSET,
	};

	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
	void	updateImagePalette();

	void	sliderZoomChanged(wxScrollEvent& e);
	void	paletteChanged(wxCommandEvent& e);
	void	textXOffsetChanged(wxCommandEvent& e);
	void	textYOffsetChanged(wxCommandEvent& e);

	DECLARE_EVENT_TABLE()
};

#endif //__GFXENTRYPANEL_H__
