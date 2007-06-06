
#ifndef __PREFS_DIALOG_H__
#define __PREFS_DIALOG_H__

#include "wx_stuff.h"
#include <wx/notebook.h>
#include <wx/tglbtn.h>
#include "keybind.h"

class PrefsDialog : public wxDialog
{
private:
	wxNotebook			*tabs;

public:
	PrefsDialog();
	~PrefsDialog();
};

class VisualPrefs : public wxPanel
{
private:
	wxCheckBox	*cb_grid_dash;
	wxCheckBox	*cb_64_grid;
	wxCheckBox	*cb_snap_to_grid;
	wxCheckBox	*cb_grid_origin;
	wxCheckBox	*cb_thing_sprites;
	wxCheckBox	*cb_thing_angles;
	wxCheckBox	*cb_pan_detail;
	wxChoice	*ch_2d_xhair;

	wxSlider	*sl_line_size;
	wxCheckBox	*cb_line_aa;

	wxCheckBox	*cb_tex_np2;
	wxChoice	*ch_tex_filter;

public:
	enum
	{
		VP_GRID_DASH,
		VP_64_GRID,
		VP_SNAP_GRID,
		VP_GRID_ORIGIN,
		VP_THING_SPRITES,
		VP_THING_ANGLES,
		VP_LINE_AA,
		VP_TEX_NP2,
		VP_PAN_DETAIL,

		VP_LINE_SIZE,
		VP_TEX_FILTER,
		VP_XHAIR_2D,
	};

	VisualPrefs(wxWindow *parent);
	~VisualPrefs();

	void cbox_toggled(wxCommandEvent &event);

	void filter_changed(wxCommandEvent &event);
	void xhair_2d_changed(wxCommandEvent &event);
	void line_size_changed(wxScrollEvent &event);

	DECLARE_EVENT_TABLE()
};

#include "structs.h"

class ColourBox : public wxPanel
{
private:
	rgba_t		*rgba;

public:
	ColourBox(wxWindow *parent, rgba_t *colour);
	~ColourBox();

	void set_colour(rgba_t *colour) { rgba = colour; }

	void paint(wxPaintEvent &event);
	void clicked(wxMouseEvent &event);
	void clicked2(wxMouseEvent &event);

	DECLARE_EVENT_TABLE()
};

class ColourEdit : public wxPanel
{
private:
	ColourBox		*cbox_colour;
	wxCheckBox		*cb_additive;
	rgba_t			*col;

public:
	ColourEdit(wxWindow *parent, rgba_t *colour, string name);
	~ColourEdit();

	void set_colour(rgba_t *colour) { col = colour; }

	rgba_t *get_colour() { return col; }

