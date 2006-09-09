
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
	wxCheckBox	*cb_thing_sprites;
	wxCheckBox	*cb_thing_angles;
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
		VP_THING_SPRITES,
		VP_THING_ANGLES,
		VP_LINE_AA,
		VP_TEX_NP2,

		VP_LINE_SIZE,
		VP_TEX_FILTER,
		VP_XHAIR_2D,
	};

	VisualPrefs(wxWindow *parent);
	~VisualPrefs();

	void grid_dash_toggled(wxCommandEvent &event);
	void grid_64_toggled(wxCommandEvent &event);
	void grid_snap_toggled(wxCommandEvent &event);
	void thing_sprites_toggled(wxCommandEvent &event);
	void thing_angles_toggled(wxCommandEvent &event);
	void line_aa_toggled(wxCommandEvent &event);
	void tex_np2_toggled(wxCommandEvent &event);

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
	//wxColour	colour;
	//float		alpha;

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

	wxButton	*btn_defaults;
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

		IP_BTN_DEFAULTS,
		IP_BTN_LOADCONFIG,
		IP_BTN_SAVECONFIG,
	};

	InputPrefs(wxWindow *parent);
	~InputPrefs();

	void list_controls_changed(wxCommandEvent &event);
	void btn_defaults_clicked(wxCommandEvent &event);
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

/*
class MiscPrefs : public wxPanel
{
private:
	bool init;

	wxTextCtrl		*entry_path_doomexe;
	wxTextCtrl		*entry_runparams;
	wxButton		*btn_browse_doomexe;

	wxChoice		*combo_nodebuilder;
	wxTextCtrl		*entry_path_zdbsp;
	wxTextCtrl		*entry_path_zennode;
	wxTextCtrl		*entry_path_bsp;
	wxTextCtrl		*entry_path_glbsp;
	wxButton		*btn_browse_zdbsp;
	wxButton		*btn_browse_zennode;
	wxButton		*btn_browse_bsp;
	wxButton		*btn_browse_glbsp;

public:
	enum
	{
		MP_ENTRY_DOOMEXE,
		MP_ENTRY_RUNPARAMS,
		MP_BTN_BROWSE_DOOMEXE,

		MP_COMBO_NODEBUILDER,
		MP_ENTRY_ZDBSP,
		MP_ENTRY_ZENNODE,
		MP_ENTRY_BSP,
		MP_ENTRY_GLBSP,
		MP_BTN_BROWSE_ZDBSP,
		MP_BTN_BROWSE_ZENNODE,
		MP_BTN_BROWSE_BSP,
		MP_BTN_BROWSE_GLBSP,
	};

	MiscPrefs(wxWindow *parent);
	~MiscPrefs();

	void entry_doomexe_changed(wxCommandEvent &event);
	void entry_runparams_changed(wxCommandEvent &event);
	void btn_doomexe_clicked(wxCommandEvent &event);

	void combo_nodebuilder_changed(wxCommandEvent &event);
	void entry_zdbsp_changed(wxCommandEvent &event);
	void entry_zennode_changed(wxCommandEvent &event);
	void entry_bsp_changed(wxCommandEvent &event);
	void entry_glbsp_changed(wxCommandEvent &event);
	void btn_zdbsp_clicked(wxCommandEvent &event);
	void btn_zennode_clicked(wxCommandEvent &event);
	void btn_bsp_clicked(wxCommandEvent &event);
	void btn_glbsp_clicked(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
*/

#endif //__PREFS_DIALOG_H__
