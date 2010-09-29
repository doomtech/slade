
#include "Main.h"
#include "WxStuff.h"
#include "TextStylePrefsPanel.h"
#include <wx/gbsizer.h>
#include <wx/statline.h>

TextStylePrefsPanel::TextStylePrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Init variables
	ss_current.copySet(StyleSet::currentSet());
	ts_current = ss_current.getStyle("default");

	// Create main sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Text Editor Fonts and Colours");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);


	// Styleset controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

	// Styleset selection dropdown
	wxArrayString style_sets;
	for (unsigned a = 0; a < StyleSet::numSets(); a++)
		style_sets.Add(StyleSet::getName(a));
	choice_styleset = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, style_sets);
	hbox->Add(new wxStaticText(this, -1, "Style Set:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(choice_styleset, 1, wxEXPAND|wxRIGHT, 4);


	// Style configuration controls
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 1, wxEXPAND|wxALL, 4);
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	hbox->Add(vbox, 0, wxEXPAND|wxALL, 4);

	// Style list
	wxArrayString style_names;
	style_names.Add("Default");
	style_names.Add("Preprocessor");
	style_names.Add("Comment");
	style_names.Add("String");
	style_names.Add("Character");
	style_names.Add("Keyword");
	style_names.Add("Constant");
	style_names.Add("Function");
	style_names.Add("Brace Match");
	style_names.Add("Brace Mismatch");
	list_styles = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize, style_names);
	vbox->Add(list_styles, 1, wxEXPAND|wxBOTTOM, 4);

	// 'Save Set' button
	btn_savestyleset = new wxButton(this, -1, "Save Set");
	vbox->Add(btn_savestyleset, 0, wxEXPAND);


	// Style settings
	vbox = new wxBoxSizer(wxVERTICAL);
	hbox->Add(vbox, 1, wxEXPAND|wxALL, 4);

	// Font picker
	vbox->Add(new wxStaticText(this, -1, "Font:"), 0, wxBOTTOM, 2);
	fp_font = new wxFontPickerCtrl(this, -1);
	vbox->Add(fp_font, 0, wxEXPAND|wxBOTTOM, 4);

	// Font property overrides
	wxGridBagSizer* sizer_gb = new wxGridBagSizer(4, 4);
	vbox->Add(sizer_gb, 0, wxEXPAND|wxBOTTOM, 4);
	sizer_gb->Add(new wxStaticText(this, -1, "Override default font properties:"), wxGBPosition(0, 0), wxGBSpan(1, 3));
	cb_override_font_face = new wxCheckBox(this, -1, "Face");
	cb_override_font_size = new wxCheckBox(this, -1, "Size");
	cb_override_font_bold = new wxCheckBox(this, -1, "Bold");
	cb_override_font_italic = new wxCheckBox(this, -1, "Italic");
	cb_override_font_underlined = new wxCheckBox(this, -1, "Underlined");
	sizer_gb->Add(cb_override_font_face, wxGBPosition(1, 0));
	sizer_gb->Add(cb_override_font_size, wxGBPosition(1, 1));
	sizer_gb->Add(cb_override_font_bold, wxGBPosition(2, 0));
	sizer_gb->Add(cb_override_font_italic, wxGBPosition(2, 1));
	sizer_gb->Add(cb_override_font_underlined, wxGBPosition(2, 2));

	// Foreground colour
	vbox->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(new wxStaticText(this, -1, "Foreground:"), 1, wxALIGN_CENTER_VERTICAL);
	cb_override_foreground = new wxCheckBox(this, -1, "Override Default");
	hbox->Add(cb_override_foreground, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
	cp_foreground = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	vbox->Add(cp_foreground, 0, wxEXPAND|wxBOTTOM, 4);

	// Background colour
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(new wxStaticText(this, -1, "Background:"), 1, wxALIGN_CENTER_VERTICAL);
	cb_override_background = new wxCheckBox(this, -1, "Override Default");
	hbox->Add(cb_override_background, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
	cp_background = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	vbox->Add(cp_background, 0, wxEXPAND);


	// Bind events
	list_styles->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &TextStylePrefsPanel::onStyleSelected, this);
	cb_override_font_face->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBOverrideFontFace, this);
	cb_override_font_size->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBOverrideFontSize, this);
	cb_override_font_bold->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBOverrideFontBold, this);
	cb_override_font_italic->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBOverrideFontItalic, this);
	cb_override_font_underlined->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBOverrideFontUnderlined, this);
	cb_override_foreground->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBOverrideForeground, this);
	cb_override_background->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBOverrideBackground, this);
	fp_font->Bind(wxEVT_COMMAND_FONTPICKER_CHANGED, &TextStylePrefsPanel::onFontChanged, this);
	cp_foreground->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &TextStylePrefsPanel::onForegroundChanged, this);
	cp_background->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &TextStylePrefsPanel::onBackgroundChanged, this);
	btn_savestyleset->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextStylePrefsPanel::onBtnSaveStyleSet, this);
	choice_styleset->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &TextStylePrefsPanel::onStyleSetSelected, this);

	// Select default style
	list_styles->SetSelection(0);
	updateStyleControls();
}

TextStylePrefsPanel::~TextStylePrefsPanel() {
}

void TextStylePrefsPanel::updateStyleControls() {
	if (!ts_current)
		return;

	// Get default style
	TextStyle* style_default = ss_current.getStyle("default");

	// Reset UI stuff
	cb_override_font_face->SetValue(true);
	cb_override_font_size->SetValue(true);
	cb_override_font_bold->SetValue(true);
	cb_override_font_italic->SetValue(true);
	cb_override_font_underlined->SetValue(true);
	cb_override_foreground->SetValue(true);
	cb_override_background->SetValue(true);

	// Disable override checkboxes if default style
	bool enable = (ts_current != style_default);
	cb_override_font_face->Enable(enable);
	cb_override_font_size->Enable(enable);
	cb_override_font_bold->Enable(enable);
	cb_override_font_italic->Enable(enable);
	cb_override_font_underlined->Enable(enable);
	cb_override_foreground->Enable(enable);
	cb_override_background->Enable(enable);

	// Update style properties
	wxFont font = fp_font->GetSelectedFont();

	// Font face
	string font_face = ts_current->getFontFace();
	if (font_face.IsEmpty()) {
		font_face = style_default->getFontFace();
		cb_override_font_face->SetValue(false);
	}
	font.SetFaceName(font_face);

	// Font size
	int font_size = ts_current->getFontSize();
	if (font_size <= 0) {
		font_size = style_default->getFontSize();
		cb_override_font_size->SetValue(false);
	}
	font.SetPointSize(font_size);

	// Bold
	bool bold = false;
	if (ts_current->getBold() > 0)
		bold = true;
	else if (ts_current->getBold() < 0) {
		bold = !!style_default->getBold();
		cb_override_font_bold->SetValue(false);
	}
	if (bold)
		font.SetWeight(wxFONTWEIGHT_BOLD);
	else
		font.SetWeight(wxFONTWEIGHT_NORMAL);

	// Italic
	bool italic = false;
	if (ts_current->getItalic() > 0)
		italic = true;
	else if (ts_current->getItalic() < 0) {
		italic = !!style_default->getItalic();
		cb_override_font_italic->SetValue(false);
	}
	if (italic)
		font.SetStyle(wxFONTSTYLE_ITALIC);
	else
		font.SetStyle(wxFONTSTYLE_NORMAL);

	// Underlined
	bool underlined = false;
	if (ts_current->getUnderlined() > 0)
		underlined = true;
	else if (ts_current->getUnderlined() < 0) {
		underlined = !!style_default->getUnderlined();
		cb_override_font_underlined->SetValue(false);
	}
	font.SetUnderlined(underlined);

	// Foreground
	rgba_t col_foreground;
	if (ts_current->hasForeground())
		col_foreground.set(ts_current->getForeground());
	else {
		col_foreground.set(style_default->getForeground());
		cb_override_foreground->SetValue(false);
	}
	cp_foreground->SetColour(WXCOL(col_foreground));

	// Background
	rgba_t col_background;
	if (ts_current->hasBackground())
		col_background.set(ts_current->getBackground());
	else {
		col_background.set(style_default->getBackground());
		cb_override_background->SetValue(false);
	}
	cp_background->SetColour(WXCOL(col_background));

	// Apply font
	fp_font->SetSelectedFont(font);
}

void TextStylePrefsPanel::updateFontFace() {
	// Update current style
	if (cb_override_font_face->GetValue())
		ts_current->setFontFace(fp_font->GetSelectedFont().GetFaceName());
	else
		ts_current->setFontFace("");
}

void TextStylePrefsPanel::updateFontSize() {
	if (cb_override_font_size->GetValue())
		ts_current->setFontSize(fp_font->GetSelectedFont().GetPointSize());
	else
		ts_current->setFontSize(-1);
}

void TextStylePrefsPanel::updateFontBold() {
	if (cb_override_font_bold->GetValue()) {
		wxFont font = fp_font->GetSelectedFont();
		if (font.GetWeight() == wxFONTWEIGHT_BOLD)
			ts_current->setBold(1);
		else
			ts_current->setBold(0);
	}
	else
		ts_current->setBold(-1);
}

