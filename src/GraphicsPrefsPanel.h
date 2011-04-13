
#ifndef __GFX_PREFS_PANEL_H__
#define __GFX_PREFS_PANEL_H__
#include <wx/clrpicker.h>

class GraphicsPrefsPanel : public wxPanel {
private:
	wxColourPickerCtrl*	cp_colour1;
	wxColourPickerCtrl*	cp_colour2;
	wxButton*			b_reset;
	wxButton*			b_cyan;
	wxButton*			b_magenta;
	wxButton*			b_black;
	wxButton*			b_white;

public:
	GraphicsPrefsPanel(wxWindow* parent);
	~GraphicsPrefsPanel();

	void	applyPreferences();

	// Events
	void	onButtonRClicked(wxCommandEvent& e);
	void	onButtonCClicked(wxCommandEvent& e);
	void	onButtonMClicked(wxCommandEvent& e);
	void	onButtonBClicked(wxCommandEvent& e);
	void	onButtonWClicked(wxCommandEvent& e);
};

#endif//__GFX_PREFS_PANEL_H__
