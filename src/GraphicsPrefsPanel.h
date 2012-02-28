
#ifndef __GFX_PREFS_PANEL_H__
#define __GFX_PREFS_PANEL_H__
#include <wx/clrpicker.h>

class GraphicsPrefsPanel : public wxPanel {
private:
	wxColourPickerCtrl*	cp_colour1;
	wxColourPickerCtrl*	cp_colour2;
	wxChoice*			choice_presets;
	wxCheckBox*			cb_show_border;
	wxCheckBox*			cb_extra_gfxconv;
	wxCheckBox*			cb_browser_bg_black;

public:
	GraphicsPrefsPanel(wxWindow* parent);
	~GraphicsPrefsPanel();

	void	applyPreferences();

	// Events
	void	onChoicePresetSelected(wxCommandEvent& e);
};

#endif//__GFX_PREFS_PANEL_H__
