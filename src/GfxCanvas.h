
#ifndef __GFXCANVAS_H__
#define	__GFXCANVAS_H__

#include "SImage.h"
#include <wx/glcanvas.h>

class GfxCanvas : public wxGLCanvas {
private:
	wxGLContext*	context;
	SImage*			image;
	bool			init_done;
	int				view_type;	// 0=default, 1=centered, 2=sprite offsets, 3=hud offsets
	double			scale;

public:
	GfxCanvas(wxWindow* parent, int id);
	~GfxCanvas();

	SImage*	getImage() { return image; }

	void	setViewType(int type) { view_type = type; }

	bool	setContext();
	void	init();
	void	draw();
	void	drawChequeredBackground();
	void	drawImage();
	void	drawOffsetLines();

	void	zoomToFit(bool mag = true, float padding = 0.0f);

	void	paint(wxPaintEvent &e);
	void	onEraseBackground(wxEraseEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__GFXCANVAS_H__
