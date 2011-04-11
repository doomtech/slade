
#ifndef __INTERFACE_PREFS_PANEL_H__
#define __INTERFACE_PREFS_PANEL_H__

class InterfacePrefsPanel : public wxPanel {
private:
	wxCheckBox*	cb_size_as_string;
	wxCheckBox* cb_filter_dirs;
	wxCheckBox*	cb_start_page;
	wxCheckBox*	cb_context_submenus;
	wxCheckBox*	cb_swap_epanel_bars;

public:
	InterfacePrefsPanel(wxWindow* parent);
	~InterfacePrefsPanel();

	void	applyPreferences();
};

#endif//__INTERFACE_PREFS_PANEL_H__
