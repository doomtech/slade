
#ifndef __COLOURBOX_H__
#define	__COLOURBOX_H__

#include "Palette.h"

class ColourBox : public wxPanel {
private:
	rgba_t			colour;
	Palette8bit*	palette;
	bool			alpha;

public:
	ColourBox(wxWindow* parent, int id = -1, bool enable_alpha = false);
	~ColourBox();

	rgba_t	getColour() { return colour; }

	void	setPalette(Palette8bit* pal)	{ palette = pal; }
	void	setColour(rgba_t col)			{ colour = col; }

	// Events
	void	paint(wxPaintEvent& e);
	void	leftClick(wxMouseEvent& e);
	void	rightClick(wxMouseEvent& e);

	void	sendChangeEvent();

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxEVT_COLOURBOX_CHANGED, -1)

#endif//__COLOURBOX_H__
