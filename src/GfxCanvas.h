
#ifndef __GFXCANVAS_H__
#define	__GFXCANVAS_H__

#include "OGLCanvas.h"
#include "SImage.h"

class GfxCanvas : public OGLCanvas {
private:
	SImage*			image;
	int				view_type;	// 0=default, 1=centered, 2=sprite offsets, 3=hud offsets
	double			scale;
	GLuint			gl_id;

public:
	GfxCanvas(wxWindow* parent, int id);
	~GfxCanvas();

	SImage*	getImage() { return image; }

	void	setViewType(int type) { view_type = type; }
	void	setScale(double scale) { this->scale = scale; }

	void	draw();
	void	drawChequeredBackground();
	void	drawImage();
	void	drawOffsetLines();

	void	zoomToFit(bool mag = true, float padding = 0.0f);
};

#endif //__GFXCANVAS_H__
