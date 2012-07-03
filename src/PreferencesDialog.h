
#ifndef __PREFERENCES_DIALOG_H__
#define __PREFERENCES_DIALOG_H__

#include <wx/treebook.h>

class PrefsPanelBase;
class BaseResourceArchivesPanel;
class PreferencesDialog : public wxDialog {
private:
	wxTreebook*				tree_prefs;
	vector<PrefsPanelBase*>	prefs_pages;

	// Base Resource Archive
	BaseResourceArchivesPanel*	panel_bra;
	wxButton*					btn_bra_open;

	// Singleton instance
	static PreferencesDialog*	instance;

public:
	PreferencesDialog(wxWindow* parent);
	~PreferencesDialog();

	wxPanel*	setupEditingPrefsPanel();
	wxPanel*	setupBaseResourceArchivesPanel();

	void	showPage(string name);
	void	applyPreferences();

	// Events
	void	onBtnBRAOpenClicked(wxCommandEvent& e);
	void	onButtonClicked(wxCommandEvent& e);

	// Static functions
	static void	openPreferences(wxWindow* parent);
};

#endif//__PREFERENCES_DIALOG_H__
