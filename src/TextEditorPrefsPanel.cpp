
#include "Main.h"
#include "WxStuff.h"
#include "TextEditorPrefsPanel.h"
#include <wx/statline.h>

EXTERN_CVAR(Int, txed_tab_width)
EXTERN_CVAR(Bool, txed_auto_indent)
EXTERN_CVAR(Bool, txed_syntax_hilight)
EXTERN_CVAR(Bool, txed_brace_match)
EXTERN_CVAR(Int, txed_edge_column)
EXTERN_CVAR(Bool, txed_indent_guides)
EXTERN_CVAR(String, txed_style_set)
EXTERN_CVAR(Bool, txed_trim_whitespace)

TextEditorPrefsPanel::TextEditorPrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create main sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Text Editor Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Tab width
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	spin_tab_width = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, txed_tab_width);
	hbox->Add(new wxStaticText(this, -1, "Tab Indentation Width: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	hbox->Add(spin_tab_width, 1, wxEXPAND);
	sizer->Add(hbox, 0, wxALL|wxEXPAND, 4);

	// Auto indent
	cb_auto_indent = new wxCheckBox(this, -1, "Enable Auto-Indent");
	sizer->Add(cb_auto_indent, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Trim whitespace
	cb_trim_whitespace = new wxCheckBox(this, -1, "Trim Whitespace on Save");
	sizer->Add(cb_trim_whitespace, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Separator
	sizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);

	// Right margin
	hbox = new wxBoxSizer(wxHORIZONTAL);
	spin_right_margin = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1000, txed_edge_column);
	hbox->Add(new wxStaticText(this, -1, "Right Margin at Column: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	hbox->Add(spin_right_margin, 1, wxEXPAND);
	sizer->Add(hbox, 0, wxALL|wxEXPAND, 4);

	// Syntax Hilighting
	cb_syntax_hilight = new wxCheckBox(this, -1, "Show Syntax Hilighting");
	sizer->Add(cb_syntax_hilight, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Indentation guides
	cb_indent_guides = new wxCheckBox(this, -1, "Show Indentation Guides");
	sizer->Add(cb_indent_guides, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Brace matching
	cb_brace_match = new wxCheckBox(this, -1, "Hilight Matching Braces");
	sizer->Add(cb_brace_match, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Init controls
	cb_auto_indent->SetValue(txed_auto_indent);
	cb_trim_whitespace->SetValue(txed_trim_whitespace);
	cb_syntax_hilight->SetValue(txed_syntax_hilight);
	cb_indent_guides->SetValue(txed_indent_guides);
	cb_brace_match->SetValue(txed_brace_match);
}

TextEditorPrefsPanel::~TextEditorPrefsPanel() {
}

void TextEditorPrefsPanel::applyPreferences() {
	txed_auto_indent = cb_auto_indent->GetValue();
	txed_trim_whitespace = cb_trim_whitespace->GetValue();
	txed_syntax_hilight = cb_syntax_hilight->GetValue();
	txed_indent_guides = cb_indent_guides->GetValue();
	txed_brace_match = cb_brace_match->GetValue();
	txed_tab_width = spin_tab_width->GetValue();
	txed_edge_column = spin_right_margin->GetValue();
}
