// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004                   >>
// << ------------------------------------- >>
// << input.cpp - Input stuff               >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"
#include "keybind.h"
#include "editor_window.h"
#include "doom_map.h"
#include "dm_thing.h"
#include "edit.h"
#include "line_edit.h"
#include "thing_edit.h"
#include "sector_edit.h"
#include "camera.h"
#include "3d_window.h"
#include "render.h"
#include "3dmode.h"
#include "ttype_select.h"
#include "edit_misc.h"

/*
#include "map.h"
#include "undoredo.h"
#include "struct_3d.h"
#include "mathstuff.h"
#include "console.h"
//#include "console_window.h"
#include "copypaste.h"
#include "linedraw.h"
*/

CVAR(Float, move_speed_3d, 0.2f, CVAR_SAVE)
CVAR(Float, mouse_speed_3d, 1.0f, CVAR_SAVE)
CVAR(Int, key_delay_3d, 12, CVAR_SAVE)

#define PRESSED(name) vector_exists(pressed_keys, _T(name))
#define RELEASED(name) vector_exists(released_keys, _T(name))

// External Variables --------------------- >>
extern BindList binds;
extern int edit_mode;
extern bool lock_hilight;
extern Camera camera;
extern Thing last_thing;

/*
extern float xoff, yoff, zoom;
extern bool paste_mode, lock_hilight;
extern point2_t mouse, down_pos;
extern rect_t sel_box;
extern Clipboard clipboard;

*/
extern vector<string> pressed_keys;
extern vector<string> released_keys;

extern EditorWindow *editor_window;
extern DoomMap d_map;

EXTERN_CVAR(Bool, render_fog)
EXTERN_CVAR(Bool, render_sky)
EXTERN_CVAR(Bool, render_fullbright)
EXTERN_CVAR(Bool, render_hilight)
EXTERN_CVAR(Int, render_things)
EXTERN_CVAR(Bool, edit_snap_grid)

// cycle_edit_mode: Cycles the edit mode
// ----------------------------------- >>
void cycle_edit_mode()
{
	int nmode = edit_mode;

	if (++nmode > 3)
		nmode = 0;

	change_edit_mode(nmode);
	update_statusbar();
	editor_window->change_infobar();
}

point2_t down_pos(bool translate)
{
	return editor_window->map()->get_down_pos(translate);
}

// edit_item: When the edit item key is pressed
// ----------------------------------------- >>
void edit_item()
{
	/*
	if (edit_mode == 0)
	{
		create_vertex();
		get_hilight_item(mouse.x, mouse.y);
		editor_window->redraw_map(true, false);
		return;
	}

	if (hilight_item == -1 && !selection())
		return;
		*/

	//wxLogMessage("Edit Item");
	bool old_lock = lock_hilight;
	lock_hilight = true;

	if (edit_mode == 1)
	{
		LineEditor le;
		if (le.ShowModal() == wxID_OK)
			le.apply_changes();
	}

	if (edit_mode == 3)
	{
		ThingEditor te;
		if (te.ShowModal() == wxID_OK)
			te.apply_changes();
	}

	if (edit_mode == 2)
	{
		SectorEditor se;
		if (se.ShowModal() == wxID_OK)
			se.apply_changes();
	}

	redraw_map();
	lock_hilight = old_lock;
}

// keys_edit: Keys for the 2d editor
// ------------------------------ >>
void keys_edit()
{
	//wxLogMessage("keys_edit()");

	if (!d_map.opened())
		return;

	// **** Dragging stuff ****


	// Selection box
	if (editor_window->map()->selection() && RELEASED("edit_selectbox"))
	{
		rect_t selection = editor_window->map()->get_selection();

		if (edit_mode == 0)
			d_map.select_vertices_box(selection);

		if (edit_mode == 1)
			d_map.select_lines_box(selection);

		if (edit_mode == 2)
			d_map.select_sectors_box(selection);

		if (edit_mode == 3)
			d_map.select_things_box(selection);

		editor_window->map()->clear_selection();
		redraw_map();
		return;
	}

	// Moving items
	if (state(STATE_MOVING) && RELEASED("edit_moveitems"))
	{
		d_map.clear_move_items();
		change_state();
		redraw_map();
		return;
	}

	/*
	// Quick thing angle
	if (state(STATE_THINGANGLE) && RELEASED("thing_quickangle"))
	{
		//thing_quickangle = false;
		change_state();
		editor_window->redraw_map(true, false);
		return;
	}
	*/

	// Moving items
	if (state(STATE_MOVING) || editor_window->map()->selection())
		return;

	// Map panning
	if (state(STATE_MAPPAN) && RELEASED("view_panmap"))
	{
		change_state();
		redraw_map();
		return;
	}

	// **** Other controls ****

	// Scroll up
	if (PRESSED("view_up"))
		editor_window->map()->pan_view(0, 96);
	
	// Scroll down
	if (PRESSED("view_down"))
		editor_window->map()->pan_view(0, -96);

	// Scroll left
	if (PRESSED("view_left"))
		editor_window->map()->pan_view(-96, 0);
	
	// Scroll right
	if (PRESSED("view_right"))
		editor_window->map()->pan_view(96, 0);

	// Zoom in
	if (PRESSED("view_zoomin"))
		editor_window->map()->zoom_view(1.2);

	// Zoom out
	if (PRESSED("view_zoomout"))
		editor_window->map()->zoom_view(0.8);

	/*
	// Center view on mouse
	if (RELEASED("view_mousecenter"))
	{
		xoff = m_x(mouse.x);
		yoff = m_y(mouse.y);
		editor_window->redraw_map(true, true);
	}

	// Set offsets to 0, 0
	if (RELEASED("view_origin"))
	{
		xoff = yoff = 0;
		editor_window->redraw_map(true, true);
	}
	*/

	// Popup context menu
	if (RELEASED("view_contextmenu") && state())
	{
		binds.clear(_T("view_contextmenu"));
		editor_window->popup_context_menu();
	}

	/*
	// Sidebar stuff
	wxCommandEvent event;
	if (RELEASED("view_hidesidebar"))
		editor_window->view_hidesidebar(event);

	if (RELEASED("view_drawsidebar"))
		editor_window->view_drawsidebar(event);

	if (RELEASED("view_themesidebar"))
		editor_window->view_themesidebar(event);
	*/

	// Vertices mode
	if (RELEASED("mode_vertices"))
	{
		change_edit_mode(0);
		editor_window->change_infobar();
	}
	
	// Linedefs mode
	if (RELEASED("mode_linedefs"))
	{
		change_edit_mode(1);
		editor_window->change_infobar();
	}
	
	// Sectors mode
	if (RELEASED("mode_sectors"))
	{
		change_edit_mode(2);
		editor_window->change_infobar();
	}
	
	// Things mode
	if (RELEASED("mode_things"))
	{
		change_edit_mode(3);
		editor_window->change_infobar();
	}

	// Change mode
	if (RELEASED("mode_change"))
		cycle_edit_mode();

	// Increase grid size
	if (RELEASED("view_increasegrid"))
	{
		increase_grid();
		redraw_map(false, true);
	}

	// Decrease grid size
	if (RELEASED("view_decreasegrid"))
	{
		decrease_grid();
		redraw_map(false, true);
	}

	// Clear selection
	if (RELEASED("edit_clearselection") && state())
	{
		d_map.clear_selection();
		redraw_map();
	}

	// Delete item
	if (RELEASED("edit_deleteitem") && state())
	{
		d_map.delete_selection(edit_mode);
		redraw_map();
		//lock_hilight = false;
	}

	//if (PRESSED("edit_deleteitem") && state())
	//	lock_hilight = true;

	// Create item
	if (RELEASED("edit_createitem") && state())
	{
		if (edit_mode == 0)
		{
			if (d_map.selection().size() == 0)
				create_vertex(down_pos(true));
			else
				create_line(false);

			redraw_map();
			return;
		}

		if (edit_mode == 1)
		{
			create_sector();
			redraw_map();
			return;
		}

		if (edit_mode == 3)
		{
			create_thing(down_pos(true));
			redraw_map();
			return;
		}

		//get_hilight_item(mouse.x, mouse.y);
		binds.clear(_T("edit_createitem"));
	}

	// Create item 2
	if (RELEASED("edit_createitem2") && state())
	{
		if (edit_mode == 0)
		{
			if (d_map.selection().size() == 0)
				create_vertex(down_pos(true));
			else
				create_line(true);

			redraw_map();
			return;
		}

		if (edit_mode == 1)
		{
			create_sector();
			redraw_map(true);
			return;
		}

		if (edit_mode == 3)
		{
			TTypeSelectDialog td(last_thing.get_type());

			if (td.ShowModal() == wxID_OK)
			{
				last_thing.set_type(td.get_type());
				create_thing(down_pos(true));
			}

			redraw_map();
			return;
		}

		//get_hilight_item(mouse.x, mouse.y);
		binds.clear(_T("edit_createitem2"));
	}

	// Select item
	if (RELEASED("edit_selectitem") && state())
	{
		d_map.select_deselect();
		redraw_map();
	}

	// Sector height quick changes (8 units)
	if (PRESSED("sector_upfloor8"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, 8);
	}

	if (PRESSED("sector_downfloor8"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, -8);
	}

	if (PRESSED("sector_upceil8"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, 8);
	}

	if (PRESSED("sector_downceil8"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, -8);
	}

	if (PRESSED("sector_upboth8"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, 8);
			sector_changeheight(false, 8);
		}
	}

	if (PRESSED("sector_downboth8"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, -8);
			sector_changeheight(false, -8);
		}
	}

	// Sector height quick changes (1 unit)
	if (PRESSED("sector_upfloor"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, 1);
	}

	if (PRESSED("sector_downfloor"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, -1);
	}

	if (PRESSED("sector_upceil"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, 1);
	}

	if (PRESSED("sector_downceil"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, -1);
	}

	if (PRESSED("sector_upboth"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, 1);
			sector_changeheight(false, 1);
		}
	}

	if (PRESSED("sector_downboth"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, -1);
			sector_changeheight(false, -1);
		}
	}

	// Sector light change
	if (PRESSED("sector_uplight"))
	{
		//binds.clear("sector_uplight");
		sector_changelight(16);
	}

	if (PRESSED("sector_downlight"))
	{
		//binds.clear("sector_downlight");
		sector_changelight(-16);
	}

	// Flip line
	if (RELEASED("line_flip") && state())
	{
		if (edit_mode == 1)
			line_flip(true, false);

		redraw_map();
	}

	// Swap line sides
	if (RELEASED("line_swapsides") && state())
	{
		if (edit_mode == 1)
			line_flip(false, true);

		redraw_map();
	}

	// Flip both line direction and sides
	if (RELEASED("line_flipboth") && state())
	{
		if (edit_mode == 1)
			line_flip(true, true);

		redraw_map();
	}

	/*
	if (RELEASED("line_correctrefs") && state())
		line_correct_references();
		*/

	// Begin line draw
	if (PRESSED("line_begindraw"))
	{
		//if (!line_draw)
		//	line_draw = true;

		change_state(STATE_LINEDRAW);

		binds.clear(_T("line_begindraw"));
	}

	// Begin rectangle draw
	if (PRESSED("line_begindraw_rect"))
	{
		change_state(STATE_SHAPEDRAW);

		binds.clear(_T("line_begindraw_rect"));
	}

	/*
	// Undo
	if (RELEASED("edit_undo") && state())
	{
		undo();
		clear_selection();
		hilight_item = -1;
		editor_window->redraw_map(true, true);
		//map_changelevel(3);
		map.change_level(MC_NODE_REBUILD);
		binds.clear("edit_undo");
	}
	*/

	if (RELEASED("edit_createsector"))
	{
		vector<Sector*> n_s;
		sector_create(down_pos(true), n_s);
	}

	// Edit item
	if (RELEASED("edit_edititem") && state())
	{
		edit_item();
		binds.clear(_T("edit_edititem"));
	}

	if (RELEASED("line_split") && state())
	{
		line_split_at(down_pos(true));
		binds.clear(_T("line_split"));
	}

	// Merge sectors
	if (RELEASED("sector_merge") && state())
	{
		sector_merge(false);
		binds.clear("sector_merge");
	}

	// Join sectors
	if (RELEASED("sector_join") && state())
	{
		sector_merge(true);
		binds.clear("sector_join");
	}

	if (RELEASED("view_3dmode") && state())
	{
		binds.clear(_T("view_3dmode"));
		binds.clear(_T("3d_exit"));
		start_3d_mode();
	}

	/*
	if (RELEASED("open_console"))
	{
		binds.clear("open_console");
		//popup_console();
	}

	if (RELEASED("copy"))
	{
		binds.clear("copy");
		clipboard.Copy();
	}

	if (RELEASED("paste") && state())
	{
		binds.clear("paste");
		//paste_mode = true;
		change_state(STATE_PASTE);
		clear_selection();
	}

	if (RELEASED("cancel_paste"))
	{
		binds.clear("cancel_paste");
		//paste_mode = false;
		change_state();
		editor_window->redraw_map(true, false);
	}
	*/

	// Toggle grid snap
	if (RELEASED("edit_gridsnap"))
	{
		binds.clear("edit_gridsnap");
		edit_snap_grid = !edit_snap_grid;
		redraw_map();
	}

	// Toggle hilight lock
	if (RELEASED("edit_lockhilight"))
	{
		binds.clear("edit_lockhilight");
		lock_hilight = !lock_hilight;
	}
}

