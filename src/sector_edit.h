
#include "tex_box.h"

class SectorEditor : public wxDialog
{
private:
	TextureBox		*tbox_ftex;
	wxTextCtrl		*entry_ftex;
	wxTextCtrl		*entry_fheight;

	TextureBox		*tbox_ctex;
	wxTextCtrl		*entry_ctex;
	wxTextCtrl		*entry_cheight;

	wxTextCtrl		*entry_special;
	wxButton		*btn_changespecial;
	wxStaticText	*label_special;

	wxTextCtrl		*entry_tag;
	wxButton		*btn_findtag;

	wxTextCtrl		*entry_light;

	void setup_widgets();

public:
	enum
	{
		SE_BTN_CHANGESPECIAL,
		SE_BTN_FINDTAG,
		SE_ENTRY_FTEX,
		SE_ENTRY_CTEX,
		SE_ENTRY_SPECIAL,
	};
	
	SectorEditor();
	~SectorEditor();

	void apply_changes();

	void btn_changespecial_clicked(wxCommandEvent &event);
	void btn_findtag_clicked(wxCommandEvent &event);
	void entry_ftex_changed(wxCommandEvent &event);
	void entry_ctex_changed(wxCommandEvent &event);
	void entry_special_changed(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
