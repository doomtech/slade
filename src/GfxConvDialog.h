
#ifndef __GFXCONVDIALOG_H__
#define	__GFXCONVDIALOG_H__

#include <wx/dialog.h>
#include "GfxCanvas.h"
#include "ArchiveEntry.h"
#include "PaletteChooser.h"

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
	int						current_entry;

	GfxCanvas*		gfx_current;
	GfxCanvas*		gfx_target;
	wxButton*		btn_convert;
	wxButton*		btn_convert_all;
	wxButton*		btn_skip;
	wxButton*		btn_skip_all;
	wxComboBox*		combo_target_format;
	PaletteChooser*	pal_chooser_current;
	PaletteChooser*	pal_chooser_target;

	// Conversion options
	uint8_t			target_format;		// 0=dgfx, 1=dflat, 2=png32, 3=png8
	Palette8bit		target_pal;
	uint8_t			pal_convert_type;	// 0=nearest colour, 1=keep indices
	uint8_t			alpha_threshold;
	bool			keep_trans;
	rgba_t			colour_trans;

	void nextEntry();

public:
	enum {
		BTN_CONVERT,
		BTN_CONVERT_ALL,
		BTN_SKIP,
		BTN_SKIP_ALL,
		COMBO_TARGET_FORMAT,
		PALETTE_CURRENT,
		PALETTE_TARGET,
	};

	GfxConvDialog();
	~GfxConvDialog();

	void	setupLayout();

	bool	openEntries(vector<ArchiveEntry*> entries);
	void	updatePreviewGfx();
	bool	doConvert();

	// Events
	void	resize(wxSizeEvent &e);
	void	btnConvertClicked(wxCommandEvent &e);
	void	btnConvertAllClicked(wxCommandEvent &e);
	void	btnSkipClicked(wxCommandEvent &e);
	void	btnSkipAllClicked(wxCommandEvent &e);
	void	comboTargetFormatChanged(wxCommandEvent &e);
	void	paletteCurrentChanged(wxCommandEvent &e);
	void	paletteTargetChanged(wxCommandEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__GFXCONVDIALOG_H__