void keys_linedraw()
{
	// Scroll up
	if (PRESSED("view_up"))
		editor_window->map()->pan_view(0, 96);
	
	// Scroll down
	if (PRESSED("view_down"))
		editor_window->map()->pan_view(0, -96);

	// Scroll left
	if (PRESSED("view_left"))
		editor_window->map()->pan_view(-96, 0);
	
	// Scroll right
	if (PRESSED("view_right"))
		editor_window->map()->pan_view(96, 0);

	// Zoom in
	if (PRESSED("view_zoomin"))
		editor_window->map()->zoom_view(1.2);

	// Zoom out
	if (PRESSED("view_zoomout"))
		editor_window->map()->zoom_view(0.8);

	// Increase grid size
	if (RELEASED("view_increasegrid"))
	{
		increase_grid();
		redraw_map(false, true);
	}

	// Decrease grid size
	if (RELEASED("view_decreasegrid"))
	{
		decrease_grid();
		redraw_map(false, true);
	}

	// Draw point
	if (RELEASED("ldraw_drawpoint"))
	{
		ldraw_addpoint(false);
		redraw_map();
	}

	// Undraw point
	if (RELEASED("ldraw_cancelpoint"))
	{
		ldraw_removepoint();
		redraw_map();
	}

	// Draw point at nearest vertex
	if (RELEASED("ldraw_nearestvert"))
	{
		ldraw_addpoint(true);
		redraw_map();
	}

	/*
	// Scroll up
	if (PRESSED("view_up"))
	{
		yoff -= ((1.0f / zoom) * 64);
		//yoff += ((MAJOR_UNIT / (int)zoom)) + 1;
		editor_window->redraw_map(true, true);
	}
	
	// Scroll down
	if (PRESSED("view_down"))
	{
		yoff += ((1.0f / zoom) * 64);
		//yoff -= ((MAJOR_UNIT / (int)zoom)) + 1;
		editor_window->redraw_map(true, true);
	}

	// Scroll left
	if (PRESSED("view_left"))
	{
		xoff -= ((1.0f / zoom) * 64);
		//xoff += ((MAJOR_UNIT / (int)zoom)) + 1;
		editor_window->redraw_map(true, true);
	}
	
	// Scroll right
	if (PRESSED("view_right"))
	{
		xoff += ((1.0f / zoom) * 64);
		//xoff -= ((MAJOR_UNIT / (int)zoom)) + 1;
		editor_window->redraw_map(true, true);
	}

	// Zoom in
	if (PRESSED("view_zoomin"))
		view_zoom(true);

	// Zoom out
	if (PRESSED("view_zoomout"))
		view_zoom(false);

	// Increase grid size
	if (RELEASED("view_increasegrid"))
	{
		increase_grid();
		editor_window->redraw_map(false, true);
	}

	// Decrease grid size
	if (RELEASED("view_decreasegrid"))
	{
		decrease_grid();
		editor_window->redraw_map(false, true);
	}

	// Toggle grid snap
	if (RELEASED("edit_gridsnap"))
	{
		binds.clear("edit_gridsnap");
		edit_snap_grid = !edit_snap_grid;
		editor_window->redraw_map(false, false);
	}

	// Draw point
	if (RELEASED("ldraw_drawpoint"))
	{
		line_drawpoint(false);
		editor_window->redraw_map(false, false);
	}

	// Undraw point
	if (RELEASED("ldraw_cancelpoint"))
	{
		line_undrawpoint();
		editor_window->redraw_map(false, false);
	}

	// Draw point at nearest vertex
	if (RELEASED("ldraw_nearestvert"))
	{
		line_drawpoint(true);
		editor_window->redraw_map(false, false);
	}
	*/
}

