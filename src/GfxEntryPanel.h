
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"
#include <wx/glcanvas.h>

class GfxCanvas : public wxGLCanvas {
private:
	wxGLContext*	context;

public:
	GfxCanvas(wxWindow* parent, int id);
	~GfxCanvas();

	bool	setContext();
	void	draw();
	void	drawChequeredBackground();

	void	paint(wxPaintEvent &e);
	void	resize(wxSizeEvent &e);

	DECLARE_EVENT_TABLE()
};

class GfxEntryPanel : public EntryPanel {
private:
	GfxCanvas*	gfx_canvas;

public:
	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
};

#endif //__GFXENTRYPANEL_H__
