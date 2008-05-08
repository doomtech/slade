
#ifndef __MAP_CANVAS_H__
#define __MAP_CANVAS_H__

#include "structs.h"
#include "wx_stuff.h"
#include <wx/glcanvas.h>

class MapCanvas : public wxGLCanvas
{
private:
	wxMenu			*pmenu_line;
	wxMenu			*pmenu_sector;
	wxMenu			*pmenu_thing;

	double			xoff;
	double			yoff;
	double			zoom;

	point2_t		down_pos;
	point2_t		mouse;
	rect_t			sel_box;

	GLuint			map_list;
	GLuint			grid_list;

protected:

public:
	MapCanvas(wxWindow *parent);
	~MapCanvas();

	point2_t mouse_pos(bool translate);
	point2_t get_down_pos(bool translate);

	void init();

	double get_zoom() { return zoom; }

	bool selection();
	void clear_selection() { sel_box.set(-1, -1, -1, -1); }

	void draw_grid();
	void draw_map();
	void redraw(bool map = false, bool grid = false);
	void popup_menu();

	void pan_view(double x, double y, bool redraw = true);
	bool zoom_view(double factor, bool redraw = true);
	void set_view();
	void set_offsets(point2_t offsets, bool redraw = true);

	fpoint2_t translate(point2_t point);
	double translate_x(int x);
	double translate_y(int y);
	rect_t translate_rect(rect_t rect);

	rect_t get_border();
	rect_t get_selection();

	void set_menus(wxMenu* line, wxMenu* sector, wxMenu* thing)
	{
		this->pmenu_line = line;
		this->pmenu_sector = sector;
		this->pmenu_thing = thing;
	}

	// Window events
	void paint(wxPaintEvent &event);
	void resize(wxSizeEvent& event);
	void key_down(wxKeyEvent &event);
	void key_up(wxKeyEvent &event);

	void mouse_event(wxMouseEvent& event);
	void mouse_motion(wxMouseEvent& event);
	void mouse_down(wxMouseEvent& event);
	void mouse_up(wxMouseEvent& event);
	void mouse_wheel(wxMouseEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif
