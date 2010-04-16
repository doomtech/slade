
#ifndef __CTEXTURECANVAS_H__
#define __CTEXTURECANVAS_H__

#include "OGLCanvas.h"
#include "CTexture.h"
#include "Palette.h"
#include "GLTexture.h"

wxDECLARE_EVENT(EVT_DRAG_END, wxCommandEvent);

class CTextureCanvas : public OGLCanvas {
private:
	CTexture			texture;
	vector<GLTexture*>	patch_textures;
	GLTexture*			tex_background;
	vector<bool>		selected_patches;
	int					hilight_patch;
	fpoint2_t			offset;
	point2_t			mouse_prev;
	double				scale;
	bool				draw_outside;
	bool				dragging;

public:
	CTextureCanvas(wxWindow* parent, int id);
	~CTextureCanvas();

	CTexture&	getTexture() { return texture; }
	void		setScale(double scale) { this->scale = scale; }
	void		drawOutside(bool draw = true) { draw_outside = draw; }
	point2_t	getMousePrevPos() { return mouse_prev; }
	bool		isDragging() { return dragging; }

	void	selectPatch(int index);
	void	deSelectPatch(int index);
	bool	patchSelected(int index);

	void	clearTexture();
	void	clearPatchTextures();
	void	updatePatchTextures();
	bool	openTexture(tx_texture_t& tex, PatchTable& ptable);
	void	draw();
	void	drawChequeredBackground();
	void	drawTexture();
	void	drawPatch(int num, rgba_t col = COL_WHITE);
	void	drawTextureBorder();
	void	resetOffsets() { offset.x = offset.y = 0; }

	point2_t	screenToTexPosition(int x, int y);
	int			patchAt(int x, int y);

	bool	swapPatches(size_t p1, size_t p2);

	void	onMouseEvent(wxMouseEvent& e);
};

#endif//__CTEXTURECANVAS_H__
