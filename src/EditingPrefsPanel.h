
#ifndef __EDITING_PREFS_PANEL_H__
#define __EDITING_PREFS_PANEL_H__

class EditingPrefsPanel : public wxPanel {
private:
	wxCheckBox*	cb_wad_force_uppercase;
	wxChoice*	choice_entry_mod;

public:
	EditingPrefsPanel(wxWindow* parent);
	~EditingPrefsPanel();

	void	applyPreferences();
};

#endif//__EDITING_PREFS_PANEL_H__
