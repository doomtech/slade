
#ifndef __EDITING_PREFS_PANEL_H__
#define __EDITING_PREFS_PANEL_H__

class EditingPrefsPanel : public wxPanel {
private:
	wxCheckBox*	cb_wad_force_uppercase;
	wxCheckBox*	cb_wad_lock_iwad;
	wxChoice*	choice_entry_mod;

public:
	EditingPrefsPanel(wxWindow* parent);
	~EditingPrefsPanel();

	void	applyPreferences();
};

#endif//__EDITING_PREFS_PANEL_H__