	void cb_additive_changed(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

class ColourPrefs : public wxPanel
{
private:
	wxButton		*btn_saveconfig;
	wxButton		*btn_loadconfig;
	wxChoice		*combo_configs;

public:
	enum
	{
		CP_COMBO_CONFIGS,
		CP_BTN_SAVE,
		CP_BTN_LOAD,
	};

	ColourPrefs(wxWindow *parent);
	~ColourPrefs();

	void combo_configs_changed(wxCommandEvent &event);
	void save_clicked(wxCommandEvent &event);
	void load_clicked(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

class InputTextCtrl : public wxTextCtrl
{
private:
	wxToggleButton	*btn_shift;
	wxToggleButton	*btn_ctrl;
	wxToggleButton	*btn_alt;

public:
	InputTextCtrl(wxWindow *parent, int id);
	~InputTextCtrl();

	void set_buttons(wxToggleButton *s, wxToggleButton *c, wxToggleButton *a);

	void key_down(wxKeyEvent &event);

	DECLARE_EVENT_TABLE()
};

class InputControlDialog : public wxDialog
{
private:
	InputTextCtrl	*text_key;
	wxChoice		*combo_keys;
	wxToggleButton	*btn_shift;
	wxToggleButton	*btn_ctrl;
	wxToggleButton	*btn_alt;

public:
	enum
	{
		ICD_TEXT_KEY,
		ICD_COMBO_KEYS,
	};

	InputControlDialog(wxWindow *parent, keybind_t *init = NULL);
	~InputControlDialog();

	string	get_key();
	BYTE	get_mods();

	void	combo_keys_changed(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

class InputPrefs : public wxPanel
{
private:
	wxListBox	*list_controls;
	wxListBox	*list_binds;

	wxButton	*btn_addbind;
	wxButton	*btn_removebind;
	wxButton	*btn_changebind;
	wxButton	*btn_defaultbind;

	wxButton	*btn_readconfig;
	wxButton	*btn_loadconfig;
	wxButton	*btn_saveconfig;

public:
	enum
	{
		IP_LIST_CONTROLS,
		IP_LIST_BINDS,

		IP_BTN_ADDBIND,
		IP_BTN_REMOVEBIND,
		IP_BTN_CHANGEBIND,
		IP_BTN_DEFAULTBIND,

		IP_BTN_READCONFIG,
		IP_BTN_LOADCONFIG,
		IP_BTN_SAVECONFIG,
	};

	InputPrefs(wxWindow *parent);
	~InputPrefs();

	void list_controls_changed(wxCommandEvent &event);
	void btn_readconfig_clicked(wxCommandEvent &event);
	void btn_loadconfig_clicked(wxCommandEvent &event);
	void btn_saveconfig_clicked(wxCommandEvent &event);
	void btn_addbind_clicked(wxCommandEvent &event);
	void btn_removebind_clicked(wxCommandEvent &event);
	void btn_changebind_clicked(wxCommandEvent &event);
	void btn_defaultbind_clicked(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

class RenderPrefs : public wxPanel
{
private:
	wxCheckBox		*cb_fog;
	wxCheckBox		*cb_fullbright;
	wxCheckBox		*cb_hilight;
	wxCheckBox		*cb_hilightthings;
	wxChoice		*combo_thingstyle;

	wxSlider		*sl_movespeed;
	wxSlider		*sl_mousespeed;
	wxCheckBox		*cb_invertmouse;

	wxCheckBox		*cb_desktopres;
	wxChoice		*combo_resolutions;

public:
	enum
	{
		RP_CB_FOG,
		RP_CB_FULLBRIGHT,
		RP_CB_HILIGHT,
		RP_CB_HLTHINGS,
		RP_COMBO_THINGSTYLE,

		RP_SL_MOVESPEED,
		RP_SL_MOUSESPEED,
		RP_CB_INVERTMOUSE,

		RP_CB_DESKRES,
		RP_COMBO_RES,
	};

	RenderPrefs(wxWindow *parent);
	~RenderPrefs();


	void cb_fog_toggled(wxCommandEvent &event);
	void cb_fullbright_toggled(wxCommandEvent &event);
	void cb_hilight_toggled(wxCommandEvent &event);
	void cb_hilightthings_toggled(wxCommandEvent &event);
	void combo_thingstyle_changed(wxCommandEvent &event);
	void sl_movespeed_changed(wxScrollEvent &event);
	void sl_mousespeed_changed(wxScrollEvent &event);
	void cb_invertmouse_toggled(wxCommandEvent &event);
	void cb_desktopres_toggled(wxCommandEvent &event);
	void combo_res_changed(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

class EditPrefs : public wxPanel
{
private:
	wxCheckBox		*cb_auto_split;
	wxCheckBox		*cb_auto_merge;
	wxCheckBox		*cb_auto_createsector;
	wxCheckBox		*cb_keep_selection;
	wxCheckBox		*cb_zoom_cursor;

public:
	enum
	{
		EP_CB_AUTOSPLIT,
		EP_CB_AUTOMERGE,
		EP_CB_AUTOCREATE,
		EP_CB_KEEPSELECTION,
		EP_CB_ZOOMCURSOR,
	};

	EditPrefs(wxWindow *parent);
	~EditPrefs();

	void cb_auto_split_toggled(wxCommandEvent &event);
	void cb_auto_merge_toggled(wxCommandEvent &event);
	void cb_auto_createsector_toggled(wxCommandEvent &event);
	void cb_keep_selection_toggled(wxCommandEvent &event);
	void cb_zoom_cursor_toggled(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

class MiscPrefs : public wxPanel
{
private:
	bool init;

	wxTextCtrl		*entry_path_doomexe;
	wxTextCtrl		*entry_runparams;
	wxButton		*btn_browse_doomexe;

	wxCheckBox		*cb_nodes_internal;
	wxTextCtrl		*entry_path_nodebuilder;
	wxButton		*btn_browse_nodebuilder;
	wxTextCtrl		*entry_nodes_params;

public:
	enum
	{
		MP_ENTRY_DOOMEXE,
		MP_ENTRY_RUNPARAMS,
		MP_BTN_BROWSE_DOOMEXE,
		MP_CB_NODES_INTERNAL,
		MP_ENTRY_PATH_NODES,
		MP_BTN_BROWSE_NODES,
		MP_ENTRY_NODES_PARAMS,
	};

	MiscPrefs(wxWindow *parent);
	~MiscPrefs();

	void entry_doomexe_changed(wxCommandEvent &event);
	void entry_runparams_changed(wxCommandEvent &event);
	void btn_doomexe_clicked(wxCommandEvent &event);
	void entry_nodepath_changed(wxCommandEvent &event);
	void entry_nodeparams_changed(wxCommandEvent &event);
	void btn_nodepath_clicked(wxCommandEvent &event);
	void cb_nodes_internal_toggled(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__PREFS_DIALOG_H__
