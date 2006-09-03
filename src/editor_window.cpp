
#include "main.h"
#include "editor_window.h"
#include "doom_map.h"
#include "dm_thing.h"
#include "game_config.h"
#include "wad_manager.h"
#include "info_bar.h"
#include "prefs_dialog.h"
#include "3d_window.h"
#include "console_window.h"
//#include "script_editor.h"
#include "tex_browser.h"
#include "edit_misc.h"
//#include "checks.h"
#include "input.h"
#include "ttype_select.h"
#include "edit.h"
//#include "sidebar.h"

#include <wx/image.h>
#include <wx/filename.h>
#include <wx/numdlg.h>

Wad	*edit_wad = NULL;
EditorWindow *editor_window = NULL;
//ScriptEditor *script_editor = NULL;

CVAR(Int, show_sidebar, 0, CVAR_SAVE)
CVAR(String, path_doomexe, "", CVAR_SAVE)
CVAR(String, run_parameters, "-file %a", CVAR_SAVE)

/*
extern int edit_mode, gridsize, vid_width, vid_height;
extern int hilight_item;
extern float zoom, xoff, yoff;
extern point2_t mouse, down_pos;

extern bool lock_hilight;
extern vector<string> valid_map_names;
extern string bg_tex, def_doortex, app_path;
extern thing_t last_thing;
*/

extern DoomMap d_map;
extern GameConfig game;
extern WadManager *wad_manager;
extern WadList wads;
extern ConsoleWindow *console_window;
extern int edit_mode, gridsize;
extern Wad reswad;
extern bool lock_hilight;
extern Thing last_thing;

EXTERN_CVAR(Bool, edit_snap_grid)


EditorWindow::EditorWindow(const wxChar *title, int xpos, int ypos, int width, int height)
    : wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height))
{
	// Setup menu
	setup_menu();

	// Setup toolbar
	setup_toolbar();

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 1, wxEXPAND);

	// Map area
	map_area = new MapCanvas(this);
	hbox->Add(map_area, 1, wxEXPAND);
	map_area->set_menus(line_context_menu, sector_context_menu, thing_context_menu);

	/*
	// Sidebar
	sb_draw = new DrawSideBar(this);
	hbox->Add(sb_draw, 0, wxEXPAND);

	sb_theme = new ThemeSideBar(this);
	hbox->Add(sb_theme, 0, wxEXPAND);
	*/

	// Info bar
	info_bar = new LineInfoBar(this);
	vbox->Add(info_bar, 0, wxEXPAND);

	CreateStatusBar(4);
	int widths[] = { -1, 80, 80, 96 };
	SetStatusWidths(4, widths);

	SetSizeHints(800, 600, -1, -1);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));
	Maximize(true);
	Show();

	/*
	if (show_sidebar != 1)
		sb_draw->Show(false);
	if (show_sidebar != 2)
		sb_theme->Show(false);
		*/

	Layout();
}

EditorWindow::~EditorWindow()
{
	remove("sladetemp");
	remove("sladetemp.bak");
}

wxImage get_image_from_pk3(string entry_name, int type)
{
	Lump* lump = reswad.getLump(entry_name, 0, true, true);
	wxImage image;

	if (lump)
	{
		string path = c_path(_T("sladetemp"), DIR_TMP);
		lump->dumpToFile(path);
		image.LoadFile(path, type);
		remove(chr(path));
	}

	return image;
}

void EditorWindow::setup_toolbar()
{
	CreateToolBar();
	wxToolBar *tb = GetToolBar();

	tb->SetToolBitmapSize(wxSize(16, 17));

	tb->AddTool(EWMENU_FILE_WADMANAGER, _T("Wad Manager"), wxBitmap(get_image_from_pk3("res/tbar/tb_temp16.png", wxBITMAP_TYPE_PNG)), _T("Wad Manager"));
	tb->AddSeparator();
	tb->AddTool(EWMENU_FILE_SAVE, _T("Save Map"), wxBitmap(get_image_from_pk3("res/tbar/tb_save16.png", wxBITMAP_TYPE_PNG)), _T("Save Map"));
	tb->AddTool(EWMENU_FILE_SAVEAS, _T("Save Map As"), wxBitmap(get_image_from_pk3("res/tbar/tb_saveas16.png", wxBITMAP_TYPE_PNG)), _T("Save Map As"));
	tb->AddTool(EWMENU_FILE_RUN, _T("Run Map"), wxBitmap(get_image_from_pk3("res/tbar/tb_run16.png", wxBITMAP_TYPE_PNG)), _T("Run Map"));
	tb->AddTool(EWMENU_FILE_CLOSE, _T("Close"), wxBitmap(get_image_from_pk3("res/tbar/tb_close16.png", wxBITMAP_TYPE_PNG)), _T("Close Map"));
	tb->AddSeparator();
	tb->AddRadioTool(EWMENU_MODE_VERTS, _T("Vertices Mode"), wxBitmap(get_image_from_pk3("res/tbar/tb_verts16.png", wxBITMAP_TYPE_PNG)),
						wxBitmap(get_image_from_pk3("res/tbar/tb_vertsd16.png", wxBITMAP_TYPE_PNG)), _T("Vertices Mode"));
	tb->AddRadioTool(EWMENU_MODE_LINES, _T("Lines Mode"), wxBitmap(get_image_from_pk3("res/tbar/tb_lines16.png", wxBITMAP_TYPE_PNG)),
						wxBitmap(get_image_from_pk3("res/tbar/tb_linesd16.png", wxBITMAP_TYPE_PNG)), _T("Lines Mode"));
	tb->AddRadioTool(EWMENU_MODE_SECTORS, _T("Sectors Mode"), wxBitmap(get_image_from_pk3("res/tbar/tb_sectors16.png", wxBITMAP_TYPE_PNG)),
						wxBitmap(get_image_from_pk3("res/tbar/tb_sectorsd16.png", wxBITMAP_TYPE_PNG)), _T("Sectors Mode"));
	tb->AddRadioTool(EWMENU_MODE_THINGS, _T("Things Mode"), wxBitmap(get_image_from_pk3("res/tbar/tb_things16.png", wxBITMAP_TYPE_PNG)),
						wxBitmap(get_image_from_pk3("res/tbar/tb_thingsd16.png", wxBITMAP_TYPE_PNG)), _T("Things Mode"));
	tb->AddSeparator();
	tb->AddTool(EWMENU_MODE_3D, _T("3D Mode"), wxBitmap(get_image_from_pk3("res/tbar/tb_3d16.png", wxBITMAP_TYPE_PNG)), _T("3d Mode"));
	tb->AddSeparator();
	tb->AddTool(EWMENU_EDIT_MIRRORX, _T("Mirror X"), wxBitmap(get_image_from_pk3("res/tbar/tb_mirrorx16.png", wxBITMAP_TYPE_PNG)), _T("Mirror X"));
	tb->AddTool(EWMENU_EDIT_MIRRORY, _T("Mirror Y"), wxBitmap(get_image_from_pk3("res/tbar/tb_mirrory16.png", wxBITMAP_TYPE_PNG)), _T("Mirror Y"));
	tb->AddTool(EWMENU_EDIT_ROTATE, _T("Rotate"), wxBitmap(get_image_from_pk3("res/tbar/tb_rotate16.png", wxBITMAP_TYPE_PNG)), _T("Rotate"));

	tb->ToggleTool(EWMENU_MODE_LINES, true);
	tb->Realize();
}

void EditorWindow::setup_menu()
{
	wxMenuBar *menu_bar = new wxMenuBar();
	SetMenuBar(menu_bar);

	// File menu
	wxMenu *file_menu = new wxMenu();
	file_menu->Append(EWMENU_FILE_WADMANAGER, _T("&Wad Manager"), _T("Open the Wad Manager window"));
	file_menu->AppendSeparator();
	file_menu->Append(EWMENU_FILE_RUN, _T("&Run Map"), _T("Run the current wads/map with a doom executable"));
	file_menu->Append(EWMENU_FILE_RENAME, _T("&Rename Map"), _T("Rename the current map"));
	file_menu->Append(EWMENU_FILE_IMPORT, _T("&Import Map"), _T("Import a map from another wad file"));
	file_menu->AppendSeparator();
	file_menu->Append(EWMENU_FILE_SAVE, _T("&Save"), _T("Save the current map"));
	file_menu->Append(EWMENU_FILE_SAVEAS, _T("Save &As"), _T("Save the current wad to a new file"));
	file_menu->Append(EWMENU_FILE_SAVESTANDALONE, _T("Save Standalone &Map"), _T("Save the current map to a new wad file"));
	file_menu->Append(EWMENU_FILE_CLOSE, _T("&Close"), _T("Close the current map"));
	file_menu->AppendSeparator();
	file_menu->Append(EWMENU_FILE_EXIT, _T("E&xit"), _T("Exit SLADE"));
	menu_bar->Append(file_menu, _T("&File"));

	// Edit menu
	wxMenu *edit_menu = new wxMenu();

	// Line edit
	wxMenu *line_edit_menu = new wxMenu();
	line_edit_menu->Append(EWMENU_EDIT_ALIGNX, _T("Align &X"), _T("Align selected lines' textures along the x axis"));
	line_edit_menu->Append(EWMENU_EDIT_CORRECTREFS, _T("Correct Sector &References"), _T("Attempts to set the correct sector references for the line"));
	line_edit_menu->Append(EWMENU_EDIT_SPLITLINE, _T("&Split Line"), _T("Split selected line(s)"));
	line_edit_menu->Append(EWMENU_EDIT_EXTRUDELINE, _T("&Extrude Line"), _T("'Extrudes' selected lines"));
	edit_menu->Insert(0, EWMENU_EDIT_LINE, _T("&Line"), line_edit_menu);

	// Line context
	line_context_menu = new wxMenu();
	line_context_menu->Append(EWMENU_EDIT_PROPERTIES, _T("&Properties"), _T("Edit the line's properties"));
	line_context_menu->AppendSeparator();
	line_context_menu->Append(EWMENU_EDIT_MIRRORX, _T("Mirror &X"), _T("Mirror selected lines on the X axis"));
	line_context_menu->Append(EWMENU_EDIT_MIRRORY, _T("Mirror &Y"), _T("Mirror selected lines on the Y axis"));
	line_context_menu->Append(EWMENU_EDIT_ROTATE, _T("&Rotate"), _T("Rotate selected lines"));
	line_context_menu->AppendSeparator();
	line_context_menu->Append(EWMENU_EDIT_ALIGNX, _T("Align &X"), _T("Align selected lines' textures along the x axis"));
	line_context_menu->Append(EWMENU_EDIT_CORRECTREFS, _T("Correct Sector &References"), _T("Attempts to set the correct sector references for the line"));
	line_context_menu->Append(EWMENU_EDIT_SPLITLINE, _T("&Split Line"), _T("Split selected line(s)"));
	line_context_menu->Append(EWMENU_LINE_SPLITAT, _T("Split &Here"), _T("Split hilighted line near mouse point"));
	line_context_menu->Append(EWMENU_EDIT_EXTRUDELINE, _T("&Extrude Line"), _T("'Extrudes' selected lines"));
	line_context_menu->Append(EWMENU_EDIT_FLIPLINE, _T("&Flip Line"), _T("Flips the selected lines"));

	// Sector edit
	wxMenu *sector_edit_menu = new wxMenu();
	sector_edit_menu->Append(EWMENU_EDIT_MERGESECTORS, _T("&Merge Sectors"), _T("Merge selected sectors (don't delete any lines)"));
	sector_edit_menu->Append(EWMENU_EDIT_JOINSECTORS, _T("&Join Sectors"), _T("Join selected sectors (delete redundant lines)"));
	sector_edit_menu->AppendSeparator();
	sector_edit_menu->Append(EWMENU_EDIT_CREATEDOOR, _T("Create &Door"), _T("Create a door from the selected sector(s)"));
	sector_edit_menu->Append(EWMENU_EDIT_CREATESTAIRS, _T("Create &Stairs"), _T("Create stairs from the selected sectors"));
	sector_edit_menu->Append(EWMENU_EDIT_LIGHTGRADIENT, _T("&Light Gradient"), _T("Create a smooth light gradient from the first selected sector to the last"));
	edit_menu->Insert(1, EWMENU_EDIT_SECTOR, _T("&Sector"), sector_edit_menu);

	// Sector context
	sector_context_menu = new wxMenu();
	sector_context_menu->Append(EWMENU_EDIT_PROPERTIES, _T("&Properties"), _T("Edit the sector's properties"));
	sector_context_menu->AppendSeparator();
	sector_context_menu->Append(EWMENU_EDIT_MIRRORX, _T("Mirror &X"), _T("Mirror selected sectors on the X axis"));
	sector_context_menu->Append(EWMENU_EDIT_MIRRORY, _T("Mirror &Y"), _T("Mirror selected sectors on the Y axis"));
	sector_context_menu->Append(EWMENU_EDIT_ROTATE, _T("&Rotate"), _T("Rotate selected sectors"));
	sector_context_menu->AppendSeparator();
	sector_context_menu->Append(EWMENU_EDIT_CREATESECTOR, _T("&Create Sector"), _T("Makes the area around the mouse pointer into a sector"));
	sector_context_menu->Append(EWMENU_EDIT_MERGESECTORS, _T("&Merge Sectors"), _T("Merge selected sectors (don't delete any lines)"));
	sector_context_menu->Append(EWMENU_EDIT_JOINSECTORS, _T("&Join Sectors"), _T("Join selected sectors (delete redundant lines)"));
	sector_context_menu->AppendSeparator();
	sector_context_menu->Append(EWMENU_EDIT_CREATEDOOR, _T("Create &Door"), _T("Create a door from the selected sector(s)"));
	sector_context_menu->Append(EWMENU_EDIT_CREATESTAIRS, _T("Create &Stairs"), _T("Create stairs from the selected sectors"));
	sector_context_menu->Append(EWMENU_EDIT_LIGHTGRADIENT, _T("&Light Gradient"), _T("Create a smooth light gradient from the first selected sector to the last"));

	// Thing context
	thing_context_menu = new wxMenu();
	thing_context_menu->Append(EWMENU_EDIT_PROPERTIES, _T("&Properties"), _T("Edit the thing's properties"));
	thing_context_menu->Append(EWMENU_THING_INSERT, _T("&Insert Thing"), _T("Insert a thing at the mouse position"));
	thing_context_menu->AppendSeparator();
	thing_context_menu->Append(EWMENU_EDIT_MIRRORX, _T("Mirror &X"), _T("Mirror selected things on the X axis"));
	thing_context_menu->Append(EWMENU_EDIT_MIRRORY, _T("Mirror &Y"), _T("Mirror selected things on the Y axis"));
	thing_context_menu->Append(EWMENU_EDIT_ROTATE, _T("Rotate"), _T("Rotate selected things"));

	edit_menu->AppendSeparator();
	edit_menu->Append(EWMENU_EDIT_SCRIPTS, _T("&ACS Script Editor"), _T("Open the ACS script editor"));
	edit_menu->AppendSeparator();
	edit_menu->Append(EWMENU_EDIT_MIRRORX, _T("Mirror &X"), _T("Mirror selected items on the X axis"));
	edit_menu->Append(EWMENU_EDIT_MIRRORY, _T("Mirror &Y"), _T("Mirror selected items on the Y axis"));
	edit_menu->Append(EWMENU_EDIT_ROTATE, _T("&Rotate"), _T("Rotates selected items"));
	edit_menu->AppendSeparator();
	edit_menu->Append(EWMENU_EDIT_PREFS, _T("&Preferences"), _T("Setup SLADE options"));
	menu_bar->Append(edit_menu, _T("&Edit"));

	// Mode menu
	wxMenu *mode_menu = new wxMenu();
	mode_menu->Append(EWMENU_MODE_VERTS, _T("&Vertices"), _T("Vertex edit mode"));
	mode_menu->Append(EWMENU_MODE_LINES, _T("&Lines"), _T("Line edit mode"));
	mode_menu->Append(EWMENU_MODE_SECTORS, _T("&Sectors"), _T("Sector edit mode"));
	mode_menu->Append(EWMENU_MODE_THINGS, _T("&Things"), _T("Thing edit mode"));
	mode_menu->Append(EWMENU_MODE_3D, _T("&3d Mode"), _T("3d Mode"));
	menu_bar->Append(mode_menu, _T("&Mode"));

	// View menu
	wxMenu *view_menu = new wxMenu();

	wxMenu *view_sb_menu = new wxMenu();
	view_sb_menu->Append(EWMENU_VIEW_DRAWSIDEBAR, _T("&Drawing"), _T("Shows the drawing options sidebar"));
	view_sb_menu->Append(EWMENU_VIEW_THEMESIDEBAR, _T("&Theme"), _T("Shows the theme options sidebar"));

	view_menu->Insert(0, -1, _T("&Show Sidebar"), view_sb_menu);
	view_menu->Append(EWMENU_VIEW_HIDESIDEBAR, _T("Hide &Sidebar"), _T("Hides the SLADE sidebar"));
	view_menu->AppendSeparator();
	view_menu->Append(EWMENU_VIEW_SETBGTEX, _T("Set &Background Texture"), _T("Sets the background texture for the 2d edit view"));
	view_menu->Append(EWMENU_VIEW_CLEARBGTEX, _T("Clear Background &Texture"), _T("Clears the background texture"));
	view_menu->AppendSeparator();
	view_menu->Append(EWMENU_VIEW_CONSOLE, _T("Show &Console"), _T("Open the SLADE console window"));
	menu_bar->Append(view_menu, _T("&View"));

	// Checks menu
	wxMenu *checks_menu = new wxMenu();
	checks_menu->Append(EWMENU_CHECK_REMOVEVERTS, _T("Remove Free &Vertices"), _T("Removes any vertices that aren't attached to a line"));
	checks_menu->Append(EWMENU_CHECK_REMOVELINES, _T("Remove Zero-Length &Lines"), _T("Removes any lines that have the same start and end vertex"));
	checks_menu->Append(EWMENU_CHECK_REMOVESECTORS, _T("Remove Unused &Sectors"), _T("Removes any sectors that aren't referenced by any sidedefs"));
	checks_menu->Append(EWMENU_CHECK_REMOVESIDES, _T("Remove Unused Side&defs"), _T("Removes any sidedefs that aren't referenced by any lines"));
	checks_menu->Append(EWMENU_CHECK_TEXTURES, _T("Check &Textures"), _T("Checks for any invalid textures"));
	menu_bar->Append(checks_menu, _T("&Checks"));
}

// open_map: Opens a map to edit, if wad is NULL then it's a new/standalone map
// ------------------------------------------------------------------------- >>
bool EditorWindow::open_map(Wad* wad, string mapname)
{
	if (wad)
	{
		if (wad->zip)
		{
			// Get map lump
			Lump* mlump = wad->getLump(_T("maps/") + mapname, 0, true, false);

			// If it doesn't exist create it
			bool exists = !!mlump;
			if (!mlump)
			{
				mlump = wad->addLump(mapname + _T(".wad"), wad->numLumps());
				mlump->addDir(_T("maps"));
			}

			Wad *nwad = new Wad();
			nwad->parent = mlump;

			if (exists)
				d_map.open(nwad, mapname);
			else
				d_map.create(mapname);

			wxFileName fn(wad->path);
			nwad->path = fn.GetFullName() + _T("/maps/") + mapname;
			nwad->deleteAllLumps();
			wad = nwad;
		}
		else
		{
			if (!wad->getLump(mapname, 0))
				d_map.create(mapname);
			else
			{
				if (!d_map.open(wad, mapname))
					return false;
			}
		}

		SetTitle(s_fmt(_T("SLADE (%s, %s)"), chr(wad->path), chr(mapname)));
	}
	else
	{
		d_map.create(mapname);
		SetTitle(s_fmt(_T("SLADE (unsaved, %s)"), chr(mapname)));
	}

	// Reset zoom and offset (0,0 and fully zoomed in)
	map_area->zoom_view(100, false);
	map_area->set_offsets(d_map.middle(), false);

	if (d_map.n_verts() == 0)
	{
		map_area->set_offsets(point2_t(0, 0), false);
		map_area->zoom_view(0.2);
	}
	else
	{
		// Zoom out until entire map is visible
		bool done = false;
		rect_t bounds = d_map.bounds();
		while (!done)
		{
			rect_t border = map_area->get_border();
			done = true;

			if (bounds.x1() < border.x1() ||
				bounds.y1() < border.y1() ||
				bounds.x2() > border.x2() ||
				bounds.y2() > border.y2())
			{
				if (!map_area->zoom_view(0.8, false))
					done = false;
			}
		}
	}

	map_area->redraw();
	edit_wad = wad;
	return true;
}

void EditorWindow::change_infobar()
{
	GetSizer()->Detach(info_bar);
	delete info_bar;

	if (edit_mode == 0)
	{
		info_bar = new VertexInfoBar(this);
		GetToolBar()->ToggleTool(EWMENU_MODE_VERTS, true);
	}
	else if (edit_mode == 1)
	{
		info_bar = new LineInfoBar(this);
		GetToolBar()->ToggleTool(EWMENU_MODE_LINES, true);
	}
	else if (edit_mode == 2)
	{
		info_bar = new SectorInfoBar(this);
		GetToolBar()->ToggleTool(EWMENU_MODE_SECTORS, true);
	}
	else if (edit_mode == 3)
	{
		info_bar = new ThingInfoBar(this);
		GetToolBar()->ToggleTool(EWMENU_MODE_THINGS, true);
	}
	else
		info_bar = new InfoBar(this);

	GetSizer()->Add(info_bar, 0, wxEXPAND);

	Layout();
}

void EditorWindow::update_statusbar()
{
	if (d_map.opened())
		SetStatusText(wxString::Format(_T("(%d, %d)"), map_area->mouse_pos(true).x, map_area->mouse_pos(true).y), 3);

	if (edit_snap_grid)
		SetStatusText(wxString::Format(_T("Snap: ON")), 2);
	else
		SetStatusText(wxString::Format(_T("Snap: OFF")), 2);

	SetStatusText(wxString::Format(_T("Grid: %d"), gridsize), 1);
}

void EditorWindow::draw_shape()
{
	//((DrawSideBar*)sb_draw)->draw_shape();
	ldraw_drawrect(map_area->mouse_pos(true), false);
}

BEGIN_EVENT_TABLE(EditorWindow, wxFrame)
	EVT_CLOSE(EditorWindow::close)

	// MENU EVENTS

	// File menu
	EVT_MENU(EWMENU_FILE_WADMANAGER,		EditorWindow::file_wadmanager)
	EVT_MENU(EWMENU_FILE_CLOSE,				EditorWindow::file_close)
	EVT_MENU(EWMENU_FILE_SAVE,				EditorWindow::file_save)
	EVT_MENU(EWMENU_FILE_SAVEAS,			EditorWindow::file_saveas)
	EVT_MENU(EWMENU_FILE_SAVESTANDALONE,	EditorWindow::file_savestandalone)
	EVT_MENU(EWMENU_FILE_RENAME,			EditorWindow::file_rename)
	EVT_MENU(EWMENU_FILE_RUN,				EditorWindow::file_run)
	EVT_MENU(EWMENU_FILE_IMPORT,			EditorWindow::file_importmap)
	EVT_MENU(EWMENU_FILE_EXIT,				EditorWindow::file_exit)

	// Edit menu
	EVT_MENU(EWMENU_EDIT_SCRIPTS,		EditorWindow::edit_scripts)
	EVT_MENU(EWMENU_EDIT_PREFS,			EditorWindow::edit_prefs)
	EVT_MENU(EWMENU_EDIT_PROPERTIES,	EditorWindow::edit_properties)
	EVT_MENU(EWMENU_EDIT_MIRRORX,		EditorWindow::edit_mirrorx)
	EVT_MENU(EWMENU_EDIT_MIRRORY,		EditorWindow::edit_mirrory)
	EVT_MENU(EWMENU_EDIT_ROTATE,		EditorWindow::edit_rotate)

	EVT_MENU(EWMENU_EDIT_ALIGNX, EditorWindow::edit_alignx)
	EVT_MENU(EWMENU_EDIT_CORRECTREFS, EditorWindow::edit_correctrefs)
	EVT_MENU(EWMENU_EDIT_SPLITLINE, EditorWindow::edit_splitline)
	EVT_MENU(EWMENU_EDIT_EXTRUDELINE, EditorWindow::edit_extrudeline)
	EVT_MENU(EWMENU_EDIT_FLIPLINE, EditorWindow::edit_flipline)

	EVT_MENU(EWMENU_EDIT_CREATESECTOR, EditorWindow::edit_createsector)
	EVT_MENU(EWMENU_EDIT_MERGESECTORS, EditorWindow::edit_mergesectors)
	EVT_MENU(EWMENU_EDIT_JOINSECTORS, EditorWindow::edit_joinsectors)
	EVT_MENU(EWMENU_EDIT_CREATEDOOR, EditorWindow::edit_createdoor)
	EVT_MENU(EWMENU_EDIT_CREATESTAIRS, EditorWindow::edit_createstairs)
	EVT_MENU(EWMENU_EDIT_LIGHTGRADIENT, EditorWindow::edit_lightgradient)

	// Mode menu
	EVT_MENU(EWMENU_MODE_VERTS,		EditorWindow::mode_vertices)
	EVT_MENU(EWMENU_MODE_LINES,		EditorWindow::mode_lines)
	EVT_MENU(EWMENU_MODE_SECTORS,	EditorWindow::mode_sectors)
	EVT_MENU(EWMENU_MODE_THINGS,	EditorWindow::mode_things)
	EVT_MENU(EWMENU_MODE_3D,		EditorWindow::mode_3d)

	// View menu
	EVT_MENU(EWMENU_VIEW_DRAWSIDEBAR,	EditorWindow::view_drawsidebar)
	EVT_MENU(EWMENU_VIEW_THEMESIDEBAR,	EditorWindow::view_themesidebar)
	EVT_MENU(EWMENU_VIEW_HIDESIDEBAR,	EditorWindow::view_hidesidebar)
	EVT_MENU(EWMENU_VIEW_CONSOLE,		EditorWindow::view_console)
	EVT_MENU(EWMENU_VIEW_SETBGTEX,		EditorWindow::view_setbgtex)
	EVT_MENU(EWMENU_VIEW_CLEARBGTEX,	EditorWindow::view_clearbgtex)

	// Checks menu
	EVT_MENU(EWMENU_CHECK_REMOVEVERTS,		EditorWindow::check_removeverts)
	EVT_MENU(EWMENU_CHECK_REMOVELINES,		EditorWindow::check_removelines)
	EVT_MENU(EWMENU_CHECK_REMOVESECTORS,	EditorWindow::check_removesectors)
	EVT_MENU(EWMENU_CHECK_REMOVESIDES,		EditorWindow::check_removesides)
	EVT_MENU(EWMENU_CHECK_TEXTURES,			EditorWindow::check_checktextures)

	// Thing context menu
	EVT_MENU(EWMENU_THING_INSERT,	EditorWindow::thing_insert)

	// Line context menu
	EVT_MENU(EWMENU_LINE_SPLITAT,	EditorWindow::line_splitat)
END_EVENT_TABLE()

void EditorWindow::close(wxCloseEvent &event)
{
	if (!event.CanVeto())
		wxTheApp->ExitMainLoop();
	else
	{
		// Exit stuff
		if (d_map.check_changed(MC_SAVE_NEEDED))
		{
			int result = wxMessageBox(_T("Save changes?"), _T("SLADE"), wxYES_NO|wxCANCEL|wxICON_QUESTION);

			if (result == wxYES)
			{
				wxCommandEvent event;
				file_save(event);
			}
			else if (result == wxCANCEL)
			{
				return;
			}
		}

		save_main_config();
		wxTheApp->ExitMainLoop();
	}
}

void EditorWindow::file_wadmanager(wxCommandEvent &event)
{
	wad_manager->Show();
}

void EditorWindow::file_close(wxCommandEvent &event)
{
	if (d_map.check_changed(MC_SAVE_NEEDED))
	{
		int result = wxMessageBox(_T("Save changes?"), _T("SLADE"), wxYES_NO|wxCANCEL|wxICON_QUESTION);

		if (result == wxYES)
		{
			wxCommandEvent event;
			file_save(event);
		}
		else if (result == wxCANCEL)
		{
			return;
		}
	}

	d_map.close();
	map_area->redraw();
	SetTitle(_T("SLADE"));

	//delete script_editor;
	//script_editor = NULL;
}

void EditorWindow::file_exit(wxCommandEvent &event)
{
	Close();
}

void EditorWindow::file_save(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	if (edit_wad)
	{
		if (!edit_wad->locked)
		{
			d_map.add_to_wad(edit_wad);
			edit_wad->save(true);
			d_map.clear_change(MC_SAVE_NEEDED);
			SetTitle(wxString::Format(_T("SLADE (%s, %s)"), edit_wad->path.c_str(), d_map.map_name().c_str()));

			if (!edit_wad->parent)
				add_recent_wad(edit_wad->path);
		}
	}
	else
		file_saveas(event);
}

void EditorWindow::file_saveas(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	string filename = wx_to_str(wxFileSelector(_T("Save Wadfile"), _T(""), _T(""), _T("*.wad"), _T("Doom Wad Files (*.wad)|*.wad"), wxSAVE|wxOVERWRITE_PROMPT));

	if (filename != "")
	{
		if (!edit_wad)
			edit_wad = new Wad();

		edit_wad->path = filename;
		d_map.add_to_wad(edit_wad);
		edit_wad->save(true);
		add_recent_wad(filename);
		wads.open(edit_wad->path);

		d_map.clear_change(MC_SAVE_NEEDED);
		SetTitle(wxString::Format(_T("SLADE (%s, %s)"), edit_wad->path.c_str(), d_map.map_name().c_str()));
	}
}

void EditorWindow::file_savestandalone(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	string filename = wx_to_str(wxFileSelector(_T("Save Standalone Map"), _T(""), _T(""), _T("*.wad"), _T("Doom Wad Files (*.wad)|*.wad"), wxSAVE|wxOVERWRITE_PROMPT));

	if (filename != "")
	{
		edit_wad = new Wad();
		edit_wad->path = filename;
		d_map.add_to_wad(edit_wad);
		edit_wad->save(true);
		add_recent_wad(filename);

		d_map.clear_change(MC_SAVE_NEEDED);
		SetTitle(wxString::Format(_T("SLADE (%s, %s)"), edit_wad->path.c_str(), d_map.map_name().c_str()));
	}
}

void EditorWindow::file_rename(wxCommandEvent &event)
{
	if (edit_wad)
	{
		if (edit_wad->locked)
			return;
	}

	string mapname = game.ask_map_name();

	if (mapname != "")
	{
		if (edit_wad)
		{
			if (vector_exists(edit_wad->available_maps, mapname))
			{
				wxMessageBox(_T("Map already exists in the wad!"), _T("Error"), wxICON_ERROR);
				return;
			}
		}

		d_map.set_name(mapname);

		if (edit_wad)
			SetTitle(wxString::Format(_T("SLADE (%s, %s)"), edit_wad->path.c_str(), d_map.map_name().c_str()));
		else
			SetTitle(wxString::Format(_T("SLADE (unsaved, %s)"), d_map.map_name().c_str()));
	}
}

