
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
	wxStaticText*	label_dimensions;
	PaletteChooser*	combo_palette;
	wxComboBox*		combo_offset_type;
	wxSpinCtrl*		spin_xoffset;
	wxSpinCtrl*		spin_yoffset;
	wxCheckBox*		cb_tile;

public:
	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
	void	updateImagePalette();
	int		detectOffsetType();
	void	applyViewType();

	void	onZoomChanged(wxCommandEvent& e);
	void	onPaletteChanged(wxCommandEvent& e);
	void	onXOffsetChanged(wxSpinEvent& e);
	void	onYOffsetChanged(wxSpinEvent& e);
	void	onOffsetTypeChanged(wxCommandEvent& e);
	void	onTileChanged(wxCommandEvent& e);
	void	onGfxOffsetChanged(wxEvent& e);
};

#endif //__GFXENTRYPANEL_H__
