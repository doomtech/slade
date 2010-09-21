
#ifndef __PREFERENCES_DIALOG_H__
#define __PREFERENCES_DIALOG_H__

#include "BaseResourceArchivesPanel.h"
#include "TextEditorPrefsPanel.h"
#include <wx/treebook.h>

class PreferencesDialog : public wxDialog {
private:
	wxTreebook*	tree_prefs;

	// General
	wxCheckBox*	cb_gl_np2;
	wxCheckBox*	cb_archive_load;
	wxCheckBox*	cb_archive_close_tab;
	wxCheckBox*	cb_size_as_string;

	// Editing
	wxCheckBox*	cb_wad_force_uppercase;
	wxCheckBox*	cb_wad_lock_iwad;
	wxChoice*	choice_entry_mod;

	// Base Resource Archive
	BaseResourceArchivesPanel*	panel_bra;
	wxButton*					btn_bra_open;

	// Text editor
	TextEditorPrefsPanel*	panel_text_editor;

public:
	PreferencesDialog(wxWindow* parent);
	~PreferencesDialog();

	wxPanel*	setupGeneralPrefsPanel();
	wxPanel*	setupEditingPrefsPanel();
	wxPanel*	setupBaseResourceArchivesPanel();

	void	applyPreferences();

	// Events
	void	onCBSASChanged(wxCommandEvent& e);
	void	onCBGLNP2Changed(wxCommandEvent& e);
	void	onCBArchiveLoadChanged(wxCommandEvent& e);
	void	onCBArchiveCloseTabChanged(wxCommandEvent& e);
	void	onCBWadForceUppercaseChanged(wxCommandEvent& e);
	void	onCBWadLockIWAD(wxCommandEvent& e);
	void	onChoiceEntryModChanged(wxCommandEvent& e);
	void	onBtnBRAOpenClicked(wxCommandEvent& e);
};

#endif//__PREFERENCES_DIALOG_H__
