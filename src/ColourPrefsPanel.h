
#ifndef __COL_PREFS_PANEL_H__
#define __COL_PREFS_PANEL_H__
#include <wx/clrpicker.h>
#include <wx/button.h>
#include <wx/propgrid/propgrid.h>

class ColourPrefsPanel : public wxPanel {
private:
	wxChoice*			choice_configs;
	wxButton*			btn_saveconfig;
	wxPropertyGrid*		pg_colours;

public:
	ColourPrefsPanel(wxWindow* parent);
	~ColourPrefsPanel();

	void	refreshPropGrid();
	void	applyPreferences();

	// Events
	void	onChoicePresetSelected(wxCommandEvent& e);
};

#endif//__COL_PREFS_PANEL_H__
