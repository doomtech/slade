
#ifndef __COMPOSITETEXTURECANVAS_H__
#define __COMPOSITETEXTURECANVAS_H__

#include "OGLCanvas.h"
#include "CompositeTexture.h"

class CompositeTextureCanvas : public OGLCanvas {
private:
	CompositeTexture*	texture;
	vector<GLuint>		patch_gl_id;

public:
	CompositeTextureCanvas(wxWindow* parent, int id);
	~CompositeTextureCanvas();

	CompositeTexture*	getTexture() { return texture; }

	bool	openTexture(CompositeTexture* tex);
	void	draw();
	void	drawChequeredBackground();
	void	drawTexture();
	void	drawPatch(int num);
};

#endif//__COMPOSITETEXTURECANVAS_H__
