
#ifndef __WAD_MANAGER_H__
#define __WAD_MANAGER_H__

#include "wx_stuff.h"
#include "structs.h"
#include "wad.h"
#include <wx/glcanvas.h>
#include <wx/notebook.h>

class MapPreviewCanvas : public wxGLCanvas
{
private:
	vector<rect_t>	preview_lines;
	point2_t		dimensions;

public:
	MapPreviewCanvas(wxWindow *parent);
	~MapPreviewCanvas();

	void preview_map(Wad* wad, string mapname);

	void paint(wxPaintEvent &event);
	void resize(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()
};

class WadManager : public wxFrame
{
private:
	wxNotebook	*tabs;

	wxListBox	*open_wads_list;
	wxListBox	*recent_wads_list;
	wxListBox	*available_maps;
	wxChoice	*game_combo;

	wxButton	*btn_open_wad;
	wxButton	*btn_close_wad;
	wxButton	*btn_close_all;
	wxButton	*btn_new_standalone;
	wxButton	*btn_new_map;

	MapPreviewCanvas	*map_preview;

protected:

public:

	enum
	{
		WM_GAME_COMBO,
		WM_RECENT_WADS,
		WM_OPEN_WADS,
		WM_MAPS_LIST,

		WM_BTN_OPENWAD,
		WM_BTN_CLOSE,
		WM_BTN_CLOSEALL,
		WM_BTN_NEWSTANDALONE,
		WM_BTN_NEWMAP,
	};

	WadManager(wxWindow *parent);
	~WadManager();

	wxStaticBoxSizer	*setup_game_combo_frame(wxWindow *panel);
	wxStaticBoxSizer	*setup_open_wads_frame(wxWindow *panel);
	wxStaticBoxSizer	*setup_recent_wads_frame(wxWindow *panel);
	wxStaticBoxSizer	*setup_map_list_frame(wxWindow *panel);

	void populate_wad_list();
	void populate_recent_wads_list();
	void populate_map_list();

	// Events
	void close(wxCloseEvent &event);

	void game_combo_changed(wxCommandEvent &event);

	void open_wads_list_changed(wxCommandEvent &event);
	void map_list_changed(wxCommandEvent &event);

	void open_wad_clicked(wxCommandEvent &event);
	void close_wad_clicked(wxCommandEvent &event);
	void close_all_clicked(wxCommandEvent &event);
	void new_standalone_clicked(wxCommandEvent &event);
	void new_map_clicked(wxCommandEvent &event);

	void recent_wads_list_activated(wxCommandEvent &event);
	void maps_list_activated(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

void save_recent_wads(FILE *fp);
void load_recent_wads(Tokenizer *tz);
void add_recent_wad(string path);
void load_open_wads(Tokenizer *tz);
void save_open_wads(FILE *fp);

#endif