void EditorWindow::file_run(wxCommandEvent &event)
{
	/*
	string exe_path = path_doomexe;
	if (exe_path == "")
	{
		wxMessageBox(_T("No executable set. See edit->preferences->misc"));
		return;
	}
	
	Tokenizer tz;
	tz.open_string(run_parameters, 0, 0);

	Wad *temp_wad = new Wad();
	map.add_to_wad(temp_wad);
	temp_wad->path = parse_string("%ssladetemp.wad", app_path.c_str());
	temp_wad->save(true);
	wads.open_wad(temp_wad->path);

	string token = tz.get_token();
	while (token != "!END")
	{
		exe_path += " ";

		if (token == "%w")
		{
			if (edit_wad)
				exe_path += parse_string("\"%s\" ", edit_wad->path.c_str());
		}

		else if (token == "%a")
		{
			for (int a = 0; a < wads.n_wads; a++)
				exe_path += parse_string("\"%s\" ", wads.get_wad(a)->path.c_str());
		}

		else if (token == "%m")
			exe_path += map.name;

		else if (token == "%i")
			exe_path += parse_string("\"%s\" ", wads.get_iwad()->path.c_str());

		else
			exe_path += token;

		token = tz.get_token();
	}

	wads.close_wad(temp_wad->path);

	if (wxExecute(str_to_wx(exe_path)) == -1)
		wxMessageBox(_T("Couldn't find executable! Check the path at edit->preferences->misc"));
		*/
}

void EditorWindow::file_importmap(wxCommandEvent &event)
{
	string filename = "";
	wxFileDialog browse(NULL, _T("Open Wad"), _T(""), _T(""), _T("Doom Wad Files (*.wad)|*.wad"), wxOPEN|wxFILE_MUST_EXIST);
	if (browse.ShowModal() == wxID_OK)
		filename = wx_to_str(browse.GetPath());

	if (filename != "")
	{
		Wad temp_wad;

		if (!temp_wad.open(filename))
		{
			wxMessageBox(_T("Error Opening Wadfile!"));
			return;
		}

		string mapname;
		wxArrayString mapnames;
		for (int a = 0; a < temp_wad.available_maps.size(); a++)
			mapnames.Add(str_to_wx(temp_wad.available_maps[a]));

		if (mapnames.size() == 0)
		{
			wxMessageBox(_T("The wadfile doesn't have any maps"));
			return;
		}

		mapname = wx_to_str(wxGetSingleChoice(_T("Select Map"), _T("Import Map"), mapnames));

		if (mapname == "")
			return;

		d_map.close();
		d_map.open(&temp_wad, mapname);
		map_area->redraw();
	}
}

void EditorWindow::edit_scripts(wxCommandEvent &event)
{
	/*
	if (!map.hexen)
		return;

	if (!script_editor)
		script_editor = new ScriptEditor(this);

	script_editor->Show();
	*/
}

void EditorWindow::edit_prefs(wxCommandEvent &event)
{
	PrefsDialog pd;
	pd.ShowModal();
}

void EditorWindow::edit_properties(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	edit_item();
}

void EditorWindow::edit_mirrorx(wxCommandEvent &event)
{
	/*
	if (!map.opened)
		return;

	lock_hilight = true;

	if (edit_mode == 3)
		thing_mirror(true, false, true);
	else
		vert_mirror(true, false);

	lock_hilight = false;
	*/
}

void EditorWindow::edit_mirrory(wxCommandEvent &event)
{
	/*
	if (!map.opened)
		return;

	lock_hilight = true;

	if (edit_mode == 3)
		thing_mirror(false, true, true);
	else
		vert_mirror(false, true);

	lock_hilight = false;
	*/
}

void EditorWindow::edit_rotate(wxCommandEvent &event)
{
	/*
	if (!map.opened)
		return;

	lock_hilight = true;

	int ang = wxGetNumberFromUser(_T(""), _T("Enter degrees to rotate"), _T("Rotate"), 0, -360, 360, this);

	if (ang == -1)
	{
		lock_hilight = false;
		return;
	}
	else
	{
		if (edit_mode == 3)
			thing_rotate(ang, true);
		else
			vert_rotate(ang);

		lock_hilight = false;
	}
	*/
}

void EditorWindow::edit_alignx(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	line_align_x();
}

void EditorWindow::edit_correctrefs(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	lock_hilight = true;
	line_correct_references();
	lock_hilight = false;
}

void EditorWindow::edit_splitline(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	lock_hilight = true;
	long splits = wxGetNumberFromUser(_T(""), _T("Enter Number of Segments:"), _T("Split Line"), 2, 2, 100);

	line_split(splits);
	lock_hilight = false;
}

void EditorWindow::edit_extrudeline(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	lock_hilight = true;
	int dist = atoi(chr(wxGetTextFromUser(_T("Enter Distance to Extrude:"), _T("Extrude Line"), _T("8"))));

	line_extrude(dist);
	lock_hilight = false;
}

void EditorWindow::edit_flipline(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	lock_hilight = true;
	line_flip(true, true);
	lock_hilight = false;

	redraw_map();
}

void EditorWindow::edit_createsector(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	vector<Sector*> temp;
	sector_create(down_pos(true), temp);
}

void EditorWindow::edit_mergesectors(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	sector_merge(false);
}

void EditorWindow::edit_joinsectors(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	sector_merge(true);
}

