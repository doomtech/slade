
#ifndef __TEXT_EDITOR_PREFS_PANEL_H__
#define __TEXT_EDITOR_PREFS_PANEL_H__

#include <wx/spinctrl.h>

class TextEditorPrefsPanel : public wxPanel {
private:
	wxSpinCtrl*	spin_tab_width;
	wxCheckBox*	cb_auto_indent;
	wxCheckBox*	cb_trim_whitespace;
	wxCheckBox*	cb_syntax_hilight;
	wxCheckBox*	cb_brace_match;
	wxSpinCtrl*	spin_right_margin;
	wxCheckBox*	cb_indent_guides;

public:
	TextEditorPrefsPanel(wxWindow* parent);
	~TextEditorPrefsPanel();

	void	applyPreferences();
};

#endif//__TEXT_EDITOR_PREFS_PANEL_H__
