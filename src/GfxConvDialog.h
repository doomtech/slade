
#ifndef __GFXCONVDIALOG_H__
#define	__GFXCONVDIALOG_H__

#include <wx/dialog.h>
#include "GfxCanvas.h"

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
 *			- Keep palette indices (only if converting from 32bit)
 *			- Nearest colour matching
 *
 *	Transparency:
 *		- Specify threshold alpha, anything above is opaque (optional if converting to 32bit)
 *		- Specify transparency info:
 *			- Keep existing transparency (threshold comes into play from 32bit-paletted)
 *			- Select transparency colour (to 32bit - select colour, to paletted - select from target palette)
 */

class GfxConvDialog : public wxDialog {
private:
	GfxCanvas*		gfx_current;
	GfxCanvas*		gfx_preview;
	vector<SImage*>	images;

	// Conversion options
	Palette8bit		target_pal;
	uint8_t			pal_convert_type;	// 0=nearest colour, 1=keep indices
	uint8_t			alpha_threshold;
	bool			keep_trans;
	rgba_t			colour_trans;

public:
	GfxConvDialog();
	~GfxConvDialog();

	bool	openImage(SImage* img);
};

#endif //__GFXCONVDIALOG_H__