void EditorWindow::edit_createdoor(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	lock_hilight = true;
	string tex = "";

	setup_tex_browser(TEXTURES_WALLS);
	TextureBrowser tb(game.def_doortex);
	if (tb.ShowModal() == wxID_OK)
		tex = tb.get_texture();
	else
		return;

	sector_create_door(tex);
	lock_hilight = false;
}

void EditorWindow::edit_createstairs(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	lock_hilight = true;
	// Custom dialog ahoy
	wxDialog dlg(NULL, -1, _T("Create Stairs"), wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	dlg.SetSizer(vbox);
	wxTextCtrl *entry_floor = new wxTextCtrl(&dlg, -1, _T("8"));
	wxTextCtrl *entry_ceil = new wxTextCtrl(&dlg, -1, _T("0"));
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(&dlg, -1, _T("Floor Step:")), 1, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_floor, 0, wxEXPAND);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(&dlg, -1, _T("Ceiling Step:")), 1, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_ceil, 0, wxEXPAND);
	vbox->Add(dlg.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);
	dlg.SetBestFittingSize();

	if (dlg.ShowModal() == wxID_OK)
		sector_create_stairs(atoi(chr(entry_floor->GetValue())), atoi(chr(entry_ceil->GetValue())));

	lock_hilight = false;
}

void EditorWindow::edit_lightgradient(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	// Custom dialog ahoy
	wxDialog dlg(NULL, -1, _T("Create Stairs"), wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	dlg.SetSizer(vbox);
	wxCheckBox *cb_first_to_last = new wxCheckBox(&dlg, -1, _T("Use Existing Values"));
	wxTextCtrl *entry_step = new wxTextCtrl(&dlg, -1, _T("16"));
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(&dlg, -1, _T("Light Step:")), 1, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_step, 0, wxEXPAND);
	vbox->Add(cb_first_to_last, 0, wxEXPAND|wxALL, 4);
	vbox->Add(dlg.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);
	dlg.SetBestFittingSize();

	if (dlg.ShowModal() == wxID_OK)
	{
		if (cb_first_to_last->GetValue())
			sector_light_gradient(9999);
		else
			sector_light_gradient(atoi(chr(entry_step->GetValue())));
	}
}

void EditorWindow::mode_vertices(wxCommandEvent &event)
{
	change_edit_mode(0);
	update_statusbar();
}

void EditorWindow::mode_lines(wxCommandEvent &event)
{
	change_edit_mode(1);
	update_statusbar();
}

void EditorWindow::mode_sectors(wxCommandEvent &event)
{
	change_edit_mode(2);
	update_statusbar();
}

void EditorWindow::mode_things(wxCommandEvent &event)
{
	change_edit_mode(3);
	update_statusbar();
}

void EditorWindow::mode_3d(wxCommandEvent &event)
{
	if (d_map.opened())
		start_3d_mode();
}

void EditorWindow::view_drawsidebar(wxCommandEvent &event)
{
	/*
	show_sidebar = 1;
	sb_draw->Show();
	sb_theme->Show(false);
	Layout();
	*/
}

void EditorWindow::view_themesidebar(wxCommandEvent &event)
{
	/*
	show_sidebar = 2;
	sb_draw->Show(false);
	sb_theme->Show();
	Layout();
	*/
}

void EditorWindow::view_hidesidebar(wxCommandEvent &event)
{
	/*
	show_sidebar = 0;
	sb_draw->Show(false);
	sb_theme->Show(false);
	Layout();
	*/
}

void EditorWindow::view_console(wxCommandEvent &event)
{
	console_window->Show();
}

void EditorWindow::view_setbgtex(wxCommandEvent &event)
{
	/*
	if (!map.opened)
		return;

	TextureBrowser tb(2);

	if (tb.ShowModal() == wxID_OK)
		bg_tex = tb.get_texture();

	redraw_map();
	*/
}

void EditorWindow::view_clearbgtex(wxCommandEvent &event)
{
	/*
	bg_tex = "-";
	redraw_map();
	*/
}

void EditorWindow::check_removeverts(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	int v = d_map.remove_unused_vertices();
	wxMessageBox(wxString::Format(_T("Removed %d Free Vertices"), v));
}

void EditorWindow::check_removelines(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	int l = d_map.remove_zerolength_lines();
	wxMessageBox(wxString::Format(_T("Removed %d Zero-Length Lines"), l));
}

void EditorWindow::check_removesectors(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	int s = d_map.remove_unused_sectors();
	wxMessageBox(wxString::Format(_T("Removed %d Unused Sectors"), s));
}

void EditorWindow::check_removesides(wxCommandEvent &event)
{
	/*
	if (!map.opened)
		return;

	int s = remove_unused_sides();
	wxMessageBox(wxString::Format(_T("Removed %d Unused Sidedefs"), s));
	*/
}

void EditorWindow::check_checktextures(wxCommandEvent &event)
{
	/*
	if (!map.opened)
		return;

	check_textures();
	*/
}

void EditorWindow::thing_insert(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	TTypeSelectDialog td(last_thing.get_type());

	if (td.ShowModal() == wxID_OK)
	{
		last_thing.set_type(td.get_type());
		create_thing(map_area->get_down_pos(true));
		redraw_map(true, false);
	}
}

void EditorWindow::line_splitat(wxCommandEvent &event)
{
	if (!d_map.opened())
		return;

	line_split_at(down_pos(true));
}
