
#ifndef __GFXCANVAS_H__
#define	__GFXCANVAS_H__

#include "SImage.h"
#include <wx/glcanvas.h>

class GfxCanvas : public wxGLCanvas {
private:
	wxGLContext*	context;
	SImage*			image;
	bool			init_done;
	uint8_t			offset_type;	// 0=none, 1=sprite, 2=hud
	double			scale;

public:
	GfxCanvas(wxWindow* parent, int id);
	~GfxCanvas();

	SImage*	getImage() { return image; }

	bool	setContext();
	void	init();
	void	draw();
	void	drawChequeredBackground();
	void	drawImage();
	void	drawOffsetLines();

	void	zoomToFit();

	void	paint(wxPaintEvent &e);
	void	onEraseBackground(wxEraseEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__GFXCANVAS_H__
