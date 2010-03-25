
#ifndef __GFXCONVDIALOG_H__
#define	__GFXCONVDIALOG_H__

#include <wx/dialog.h>
#include "GfxCanvas.h"
#include "ArchiveEntry.h"
#include "PaletteChooser.h"
#include "ColourBox.h"

/* Convert from anything to:
 * Doom Gfx
 * Doom Flat
 * PNG 32bit
 * PNG Paletted
 *
 * Conversion options:
 *	Colours:
 *		- Specify target palette (only if converting to paletted)
 *		- Specify palette conversion type:
 *			- Keep palette indices (only if converting from 8bit)
 *			- Nearest colour matching
 *
 *	Transparency:
 *		- Specify threshold alpha, anything above is opaque (optional if converting from 32bit)
 *		- Specify transparency info:
 *			- Keep existing transparency (threshold comes into play from 32bit-paletted)
 *			- Select transparency colour (to 32bit - select colour, to paletted - select from target palette)
 */

class GfxConvDialog : public wxDialog {
private:
	vector<ArchiveEntry*>	entries;
	size_t					current_entry;

	GfxCanvas*		gfx_current;
	GfxCanvas*		gfx_target;
	wxButton*		btn_convert;
	wxButton*		btn_convert_all;
	wxButton*		btn_skip;
	wxButton*		btn_skip_all;
	wxComboBox*		combo_target_format;
	PaletteChooser*	pal_chooser_current;
	PaletteChooser*	pal_chooser_target;

	wxCheckBox*		cb_enable_transparency;
	wxRadioButton*	rb_transparency_existing;
	wxRadioButton*	rb_transparency_colour;
	wxSlider*		slider_alpha_threshold;
	ColourBox*		colbox_transparent;

	// Conversion options
	Palette8bit		target_pal;
	uint8_t			pal_convert_type;	// 0=nearest colour, 1=keep indices
	uint8_t			alpha_threshold;
	bool			keep_trans;
	rgba_t			colour_trans;

	void nextEntry();

public:
	GfxConvDialog();
	~GfxConvDialog();

	void	setupLayout();

	bool	openEntries(vector<ArchiveEntry*> entries);
	void	updatePreviewGfx();
	void	updateControls();
	bool	doConvert();
	bool	writeToEntry();

	// Events
	void	onResize(wxSizeEvent& e);
	void	onBtnConvert(wxCommandEvent& e);
	void	onBtnConvertAll(wxCommandEvent& e);
	void	onBtnSkip(wxCommandEvent& e);
	void	onBtnSkipAll(wxCommandEvent& e);
	void	onTargetFormatChanged(wxCommandEvent& e);
	void	onCurrentPaletteChanged(wxCommandEvent& e);
	void	onTargetPaletteChanged(wxCommandEvent& e);
	void	onAlphaThresholdChanged(wxCommandEvent& e);
	void	onEnableTransparencyChanged(wxCommandEvent& e);
	void	onTransTypeChanged(wxCommandEvent& e);
	void	onTransColourChanged(wxEvent& e);
};

#endif //__GFXCONVDIALOG_H__
