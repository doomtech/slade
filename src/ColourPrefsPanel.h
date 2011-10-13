
#ifndef __COL_PREFS_PANEL_H__
#define __COL_PREFS_PANEL_H__
#include <wx/clrpicker.h>
#include <wx/button.h>

class ColourPrefsPanel : public wxPanel {
private:
	wxColourPickerCtrl*	cp_err;
	wxColourPickerCtrl*	cp_loc;
	wxColourPickerCtrl*	cp_mod;
	wxColourPickerCtrl*	cp_new;
	wxButton*			button_reset;

public:
	ColourPrefsPanel(wxWindow* parent);
	~ColourPrefsPanel();

	void	applyPreferences();

	// Events
	void	onReset(wxCommandEvent& e);
};

#endif//__COL_PREFS_PANEL_H__
