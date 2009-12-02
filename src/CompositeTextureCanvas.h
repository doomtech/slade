
#ifndef __COMPOSITETEXTURECANVAS_H__
#define __COMPOSITETEXTURECANVAS_H__

#include "OGLCanvas.h"
#include "CompositeTexture.h"

class CompositeTextureCanvas : public OGLCanvas {
private:
	CompositeTexture*	texture;

public:
	CompositeTextureCanvas(wxWindow* parent, int id);
	~CompositeTextureCanvas();

	CompositeTexture*	getTexture() { return texture; }

	bool	openTexture(CompositeTexture* tex);
	void	draw();
	void	drawChequeredBackground();
};

#endif//__COMPOSITETEXTURECANVAS_H__
