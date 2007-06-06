
#include "wx_stuff.h"

class STypeSelectDialog : public wxDialog
{
private:
	wxListBox	*list_types;
	int			type;

	wxChoice	*combo_damage;
	wxCheckBox	*cbox_secret;
	wxCheckBox	*cbox_friction;
	wxCheckBox	*cbox_wind;

public:
	STypeSelectDialog(int init);
	~STypeSelectDialog();

	int get_value();
};