void TextStylePrefsPanel::updateFontItalic() {
	if (cb_override_font_italic->GetValue()) {
		wxFont font = fp_font->GetSelectedFont();
		if (font.GetStyle() == wxFONTSTYLE_ITALIC)
			ts_current->setItalic(1);
		else
			ts_current->setItalic(0);
	}
	else
		ts_current->setItalic(-1);
}

void TextStylePrefsPanel::updateFontUnderlined() {
	if (cb_override_font_underlined->GetValue()) {
		wxFont font = fp_font->GetSelectedFont();
		if (font.GetUnderlined())
			ts_current->setUnderlined(1);
		else
			ts_current->setUnderlined(0);
	}
	else
		ts_current->setUnderlined(-1);
}

void TextStylePrefsPanel::updateForeground() {
	if (cb_override_foreground->GetValue()) {
		wxColour wxc = cp_foreground->GetColour();
		ts_current->setForeground(rgba_t(wxc.Red(), wxc.Green(), wxc.Blue(), 255));
	}
	else {
		ts_current->clearForeground();
	}
}

void TextStylePrefsPanel::updateBackground() {
	if (cb_override_background->GetValue()) {
		wxColour wxc = cp_background->GetColour();
		ts_current->setBackground(rgba_t(wxc.Red(), wxc.Green(), wxc.Blue(), 255));
	}
	else {
		ts_current->clearBackground();
	}
}

void TextStylePrefsPanel::applyPreferences() {
	// Apply styleset to global current
	StyleSet::currentSet()->copySet(&ss_current);
}




void TextStylePrefsPanel::onStyleSelected(wxCommandEvent& e) {
	// Set style from current set depending on selection
	switch (list_styles->GetSelection()) {
	case 0:
		ts_current = ss_current.getStyle("default");
		break;
	case 1:
		ts_current = ss_current.getStyle("preprocessor");
		break;
	case 2:
		ts_current = ss_current.getStyle("comment");
		break;
	case 3:
		ts_current = ss_current.getStyle("string");
		break;
	case 4:
		ts_current = ss_current.getStyle("character");
		break;
	case 5:
		ts_current = ss_current.getStyle("keyword");
		break;
	case 6:
		ts_current = ss_current.getStyle("constant");
		break;
	case 7:
		ts_current = ss_current.getStyle("function");
		break;
	case 8:
		ts_current = ss_current.getStyle("bracematch");
		break;
	case 9:
		ts_current = ss_current.getStyle("bracebad");
		break;
	default:
		return;
	}

	updateStyleControls();
}

void TextStylePrefsPanel::onCBOverrideFontFace(wxCommandEvent& e) {
	updateFontFace();
}

void TextStylePrefsPanel::onCBOverrideFontSize(wxCommandEvent& e) {
	updateFontSize();
}

void TextStylePrefsPanel::onCBOverrideFontBold(wxCommandEvent& e) {
	updateFontBold();
}

void TextStylePrefsPanel::onCBOverrideFontItalic(wxCommandEvent& e) {
	updateFontItalic();
}

void TextStylePrefsPanel::onCBOverrideFontUnderlined(wxCommandEvent& e) {
	updateFontUnderlined();
}

void TextStylePrefsPanel::onCBOverrideForeground(wxCommandEvent& e) {
	updateForeground();
}

void TextStylePrefsPanel::onCBOverrideBackground(wxCommandEvent& e) {
	updateBackground();
}

void TextStylePrefsPanel::onFontChanged(wxFontPickerEvent& e) {
	// Update relevant style properties
	updateFontFace();
	updateFontSize();
	updateFontBold();
	updateFontItalic();
	updateFontUnderlined();
}

void TextStylePrefsPanel::onForegroundChanged(wxColourPickerEvent& e) {
	updateForeground();
}

void TextStylePrefsPanel::onBackgroundChanged(wxColourPickerEvent& e) {
	updateBackground();
}

void TextStylePrefsPanel::onBtnSaveStyleSet(wxCommandEvent& e) {
	// Get name for set
	string name = wxGetTextFromUser("Enter Style Set name:", "Save Style Set");
	if (name.IsEmpty())
		return;

	// Create temp styleset
	StyleSet ss_temp(name);
	ss_temp.copySet(&ss_current);

	// Remove spaces from name (for filename)
	name.Replace(" ", "_");

	// Write set to file
	string filename = appPath(s_fmt("text_styles/%s.sss", chr(name)), DIR_USER);
	ss_temp.writeFile(filename);
}

void TextStylePrefsPanel::onStyleSetSelected(wxCommandEvent& e) {
	// Get selected styleset
	StyleSet* set = StyleSet::getSet(choice_styleset->GetSelection());
	if (set) {
		ss_current.copySet(set);
		updateStyleControls();
	}
}
