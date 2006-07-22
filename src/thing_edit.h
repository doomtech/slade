
#ifndef __THING_EDIT_H__
#define __THING_EDIT_H__

#include "tex_box.h"

class ThingEditor : public wxDialog
{
private:
	TextureBox				*sprite;
	wxTextCtrl				*type_entry;
	wxButton				*change_type;
	wxButton				*browse_type;
	wxStaticText			*type_label;

	vector<FlagCheckBox*>	flags;

	wxRadioButton			*angle_n;
	wxRadioButton			*angle_ne;
	wxRadioButton			*angle_e;
	wxRadioButton			*angle_se;
	wxRadioButton			*angle_s;
	wxRadioButton			*angle_sw;
	wxRadioButton			*angle_w;
	wxRadioButton			*angle_nw;
	wxRadioButton			*angle_unknown;
	wxTextCtrl				*angle_entry;

	wxTextCtrl				*tid_entry;
	wxTextCtrl				*z_entry;
	wxTextCtrl				*special_entry;
	
	wxButton				*find_tid;
	wxButton				*change_special;
	wxButton				*edit_args;

	BYTE					args[5];
	bool					args_consistent[5];

	void setup_flag_checkbox(WORD flag, wxString label, wxSizer *box);

public:
	enum
	{
		TE_ENTRY_TYPE,
		TE_ENTRY_ANGLE,
		TE_ENTRY_TID,
		TE_ENTRY_Z,
		TE_ENTRY_SPECIAL,

		TE_RADIO_N,
		TE_RADIO_NE,
		TE_RADIO_E,
		TE_RADIO_SE,
		TE_RADIO_S,
		TE_RADIO_SW,
		TE_RADIO_W,
		TE_RADIO_NW,

		TE_BTN_CHANGETYPE,
		TE_BTN_BROWSETYPE,
		TE_BTN_FINDTID,
		TE_BTN_CHANGESPECIAL,
		TE_BTN_EDITARGS,
	};

	ThingEditor(int type = 1);
	~ThingEditor();

	void setup_widgets();
	void apply_changes();

	void angle_radio_changed(wxCommandEvent &event);
	void type_entry_changed(wxCommandEvent &event);
	void angle_entry_changed(wxCommandEvent &event);
	void change_type_clicked(wxCommandEvent &event);
	void find_tid_clicked(wxCommandEvent &event);
	void change_special_clicked(wxCommandEvent &event);
	void edit_args_clicked(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__THING_EDIT_H__
