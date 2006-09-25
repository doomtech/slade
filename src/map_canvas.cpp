
#include "main.h"
#include "map_canvas.h"
#include "structs.h"
#include "textures.h"
#include "keybind.h"
#include "input.h"
#include "edit.h"
#include "editor_window.h"
#include "draw.h"
#include "doom_map.h"
#include "mathstuff.h"
#include "edit_misc.h"
#include "copypaste.h"

/*
#include "map.h"


#include "linedraw.h"
#include "undoredo.h"
*/

//bool thing_quickangle = false;
//bool items_moving = false;
//bool paste_mode = false;

CVAR(Bool, draw_debuginfo, false, CVAR_SECRET)
CVAR(Float, line_size, 1.5, CVAR_SAVE)
CVAR(Int, crosshair_2d, 0, CVAR_SAVE)
CVAR(Bool, thing_sprites, false, CVAR_SAVE)
CVAR(Bool, thing_force_angle, false, CVAR_SAVE)
CVAR(Bool, grid_dashed, false, CVAR_SAVE)
CVAR(Bool, grid_64grid, true, CVAR_SAVE)
CVAR(Bool, grid_origin, true, CVAR_SAVE)
CVAR(Bool, line_aa, true, CVAR_SAVE)
CVAR(Bool, zoom_mousecursor, false, CVAR_SAVE)
CVAR(Bool, pan_detail, true, CVAR_SAVE)

vector<string> pressed_keys;
vector<string> released_keys;

//wxGLContext *gl_context = NULL;
wxGLCanvas *share_canvas = NULL;

extern bool allow_tex_load, lock_hilight;
extern BindList binds;
extern EditorWindow *editor_window;
extern rgba_t col_background, col_grid, col_64grid, col_selbox, col_selbox_line, col_crosshair;
extern DoomMap d_map;
extern int gridsize, edit_mode;
extern Clipboard clipboard;

/*
extern int vid_width, vid_height, hilight_item, edit_mode;

extern point2_t mouse, down_pos;
extern bool allow_tex_load;//, line_draw;


extern rgba_t col_line_solid;
extern float xoff, yoff;

extern int time_upgrid, time_upmap, time_draw;

EXTERN_CVAR(Bool, line_aa)
*/

MapCanvas::MapCanvas(wxWindow *parent)
:	wxGLCanvas(parent, -1)
{
	share_canvas = this;
}

MapCanvas::~MapCanvas()
{
}

void MapCanvas::init()
{
	SetCurrent();
	glViewport(0, 0, GetSize().x, GetSize().y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	allow_tex_load = true;
	wxSafeYield();
	glEnable(GL_TEXTURE_2D);
	init_textures();

	xoff = 0;
	yoff = 0;
	zoom = 1;
	sel_box.set(-1, -1, -1, -1);
}

point2_t MapCanvas::mouse_pos(bool translate)
{
	if (translate)
		return point2_t(lround(translate_x(mouse.x)), lround(translate_y(mouse.y)));
	else
		return mouse;
}

point2_t MapCanvas::get_down_pos(bool translate)
{
	if (translate)
		return point2_t(lround(translate_x(down_pos.x)), lround(translate_y(down_pos.y)));
	else
		return down_pos;
}

bool MapCanvas::selection()
{
	return (sel_box.x1() != -1);
}

void MapCanvas::pan_view(double x, double y, bool redraw)
{
	xoff += (x/zoom);
	yoff += (y/zoom);

	if (redraw) this->redraw();
}

bool MapCanvas::zoom_view(double factor, bool redraw, bool mouse)
{
	bool limit = false;

	fpoint2_t mpos(translate_x(this->mouse.x), translate_y(this->mouse.y));

	zoom = zoom * factor;
	if (zoom < 0.005f)
	{
		zoom = 0.005f;
		limit = true;
	}

	if (zoom > 10.0f)
	{
		zoom = 10.0f;
		limit = true;
	}

	if (!limit && zoom_mousecursor && mouse)
	{
		xoff = mpos.x + (double(xoff - mpos.x) / factor);
		yoff = mpos.y + (double(yoff - mpos.y) / factor);
	}

	if (redraw) this->redraw();

	return limit;
}

void MapCanvas::set_view()
{
	// Translate to middle of screen
	glTranslated(GetClientSize().x * 0.5, GetClientSize().y * 0.5, 0);

	// Flip Y axis
	glRotated(180, 1, 0, 0);

	// Zoom
	glScaled(zoom, zoom, 0);

	// Offsets (-ve)
	glTranslated(-xoff, -yoff, 0);
}

void MapCanvas::set_offsets(point2_t offsets, bool redraw)
{
	xoff = offsets.x;
	yoff = offsets.y;

	if (redraw) this->redraw();
}

fpoint2_t MapCanvas::translate(point2_t point)
{
	double x = translate_x(point.x);
	double y = translate_y(point.y);

	return fpoint2_t(x, y);
}

double MapCanvas::translate_x(int x)
{
	return double(x / zoom) + xoff - (double(GetClientSize().x * 0.5) / zoom);
}

double MapCanvas::translate_y(int y)
{
	return -double(y / zoom) + yoff + (double(GetClientSize().y * 0.5) / zoom);
}

rect_t MapCanvas::translate_rect(rect_t rect)
{
	int x1 = translate_x(rect.x1());
	int y1 = translate_y(rect.y1());
	int x2 = translate_x(rect.x2());
	int y2 = translate_y(rect.y2());

	return rect_t(x1, y1, x2, y2);
}

rect_t MapCanvas::get_border()
{
	point2_t tl(0, 0);
	point2_t br(GetClientSize().x, GetClientSize().y);

	return rect_t(translate(tl).x, translate(br).y, translate(br).x, translate(tl).y);
}

rect_t MapCanvas::get_selection()
{
	return translate_rect(rect_t(sel_box.left(), sel_box.top(), sel_box.right(), sel_box.bottom()));
}

void MapCanvas::draw_grid()
{
	int grid_hidelevel;

	glDisable(GL_LINE_SMOOTH); // AA is unnecessary (grid lines are not diagonal)
	glLineWidth(1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	grid_hidelevel = 1 / zoom;

	int start_x = translate_x(0);
	int end_x = translate_x(GetClientSize().x);
	int start_y = translate_y(GetClientSize().y);
	int end_y = translate_y(0);

	if (grid_dashed)
	{
		glLineStipple(2, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
	}

	// If the grid should be drawn (not to small)
	int xs = 1;
	int ys = 1;
	if (gridsize > grid_hidelevel && gridsize > 0)
	{
		for (int x = start_x; x < end_x; x += xs)
		{
			if (xs != 1)
				draw_line(rect_t(x, start_y, x, end_y), col_grid, false);
			else
			{
				if (x % gridsize == 0)
				{
					xs = gridsize;
					draw_line(rect_t(x, start_y, x, end_y), col_grid, false);
				}
			}

			if (x == 0 && grid_origin)
				draw_line(rect_t(x, start_y, x, end_y), col_grid.ampf(1.0f, 1.0f, 1.0f, 0.75f), false);
		}

		for (int y = start_y; y < end_y; y += ys)
		{
			if (ys != 1)
				draw_line(rect_t(start_x, y, end_x, y), col_grid, false);
			else
			{
				if (y % gridsize == 0)
				{
					draw_line(rect_t(start_x, y, end_x, y), col_grid, false);
					ys = gridsize;
				}
			}

			if (y == 0 && grid_origin)
				draw_line(rect_t(start_x, y, end_x, y), col_grid.ampf(1.0f, 1.0f, 1.0f, 0.75f), false);
		}
	}

	if (64 < grid_hidelevel	// Don't draw 64x64 grid if it'll be too small
		|| gridsize >= 64	// Don't draw 64x64 grid if the actual grid size is >=
		|| !grid_64grid)	// Also don't draw 64x64 grid if the cvar isn't set :P
	{
		glDisable(GL_LINE_STIPPLE);
		return;
	}

	xs = 1;
	for (int x = start_x; x < end_x; x += xs)
	{
		if (xs != 1)
			draw_line(rect_t(x, start_y, x, end_y), col_64grid, false);
		else
		{
			if (x % 64 == 0)
			{
				draw_line(rect_t(x, start_y, x, end_y), col_64grid, false);
				xs = 64;
			}
		}
	}

	ys = 1;
	for (int y = start_y; y < end_y; y += ys)
	{
		if (ys != 1)
			draw_line(rect_t(start_x, y, end_x, y), col_64grid, false);
		else
		{
			if (y % 64 == 0)
			{
				draw_line(rect_t(start_x, y, end_x, y), col_64grid, false);
				ys = 64;
			}
		}
	}

	glDisable(GL_LINE_STIPPLE);
}

void MapCanvas::draw_map()
{
	point2_t tl(0, 0);
	point2_t br(GetClientSize().x, GetClientSize().y);
	rect_t rect(translate(tl).x, translate(br).y, translate(br).x, translate(tl).y);

	glLineWidth(line_size);

	if (state(STATE_MAPPAN) && !pan_detail)
		d_map.draw_lines(rect);
	else
		d_map.draw(rect, edit_mode);

	if (state(STATE_PASTE))
		clipboard.DrawPaste();
}

void MapCanvas::redraw(bool map, bool grid)
{
	SetCurrent();

	glViewport(0, 0, GetSize().x, GetSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(col_background.fr(), col_background.fg(), col_background.fb(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	set_view();

	if (d_map.opened())
	{
		// Grid
		if (!(state(STATE_MAPPAN) && !pan_detail))
			draw_grid();

		// Crosshair
		if (crosshair_2d > 0)
		{
			int x = snap_to_grid(mouse_pos(true).x);
			int y = snap_to_grid(mouse_pos(true).y);

			glLineWidth(2.0f);

			// Full
			if (crosshair_2d == 1)
			{
				draw_line(rect_t(x, (int)translate_y(0),
							x, (int)translate_y(GetSize().y)),
							col_crosshair, false, false);

				draw_line(rect_t((int)translate_x(0), y,
							(int)translate_x(GetSize().x), y),
							col_crosshair, false, false);
			}

			// Small
			if (crosshair_2d == 2)
			{
				int size = 8/zoom;
				draw_line(rect_t(x, y-size, x, y+size), col_crosshair, false, false);
				draw_line(rect_t(x-size, y, x+size, y), col_crosshair, false, false);
			}

			glLineWidth(1.0f);
		}

		// Map
		draw_map();

		// Draw lines
		if (state(STATE_LINEDRAW) || state(STATE_SHAPEDRAW))
			ldraw_draw_lines(point2_t(translate_x(mouse.x), translate_y(mouse.y)));
	}

	glLoadIdentity();

	// Selection box
	if (sel_box.x1() != -1)
	{
		glLineWidth(2.0f);
		draw_rect(sel_box, col_selbox, true);
		draw_rect(sel_box, col_selbox_line, false);
		glLineWidth(1.0f);
	}

	SwapBuffers();
}

void MapCanvas::popup_menu()
{
	if (edit_mode == 1)
		PopupMenu(pmenu_line, wxPoint(down_pos.x, down_pos.y));

	if (edit_mode == 2)
		PopupMenu(pmenu_sector, wxPoint(down_pos.x, down_pos.y));

	if (edit_mode == 3)
		PopupMenu(pmenu_thing, wxPoint(down_pos.x, down_pos.y));
}

BEGIN_EVENT_TABLE(MapCanvas, wxGLCanvas)
	EVT_PAINT(MapCanvas::paint)
	//EVT_SIZE(MapCanvas::resize)
	EVT_MOUSE_EVENTS(MapCanvas::mouse_event)
	EVT_MOUSEWHEEL(MapCanvas::mouse_wheel)
	EVT_KEY_DOWN(MapCanvas::key_down)
	EVT_KEY_UP(MapCanvas::key_up)
END_EVENT_TABLE()


void MapCanvas::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	redraw();

	//event.RequestMore();
}

void MapCanvas::resize(wxSizeEvent& event)
{
	SetCurrent();

	glViewport(0, 0, GetSize().x, GetSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	redraw();

	OnSize(event);
}

void MapCanvas::mouse_event(wxMouseEvent &event)
{
	if (event.Moving() || event.Dragging())
		mouse_motion(event);

	if (event.ButtonDown())
		mouse_down(event);

	if (event.ButtonUp())
		mouse_up(event);

	if (event.ButtonDClick())
		mouse_down(event);

	if (event.Leaving())
	{
		binds.unset(_T("Mouse1"), &released_keys, false, false, false);
		binds.unset(_T("Mouse2"), &released_keys, false, false, false);
		binds.unset(_T("Mouse3"), &released_keys, false, false, false);

		if (state(STATE_MAPPAN))
		{
			SetCursor(wxCursor(wxCURSOR_ARROW));
			change_state();
		}

		if (sel_box.x1() != -1 && state())
			clear_selection();

		redraw();
	}

	mouse_wheel(event);
}

void MapCanvas::mouse_motion(wxMouseEvent& event)
{
	mouse.set(event.GetPosition().x, event.GetPosition().y);

	// Selection box
	if (sel_box.x1() != -1)
	{
		sel_box.br.set(mouse.x, mouse.y);

		if (state(STATE_LINEDRAW))
			editor_window->draw_shape();

		redraw();
	}
	else
	{
		if (binds.pressed(_T("edit_selectbox")))
		{
			sel_box.tl.set(down_pos);
			sel_box.br.set(down_pos);
			redraw();
		}
	}

	// Moving items
	point2_t m_mouse(lround(translate_x(mouse.x)), lround(translate_y(mouse.y)));
	if (state(STATE_MOVING))
	{
		d_map.move_items(m_mouse);
		redraw();
	}
	else
	{
		if (binds.pressed(_T("edit_moveitems")))
		{
			if (change_state(STATE_MOVING))
				d_map.move_items(m_mouse);
		}
	}

	/*
	// Quick thing angle
	if (state(STATE_THINGANGLE))
	{
		thing_setquickangle();
		update_map = true;
	}
	else
	{
		if (binds.pressed(_T("thing_quickangle")))
		{
			make_backup(false, false, false, false, true);
			if (change_state(STATE_THINGANGLE))
				thing_setquickangle();
			update_map = true;
		}
	}
	*/

	// Map panning
	if (state(STATE_MAPPAN))
	{
		xoff += (down_pos.x - mouse.x) / zoom;
		yoff -= (down_pos.y - mouse.y) / zoom;
		redraw();
		down_pos = mouse;
	}
	else
	{
		if (binds.pressed(_T("view_panmap")))
		{
			if (change_state(STATE_MAPPAN))
			{
				SetCursor(wxCursor(wxCURSOR_SIZING));
				down_pos = mouse;
			}
		}
	}

	// Paste
	if (state(STATE_PASTE))
		redraw();

	if (state(STATE_LINEDRAW) || state(STATE_SHAPEDRAW))
	{
		if (state(STATE_SHAPEDRAW))
			editor_window->draw_shape();

		redraw();
	}

	if (state() && sel_box.x1() == -1 && !lock_hilight && !event.Dragging())
	{
		int old_hilight = d_map.hilight();

		point2_t trans_mouse(translate_x(mouse.x), translate_y(mouse.y));

		switch(edit_mode) {
			case 0:
				d_map.get_hilight_vertex(trans_mouse);
				break;
			case 1:
				d_map.get_hilight_line(trans_mouse);
				break;
			case 2:
				d_map.get_hilight_sector(trans_mouse);
				break;
			case 3:
				d_map.get_hilight_thing(trans_mouse);
				break;
		}

		if (old_hilight != d_map.hilight())
		{
			editor_window->update_infobar();
			d_map.update_tagged(edit_mode);
			redraw();
		}
		else if (crosshair_2d > 0)
			redraw();
	}

	editor_window->update_statusbar();
}

void MapCanvas::mouse_down(wxMouseEvent& event)
{
	SetFocus();

	if (state(STATE_PASTE))
	{
		if (event.Button(wxMOUSE_BTN_LEFT))
		{
			change_state();
			clipboard.Paste();
		}
	}
	else
	{
		down_pos.set(mouse);
		string key;

		if (event.Button(wxMOUSE_BTN_LEFT))
			binds.set(_T("Mouse1"), &pressed_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());

		if (event.Button(wxMOUSE_BTN_RIGHT))
			binds.set(_T("Mouse3"), &pressed_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());

		if (event.Button(wxMOUSE_BTN_MIDDLE))
			binds.set(_T("Mouse2"), &pressed_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());

		if (state(STATE_LINEDRAW) || state(STATE_SHAPEDRAW))
			keys_linedraw();
		else
			keys_edit();

		pressed_keys.clear();
	}

	redraw();
}

void MapCanvas::mouse_up(wxMouseEvent& event)
{
	if (state(STATE_PASTE))
		return;

	if (event.Button(wxMOUSE_BTN_LEFT))
		binds.unset(_T("Mouse1"), &released_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.Button(wxMOUSE_BTN_RIGHT))
		binds.unset(_T("Mouse3"), &released_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.Button(wxMOUSE_BTN_MIDDLE))
		binds.unset(_T("Mouse2"), &released_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (state(STATE_LINEDRAW) || state(STATE_SHAPEDRAW))
		keys_linedraw();
	else
		keys_edit();

	released_keys.clear();
}

void MapCanvas::mouse_wheel(wxMouseEvent& event)
{
	string key = _T("");

	if (event.GetWheelRotation() > 0)
		key = _T("MWheel Up");
	else if (event.GetWheelRotation() < 0)
		key = _T("MWheel Down");
	else
		return;

	// 'Press' wheel 'key'
	down_pos.set(mouse);
	binds.set(key, &pressed_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());
	keys_edit();
	pressed_keys.clear();

	// 'Release' wheel 'key'
	binds.unset(key, &released_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());
	keys_edit();
	released_keys.clear();
}

void MapCanvas::key_down(wxKeyEvent &event)
{
	released_keys.clear();
	pressed_keys.clear();

	down_pos.set(mouse);
	binds.set(get_key_name(event.GetKeyCode()), &pressed_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());
	if (state(STATE_LINEDRAW) || state(STATE_SHAPEDRAW))
		keys_linedraw();
	else
		keys_edit();
	pressed_keys.clear();

	event.Skip();
}

void MapCanvas::key_up(wxKeyEvent &event)
{
	released_keys.clear();
	pressed_keys.clear();

	binds.unset(get_key_name(event.GetKeyCode()), &released_keys, event.ShiftDown(), event.ControlDown(), event.AltDown());
	if (state(STATE_LINEDRAW) || state(STATE_SHAPEDRAW))
		keys_linedraw();
	else
		keys_edit();
	released_keys.clear();

	event.Skip();
}