//#define KEY_3D_DELAY 7
double key_3d_rep = 0;

bool keys_3d(float mult, bool mwheel)
{
	bool key_3d_allow = false;
	float speed = (20 * move_speed_3d) * mult;

	if (key_3d_rep == 0)
		key_3d_allow = true;

	if (binds.pressed(_T("3d_exit")))
	{
		binds.clear(_T("view_3dmode"));
		binds.clear(_T("3d_exit"));
		return false;
	}

	if (binds.pressed("3d_forward"))
		camera.move_camera(speed);

	if (binds.pressed("3d_back"))
		camera.move_camera(-speed);

	if (binds.pressed("3d_left"))
		camera.rotate_view((speed*0.002f), 0.0f, 0.0f, 1.0f);

	if (binds.pressed("3d_right"))
		camera.rotate_view(-(speed*0.002f), 0.0f, 0.0f, 1.0f);

	if (binds.pressed("3d_strafeleft"))
		camera.strafe_camera(-speed);

	if (binds.pressed("3d_straferight"))
		camera.strafe_camera(speed);

	if (binds.pressed("3d_moveup"))
	{
		camera.position.z += speed;
		camera.view.z += speed;
	}

	if (binds.pressed("3d_movedown"))
	{
		camera.position.z -= speed;
		camera.view.z -= speed;
	}

	if (binds.pressed("3d_toggle_fullbright"))
	{
		binds.clear(_T("3d_toggle_fullbright"));
		render_fullbright = !render_fullbright;
	}

	if (binds.pressed("3d_toggle_fog"))
	{
		binds.clear(_T("3d_toggle_fog"));
		render_fog = !render_fog;
	}

	if (binds.pressed("3d_toggle_sky"))
	{
		binds.clear(_T("3d_toggle_sky"));
		render_sky = !render_sky;
	}
	
	/*
	if (binds.pressed("3d_toggle_things"))
	{
		binds.clear("3d_toggle_things");
		render_things = render_things + 1;

		if (render_things > 3)
			render_things = 0;
	}
	*/

	if (binds.pressed("3d_toggle_gravity"))
	{
		binds.clear(_T("3d_toggle_gravity"));
		camera.gravity = !camera.gravity;

		if (camera.gravity)
			add_3d_message(_T("Gravity ON"));
		else
			add_3d_message(_T("Gravity OFF"));
	}

	// Sector height quick changes (8 units)
	if (binds.pressed("3d_upfloor8") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downfloor8") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(-8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upceil8") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(8, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downceil8") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(-8, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upboth8") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(8, true, true);
		change_sector_height_3d(8, false, true);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downboth8") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(-8, true, true);
		change_sector_height_3d(-8, false, true);
		key_3d_rep = key_delay_3d;
	}

	// Sector height quick changes (1 unit)
	if (binds.pressed("3d_upfloor") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downfloor") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(-1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upceil") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(1, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downceil") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(-1, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upboth") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(1, true, true);
		change_sector_height_3d(1, false, true);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downboth") && (key_3d_allow || mwheel))
	{
		change_sector_height_3d(-1, true, true);
		change_sector_height_3d(-1, false, true);
		key_3d_rep = key_delay_3d;
	}

	// Texture offset
	if (binds.pressed("3d_upyoffset") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(0, 1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downyoffset") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(0, -1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upxoffset") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(1, 0);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downxoffset") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(-1, 0);
		key_3d_rep = key_delay_3d;
	}

	// Texture offset x8
	if (binds.pressed("3d_upyoffset8") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(0, 8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downyoffset8") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(0, -8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upxoffset8") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(8, 0);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downxoffset8") && (key_3d_allow || mwheel))
	{
		change_offsets_3d(-8, 0);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_toggle_hilight"))
	{
		binds.clear(_T("3d_toggle_hilight"));
		render_hilight = !render_hilight;
	}

	// Upper/Lower unpegged toggle
	if (binds.pressed("3d_upperunpegged"))
	{
		binds.clear(_T("3d_upperunpegged"));
		toggle_texture_peg_3d(true);
	}

	if (binds.pressed("3d_lowerunpegged"))
	{
		binds.clear(_T("3d_lowerunpegged"));
		toggle_texture_peg_3d(false);
	}

	// Change light level
	if (binds.pressed("3d_uplightlevel"))
	{
		binds.clear(_T("3d_uplightlevel"));
		change_light_3d(16);
	}

	if (binds.pressed("3d_downlightlevel"))
	{
		binds.clear(_T("3d_downlightlevel"));
		change_light_3d(-16);
	}

	if (binds.pressed("3d_uplightlevel1"))
	{
		binds.clear(_T("3d_uplightlevel1"));
		change_light_3d(1);
	}

	if (binds.pressed("3d_downlightlevel1"))
	{
		binds.clear(_T("3d_downlightlevel1"));
		change_light_3d(-1);
	}

	/*
	// Change thing angle
	if (binds.pressed("3d_upthingangle") && (key_3d_allow || mwheel))
	{
		change_thing_angle_3d(45);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downthingangle") && (key_3d_allow || mwheel))
	{
		change_thing_angle_3d(-45);
		key_3d_rep = key_delay_3d;
	}

	// Change thing z height
	if (map.hexen)
	{
		if (binds.pressed("3d_upthingz8") && (key_3d_allow || mwheel))
		{
			change_thing_z_3d(8);
			key_3d_rep = key_delay_3d;
		}

		if (binds.pressed("3d_downthingz8") && (key_3d_allow || mwheel))
		{
			change_thing_z_3d(-8);
			key_3d_rep = key_delay_3d;
		}

		if (binds.pressed("3d_upthingz") && (key_3d_allow || mwheel))
		{
			change_thing_z_3d(1);
			key_3d_rep = key_delay_3d;
		}

		if (binds.pressed("3d_downthingz") && (key_3d_allow || mwheel))
		{
			change_thing_z_3d(-1);
			key_3d_rep = key_delay_3d;
		}
	}
	*/

	if (binds.pressed(_T("3d_align_tex_x")))
	{
		binds.clear(_T("3d_align_tex_x"));
		auto_align_x_3d();
	}

	// Reset offsets
	if (binds.pressed("3d_reset_offsets"))
	{
		binds.clear(_T("3d_reset_offsets"));
		reset_offsets_3d();
	}

	if (binds.pressed("3d_reset_xoffset"))
	{
		binds.clear(_T("3d_reset_xoffset"));
		reset_offsets_3d(true, false);
	}

	if (binds.pressed("3d_reset_yoffset"))
	{
		binds.clear(_T("3d_reset_yoffset"));
		reset_offsets_3d(false, true);
	}

	// Change (browse) texture
	if (binds.pressed("3d_change_texture"))
	{
		binds.clear(_T("3d_change_texture"));
		change_texture_3d(false);
	}

	if (binds.pressed("3d_change_tex_paint"))
	{
		binds.clear(_T("3d_change_tex_paint"));
		change_texture_3d(true);
	}

	// Copy/Paste texture
	if (binds.pressed("3d_copy_texture"))
	{
		binds.clear(_T("3d_copy_texture"));
		copy_texture_3d();
	}

	if (binds.pressed("3d_paste_texture"))
	{
		binds.clear(_T("3d_paste_texture"));
		paste_texture_3d(false);
	}

	if (binds.pressed("3d_paste_paint"))
	{
		binds.clear(_T("3d_paste_paint"));
		paste_texture_3d(true);
	}

	// Copy/paste side properties
	if (binds.pressed("3d_copy_side"))
	{
		binds.clear(_T("3d_copy_side"));
		copy_side_3d(true, true, true);
	}

	if (binds.pressed("3d_paste_side"))
	{
		binds.clear(_T("3d_paste_side"));
		paste_side_3d(true, true, true);
	}

	// Copy/paste offsets
	if (binds.pressed("3d_copy_offsets"))
	{
		binds.clear(_T("3d_copy_offsets"));
		copy_side_3d(true, true);
	}

	if (binds.pressed("3d_paste_offsets"))
	{
		binds.clear(_T("3d_paste_offsets"));
		paste_side_3d(true, true);
	}

	if (binds.pressed("3d_copy_xoffset"))
	{
		binds.clear(_T("3d_copy_xoffset"));
		copy_side_3d(true, false);
	}

	if (binds.pressed("3d_paste_xoffset"))
	{
		binds.clear(_T("3d_paste_xoffset"));
		paste_side_3d(true, false);
	}

	if (binds.pressed("3d_copy_yoffset"))
	{
		binds.clear(_T("3d_copy_yoffset"));
		copy_side_3d(false, true);
	}

	if (binds.pressed("3d_paste_yoffset"))
	{
		binds.clear(_T("3d_paste_yoffset"));
		paste_side_3d(false, true);
	}

	if (mult < 1.0f)
		key_3d_rep--;
	else
		key_3d_rep -= mult;

	if (key_3d_rep < 0)
		key_3d_rep = 0;

	return true;
}
