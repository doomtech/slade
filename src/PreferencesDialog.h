
#ifndef __PREFERENCES_DIALOG_H__
#define __PREFERENCES_DIALOG_H__

#include "ColourPrefsPanel.h"
#include <wx/treebook.h>

class BaseResourceArchivesPanel;
class GeneralPrefsPanel;
class InterfacePrefsPanel;
class EditingPrefsPanel;
class TextEditorPrefsPanel;
class TextStylePrefsPanel;
class ACSPrefsPanel;
class GraphicsPrefsPanel;
class PNGPrefsPanel;
class AudioPrefsPanel;
class ColourPrefsPanel;
class MapEditorPrefsPanel;
class MapDisplayPrefsPanel;
class AdvancedPrefsPanel;

class PreferencesDialog : public wxDialog {
private:
	wxTreebook*	tree_prefs;

	GeneralPrefsPanel*		panel_general;
	InterfacePrefsPanel*	panel_interface;
	EditingPrefsPanel*		panel_editing;
	TextEditorPrefsPanel*	panel_text_editor;
	TextStylePrefsPanel*	panel_text_styles;
	ACSPrefsPanel*			panel_script_acs;
	GraphicsPrefsPanel*		panel_gfx_prefs;
	PNGPrefsPanel*			panel_gfx_png;
	AudioPrefsPanel*		panel_audio;
	ColourPrefsPanel*		panel_colours;
	MapEditorPrefsPanel*	panel_maped;
	MapDisplayPrefsPanel*	panel_map_display;
	AdvancedPrefsPanel*		panel_advanced;

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
	void	onButtonClicked(wxCommandEvent& e);
};

#endif//__PREFERENCES_DIALOG_H__
