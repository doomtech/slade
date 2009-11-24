
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"
#include "GfxCanvas.h"
#include "PaletteChooser.h"
#include <wx/spinctrl.h>

class GfxEntryPanel : public EntryPanel {
private:
	GfxCanvas*		gfx_canvas;
	wxSlider*		slider_zoom;
	wxStaticText*	label_current_zoom;
	PaletteChooser*	combo_palette;
	wxComboBox*		combo_offset_type;
	wxSpinCtrl*		spin_xoffset;
	wxSpinCtrl*		spin_yoffset;

public:
	enum {
		SLIDER_ZOOM,
		COMBO_PALETTE,
		COMBO_OFFSET_TYPE,
		SPIN_XOFFSET,
		SPIN_YOFFSET,
	};

	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
	void	updateImagePalette();
	int		detectOffsetType();
	void	applyViewType();

	void	sliderZoomChanged(wxScrollEvent& e);
	void	paletteChanged(wxCommandEvent& e);
	void	spinXOffsetChanged(wxSpinEvent& e);
	void	spinYOffsetChanged(wxSpinEvent& e);
	void	comboOffsetTypeChanged(wxCommandEvent& e);

	DECLARE_EVENT_TABLE()
};

#endif //__GFXENTRYPANEL_H__
