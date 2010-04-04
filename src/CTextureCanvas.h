
#ifndef __CTEXTURECANVAS_H__
#define __CTEXTURECANVAS_H__

#include "OGLCanvas.h"
#include "CTexture.h"
#include "Palette.h"
#include "GLTexture.h"

class CTextureCanvas : public OGLCanvas {
private:
	CTexture*			texture;
	vector<GLTexture*>	patch_textures;
	GLTexture*			tex_background;

public:
	CTextureCanvas(wxWindow* parent, int id);
	~CTextureCanvas();

	CTexture*	getTexture() { return texture; }

	void	clearTexture();
	void	clearPatchTextures();
	bool	openTexture(CTexture* tex);
	void	draw();
	void	drawChequeredBackground();
	void	drawTexture();
	void	drawPatch(int num);
	void	drawTextureBorder();
};

#endif//__CTEXTURECANVAS_H__
