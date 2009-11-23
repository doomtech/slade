
#ifndef __MODIFYOFFSETSDIALOG_H__
#define	__MODIFYOFFSETSDIALOG_H__

#include <wx/dialog.h>

class ModifyOffsetsDialog : public wxDialog {
private:
	wxChoice	*combo_aligntype;

	wxTextCtrl	*entry_xoff;
	wxTextCtrl	*entry_yoff;
	wxCheckBox	*cbox_relative;

	wxRadioButton	*opt_set;
	wxRadioButton	*opt_auto;

public:
	ModifyOffsetsDialog();
	~ModifyOffsetsDialog();

	enum {
		MOD_OPT_SET,
		MOD_OPT_AUTO,
	};

	point2_t	getOffset();
	int			getAlignType();
	bool		autoOffset();
	bool		relativeOffset();
	bool		xOffChange();
	bool		yOffChange();

	void		onOptSet(wxCommandEvent &event);
	void		onOptAuto(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif//__MODIFYOFFSETSDIALOG_H__
