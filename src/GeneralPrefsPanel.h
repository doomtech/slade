
#ifndef __GENERAL_PREFS_PANEL_H__
#define __GENERAL_PREFS_PANEL_H__

class GeneralPrefsPanel : public wxPanel {
private:
	wxCheckBox*	cb_gl_np2;
	wxCheckBox*	cb_archive_load;
	wxCheckBox*	cb_archive_close_tab;
	wxCheckBox*	cb_size_as_string;
	wxCheckBox* cb_filter_dirs;
	wxCheckBox*	cb_start_page;
	wxCheckBox*	cb_temp_dir;

public:
	GeneralPrefsPanel(wxWindow* parent);
	~GeneralPrefsPanel();

	void	applyPreferences();
};

#endif//__GENERAL_PREFS_PANEL_H__
