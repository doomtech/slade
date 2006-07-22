

#include "wx_stuff.h"
#include <wx/spinctrl.h>
#include "tex_box.h"

class FlagCheckBox : public wxCheckBox
{
private:

public:
	int	flag;

	FlagCheckBox(wxWindow *parent, int flag, wxString label);
	~FlagCheckBox();

	void apply_flag(WORD *flags);
};


class LinePropertiesPage : public wxPanel
{
private:
	vector<FlagCheckBox*>	flags;

	wxStaticText	*label_special;
	wxTextCtrl		*entry_special;
	wxButton		*btn_changespecial;

	wxTextCtrl		*entry_tag;
	wxButton		*btn_getfreetag;

	wxButton		*btn_editargs;
	wxChoice		*combo_trigger;

	BYTE			args[5];
	bool			args_consistent[5];

public:
	enum
	{
		LP_BTN_CHANGESPECIAL,
		LP_BTN_GETFREETAG,
		LP_BTN_EDITARGS,
		LP_COMBO_TRIGGER,
		LP_ENTRY_SPECIAL,
	};

	LinePropertiesPage(wxWindow *parent);
	~LinePropertiesPage();

	void setup_flag_checkbox(WORD flag, wxString label, wxSizer *box);
	void setup_widgets();
	void apply_changes();

	void change_special_clicked(wxCommandEvent &event);
	void get_free_tag_clicked(wxCommandEvent &event);
	void edit_args_clicked(wxCommandEvent &event);
	void entry_special_changed(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};


class SidePropertiesPage : public wxPanel
{
private:
	int			side;

	//TextureBox	*tex_upper;
	//TextureBox	*tex_middle;
	//TextureBox	*tex_lower;

	wxTextCtrl	*entry_upper;
	wxTextCtrl	*entry_middle;
	wxTextCtrl	*entry_lower;

	wxTextCtrl	*entry_xoff;
	wxTextCtrl	*entry_yoff;

	wxTextCtrl	*entry_sector;
	wxButton	*btn_create;
	wxButton	*btn_delete;

	ClickableTexBox	*tex_upper;
	ClickableTexBox	*tex_middle;
	ClickableTexBox	*tex_lower;

public:
	enum
	{
		SP_ENTRY_UPPER,
		SP_ENTRY_MIDDLE,
		SP_ENTRY_LOWER,
		SP_TEX_UPPER,
		SP_TEX_MIDDLE,
		SP_TEX_LOWER,
	};

	SidePropertiesPage(wxWindow *parent, int side = 1);
	~SidePropertiesPage();

	void setup_widgets();
	void apply_changes();

	void upper_tex_entry_changed(wxCommandEvent& event);
	void middle_tex_entry_changed(wxCommandEvent& event);
	void lower_tex_entry_changed(wxCommandEvent& event);

	void upper_tex_clicked(wxMouseEvent &event);

	DECLARE_EVENT_TABLE()
};


class LineEditor : public wxDialog
{
private:
	wxNotebook			*tabs;
	LinePropertiesPage	*props_page;
	SidePropertiesPage	*side1_page;
	SidePropertiesPage	*side2_page;

public:
	LineEditor();
	~LineEditor();

	void apply_changes();
};
