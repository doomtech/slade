
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"
#include "SImage.h"
#include <wx/glcanvas.h>
#include <wx/aui/auibar.h>

class GfxCanvas : public wxGLCanvas {
private:
	wxGLContext*	context;
	SImage*			image;
	bool			init_done;
	uint8_t			offset_type;	// 0=none, 1=sprite, 2=hud

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

	void	paint(wxPaintEvent &e);
	void	onEraseBackground(wxEraseEvent &e);

	DECLARE_EVENT_TABLE()
};

class GfxEntryPanel : public EntryPanel {
private:
	GfxCanvas*		gfx_canvas;
	wxAuiToolBar*	toolbar;

public:
	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
};

#endif //__GFXENTRYPANEL_H__
