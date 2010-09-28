
#ifndef __PREFERENCES_DIALOG_H__
#define __PREFERENCES_DIALOG_H__

#include "BaseResourceArchivesPanel.h"
#include "TextEditorPrefsPanel.h"
#include "TextStylePrefsPanel.h"
#include "GeneralPrefsPanel.h"
#include "EditingPrefsPanel.h"
#include <wx/treebook.h>

class PreferencesDialog : public wxDialog {
private:
	wxTreebook*	tree_prefs;

	GeneralPrefsPanel*		panel_general;
	EditingPrefsPanel*		panel_editing;
	TextEditorPrefsPanel*	panel_text_editor;
	TextStylePrefsPanel*	panel_text_styles;

	// Base Resource Archive
	BaseResourceArchivesPanel*	panel_bra;
	wxButton*					btn_bra_open;

public:
	PreferencesDialog(wxWindow* parent);
	~PreferencesDialog();

	wxPanel*	setupEditingPrefsPanel();
	wxPanel*	setupBaseResourceArchivesPanel();

	void	applyPreferences();

	// Events
	void	onBtnBRAOpenClicked(wxCommandEvent& e);
};

#endif//__PREFERENCES_DIALOG_H__
