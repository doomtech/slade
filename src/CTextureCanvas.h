
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
	int					selected_patch;
	fpoint2_t			offset;
	point2_t			mouse_prev;

public:
	CTextureCanvas(wxWindow* parent, int id);
	~CTextureCanvas();

	CTexture*	getTexture() { return texture; }

	void	selectPatch(int index) { selected_patch = index; }
	void	clearTexture();
	void	clearPatchTextures();
	bool	openTexture(CTexture* tex);
	void	draw();
	void	drawChequeredBackground();
	void	drawTexture();
	void	drawPatch(int num);
	void	drawTextureBorder();
	void	resetOffsets() { offset.x = offset.y = 0; }
	void	onMouseMovement(wxMouseEvent& e);
};

#endif//__CTEXTURECANVAS_H__
