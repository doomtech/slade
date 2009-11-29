
#ifndef __GFXCANVAS_H__
#define	__GFXCANVAS_H__

#include "OGLCanvas.h"
#include "SImage.h"

// Enumeration for view types
enum {
	GFXVIEW_DEFAULT,
	GFXVIEW_CENTERED,
	GFXVIEW_SPRITE,
	GFXVIEW_HUD,
	GFXVIEW_TILED,
};

class GfxCanvas : public OGLCanvas, Listener {
private:
	SImage*			image;
	int				view_type;	// 0=default, 1=centered, 2=sprite offsets, 3=hud offsets, 4=tiled
	double			scale;
	GLuint			gl_id;
	bool			update_texture;
	bool			image_hilight;
	bool			allow_drag;
	point2_t		drag_pos;
	point2_t		drag_origin;

public:
	GfxCanvas(wxWindow* parent, int id);
	~GfxCanvas();

	SImage*	getImage() { return image; }

	void	setViewType(int type) { view_type = type; }
	void	setScale(double scale) { this->scale = scale; }
	bool	allowDrag() { return allow_drag; }
	void	allowDrag(bool allow) { allow_drag = allow; }

	void	draw();
	void	drawChequeredBackground();
	void	drawImage();
	void	drawOffsetLines();
	void	updateImageTexture();
	void	endOffsetDrag();

	void	zoomToFit(bool mag = true, float padding = 0.0f);
	bool	onImage(int x, int y);

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void	onMouseLeftDown(wxMouseEvent& e);
	void	onMouseLeftUp(wxMouseEvent& e);
	void	onMouseMovement(wxMouseEvent& e);
};

DECLARE_EVENT_TYPE(wxEVT_GFXCANVAS_OFFSET_CHANGED, -1)

#endif //__GFXCANVAS_H__
