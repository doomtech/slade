
#ifndef __CTEXTURECANVAS_H__
#define __CTEXTURECANVAS_H__

#include "OGLCanvas.h"
#include "CTexture.h"

class CTextureCanvas : public OGLCanvas {
private:
	CTexture*	texture;
	vector<GLuint>		patch_gl_id;

public:
	CTextureCanvas(wxWindow* parent, int id);
	~CTextureCanvas();

	CTexture*	getTexture() { return texture; }

	bool	openTexture(CTexture* tex);
	void	draw();
	void	drawChequeredBackground();
	void	drawTexture();
	void	drawPatch(int num);
};

#endif//__CTEXTURECANVAS_H__
