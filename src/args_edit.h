
#ifndef __ARGS_EDIT_H__
#define __ARGS_EDIT_H__

#include "args.h"

class ArgValueDialog : public wxDialog
{
private:
	argtype_t			*arg_type;
	BYTE				arg;
	wxChoice			*combo_value;
	vector<wxCheckBox*>	cbox_flags;

	vector<BYTE>		values;
	vector<BYTE>		flags;

public:
	ArgValueDialog(BYTE value, argtype_t *arg_type);
	~ArgValueDialog();

	BYTE get_value();
};

class ArgEdit : public wxPanel
{
private:
	wxTextCtrl		*entry_arg;
	wxStaticText	*label_arg;
	wxButton		*btn_change;

	string			argname;
	argtype_t		*arg_type;

public:
	enum
	{
		AE_ENTRY_ARG,
		AE_BTN_CHANGE,
	};

	ArgEdit(wxWindow *parent, int val, string argname, argtype_t *arg_type);
	~ArgEdit();

	void entry_arg_changed(wxCommandEvent &event);
	void change_clicked(wxCommandEvent &event);

	int get_value();

	DECLARE_EVENT_TABLE()
};

class ArgsEditor : public wxDialog
{
private:
	ArgEdit		*args[5];

public:
	ArgsEditor(BYTE* args, string* argnames, string* argtypes, bool* consistent);
	~ArgsEditor();

	void apply_changes(BYTE* args, bool* consistent);
};

#endif //__ARGS_EDIT_H__
