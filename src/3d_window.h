
#include "wx_stuff.h"
#include <wx/glcanvas.h>

class Render3dCanvas : public wxGLCanvas
{
private:

public:
	Render3dCanvas(wxWindow *parent);
	~Render3dCanvas();

	void init();
	void do_loop();
	void render();

	void idle_event(wxIdleEvent &event);
	void paint_event(wxPaintEvent &event);
	void key_down(wxKeyEvent &event);
	void key_up(wxKeyEvent &event);
	void mouse_event(wxMouseEvent &event);
	void size_event(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()
};

void start_3d_mode();
