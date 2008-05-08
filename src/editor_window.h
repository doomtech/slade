

#ifndef __EDITOR_WINDOW_H__
#define __EDITOR_WINDOW_H__

#include "wx_stuff.h"
#include "info_bar.h"
#include "map_canvas.h"
#include "wad.h"

class EditorWindow : public wxFrame
{
private:
	InfoBar		*info_bar;
	MapCanvas	*map_area;

	wxMenu		*line_context_menu;
	wxMenu		*sector_context_menu;
	wxMenu		*thing_context_menu;

	wxPanel		*sb_draw;
	wxPanel		*sb_theme;

protected:

public:
	enum
	{
		// File menu
		EWMENU_FILE_WADMANAGER,
		EWMENU_FILE_SAVE,
		EWMENU_FILE_SAVEAS,
		EWMENU_FILE_SAVESTANDALONE,
		EWMENU_FILE_RUN,
		EWMENU_FILE_RENAME,
		EWMENU_FILE_IMPORT,
		EWMENU_FILE_CLOSE,
		EWMENU_FILE_EXIT,

		// Line edit menu
		EWMENU_EDIT_LINE,
		EWMENU_EDIT_ALIGNX,
		EWMENU_EDIT_ALIGNY,
		EWMENU_EDIT_CORRECTREFS,
		EWMENU_EDIT_SPLITLINE,
		EWMENU_EDIT_EXTRUDELINE,
		EWMENU_EDIT_FLIPLINE,

		// Sector edit menu
		EWMENU_EDIT_SECTOR,
		EWMENU_EDIT_CREATESECTOR,
		EWMENU_EDIT_MERGESECTORS,
		EWMENU_EDIT_JOINSECTORS,
		EWMENU_EDIT_CREATEDOOR,
		EWMENU_EDIT_CREATESTAIRS,
		EWMENU_EDIT_LIGHTGRADIENT,

		// Edit menu
		EWMENU_EDIT_SCRIPTS,
		EWMENU_EDIT_PREFS,
		EWMENU_EDIT_PROPERTIES,
		EWMENU_EDIT_MIRRORX,
		EWMENU_EDIT_MIRRORY,
		EWMENU_EDIT_ROTATE,

		// Mode menu
		EWMENU_MODE_VERTS,
		EWMENU_MODE_LINES,
		EWMENU_MODE_SECTORS,
		EWMENU_MODE_THINGS,
		EWMENU_MODE_3D,

		// View menu
		EWMENU_VIEW_DRAWSIDEBAR,
		EWMENU_VIEW_THEMESIDEBAR,
		EWMENU_VIEW_HIDESIDEBAR,
		EWMENU_VIEW_CONSOLE,
		EWMENU_VIEW_SETBGTEX,
		EWMENU_VIEW_CLEARBGTEX,

		// Checks menu
		EWMENU_CHECK_REMOVEVERTS,
		EWMENU_CHECK_REMOVELINES,
		EWMENU_CHECK_REMOVESECTORS,
		EWMENU_CHECK_REMOVESIDES,
		EWMENU_CHECK_TEXTURES,

		// Thing context menu
		EWMENU_THING_INSERT,

		// Line context menu
		EWMENU_LINE_SPLITAT,
	};

	EditorWindow(const wxChar *title, int xpos, int ypos, int width, int height);
	~EditorWindow();

	void setup_menu();
	void setup_toolbar();

	bool open_map(Wad* wad, string mapname);
	void update_infobar() { info_bar->update(); }
	void update_statusbar();
	void change_infobar();
	void popup_context_menu() { map_area->popup_menu(); }
	void draw_shape();

	MapCanvas* map() { return map_area; }

	// Window events
	//void key_down(wxKeyEvent &event);
	//void key_up(wxKeyEvent &event);
	void close(wxCloseEvent &event);

	// Menu events
	void file_wadmanager(wxCommandEvent &event);
	void file_close(wxCommandEvent &event);
	void file_exit(wxCommandEvent &event);
	void file_save(wxCommandEvent &event);
	void file_saveas(wxCommandEvent &event);
	void file_savestandalone(wxCommandEvent &event);
	void file_rename(wxCommandEvent &event);
	void file_run(wxCommandEvent &event);
	void file_importmap(wxCommandEvent &event);

	void edit_scripts(wxCommandEvent &event);
	void edit_prefs(wxCommandEvent &event);
	void edit_properties(wxCommandEvent &event);
	void edit_mirrorx(wxCommandEvent &event);
	void edit_mirrory(wxCommandEvent &event);
	void edit_rotate(wxCommandEvent &event);

	void edit_alignx(wxCommandEvent &event);
	void edit_correctrefs(wxCommandEvent &event);
	void edit_splitline(wxCommandEvent &event);
	void edit_extrudeline(wxCommandEvent &event);
	void edit_flipline(wxCommandEvent &event);

	void edit_mergesectors(wxCommandEvent &event);
	void edit_joinsectors(wxCommandEvent &event);
	void edit_createdoor(wxCommandEvent &event);
	void edit_createstairs(wxCommandEvent &event);
	void edit_createsector(wxCommandEvent &event);
	void edit_lightgradient(wxCommandEvent &event);

	void mode_vertices(wxCommandEvent &event);
	void mode_lines(wxCommandEvent &event);
	void mode_sectors(wxCommandEvent &event);
	void mode_things(wxCommandEvent &event);
	void mode_3d(wxCommandEvent &event);

	void view_drawsidebar(wxCommandEvent &event);
	void view_themesidebar(wxCommandEvent &event);
	void view_hidesidebar(wxCommandEvent &event);
	void view_console(wxCommandEvent &event);
	void view_setbgtex(wxCommandEvent &event);
	void view_clearbgtex(wxCommandEvent &event);

	void check_removeverts(wxCommandEvent &event);
	void check_removelines(wxCommandEvent &event);
	void check_removesectors(wxCommandEvent &event);
	void check_removesides(wxCommandEvent &event);
	void check_checktextures(wxCommandEvent &event);

	void thing_insert(wxCommandEvent &event);

	void line_splitat(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif
