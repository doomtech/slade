
#include "Main.h"
#include "WxStuff.h"
#include "TextStylePrefsPanel.h"
#include "TextStyle.h"
#include <wx/gbsizer.h>
#include <wx/statline.h>

TextStylePrefsPanel::TextStylePrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
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

	// 'Save' styleset button
	btn_savestyleset = new wxButton(this, -1, "Save");
	hbox->Add(btn_savestyleset, 0, wxEXPAND);


	// Style configuration controls
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 1, wxEXPAND|wxALL, 4);

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
	hbox->Add(list_styles, 0, wxEXPAND|wxALL, 4);

	// Style settings
	wxGridBagSizer* sizer_gb = new wxGridBagSizer(4, 4);
	hbox->Add(sizer_gb, 1, wxEXPAND|wxALL, 4);

	// Font
	sizer_gb->Add(new wxStaticText(this, -1, "Font:"), wxGBPosition(0, 0));
	fp_font = new wxFontPickerCtrl(this, -1);
	sizer_gb->Add(fp_font, wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);

	cb_font_default = new wxCheckBox(this, -1, "");
	sizer_gb->Add(new wxStaticText(this, -1, "Use Default"), wxGBPosition(0, 1), wxDefaultSpan, wxALIGN_CENTER);
	sizer_gb->Add(cb_font_default, wxGBPosition(1, 1), wxDefaultSpan, wxALIGN_CENTER);
	sizer_gb->AddGrowableCol(0);

	// Bold
	cb_bold = new wxCheckBox(this, -1, "Bold");
	sizer_gb->Add(cb_bold, wxGBPosition(2, 0));
	cb_bold_default = new wxCheckBox(this, -1, "");
	sizer_gb->Add(cb_bold_default, wxGBPosition(2, 1), wxDefaultSpan, wxALIGN_CENTER);

	// Italic
	cb_italic = new wxCheckBox(this, -1, "Italic");
	sizer_gb->Add(cb_italic, wxGBPosition(3, 0));
	cb_italic_default = new wxCheckBox(this, -1, "");
	sizer_gb->Add(cb_italic_default, wxGBPosition(3, 1), wxDefaultSpan, wxALIGN_CENTER);

	// Underlined
	cb_underlined = new wxCheckBox(this, -1, "Underlined");
	sizer_gb->Add(cb_underlined, wxGBPosition(4, 0));
	cb_underlined_default = new wxCheckBox(this, -1, "");
	sizer_gb->Add(cb_underlined_default, wxGBPosition(4, 1), wxDefaultSpan, wxALIGN_CENTER);

	// Separator
	sizer_gb->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), wxGBPosition(5, 0), wxGBSpan(1, 2), wxEXPAND);

	// Foreground Colour
	sizer_gb->Add(new wxStaticText(this, -1, "Foreground Colour:"), wxGBPosition(6, 0), wxGBSpan(1, 2));
	cp_foreground = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	sizer_gb->Add(cp_foreground, wxGBPosition(7, 0));
	cb_foreground_default = new wxCheckBox(this, -1, "");
	sizer_gb->Add(cb_foreground_default, wxGBPosition(7, 1), wxDefaultSpan, wxALIGN_CENTER);

	// Background Colour
	sizer_gb->Add(new wxStaticText(this, -1, "Background Colour:"), wxGBPosition(8, 0), wxGBSpan(1, 2));
	cp_background = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	sizer_gb->Add(cp_background, wxGBPosition(9, 0));
	cb_background_default = new wxCheckBox(this, -1, "");
	sizer_gb->Add(cb_background_default, wxGBPosition(9, 1), wxDefaultSpan, wxALIGN_CENTER);


	// Bind events
	list_styles->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &TextStylePrefsPanel::onStyleSelected, this);
	cb_font_default->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBFontDefaultChanged, this);
	cb_bold_default->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBBoldDefaultChanged, this);
	cb_italic_default->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBItalicDefaultChanged, this);
	cb_underlined_default->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBUnderlinedDefaultChanged, this);
	cb_foreground_default->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBForegroundDefaultChanged, this);
	cb_background_default->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextStylePrefsPanel::onCBBackgroundDefaultChanged, this);

	// Select default style
	list_styles->SetSelection(0);
}

TextStylePrefsPanel::~TextStylePrefsPanel() {
}



void TextStylePrefsPanel::onStyleSelected(wxCommandEvent& e) {
	StyleSet* currentset = StyleSet::currentSet();

	// Check if something went wrong
	if (!currentset)
		return;

	// Get style from current set depending on selection
	TextStyle* style;
	TextStyle* style_default = currentset->getStyle("default");
	switch (list_styles->GetSelection()) {
	case 0:
		style = style_default;
		break;
	case 1:
		style = currentset->getStyle("preprocessor");
		break;
	case 2:
		style = currentset->getStyle("comment");
		break;
	case 3:
		style = currentset->getStyle("string");
		break;
	case 4:
		style = currentset->getStyle("character");
		break;
	case 5:
		style = currentset->getStyle("keyword");
		break;
	case 6:
		style = currentset->getStyle("constant");
		break;
	case 7:
		style = currentset->getStyle("function");
		break;
	case 8:
		style = currentset->getStyle("bracematch");
		break;
	case 9:
		style = currentset->getStyle("bracebad");
		break;
	default:
		return;
	}

	// Check if something went wrong
	if (!style)
		return;

	// If default style was selected, disable 'use default' checkboxes
	if (list_styles->GetSelection() == 0) {
		cb_background_default->Enable(false);
		cb_bold_default->Enable(false);
		cb_font_default->Enable(false);
		cb_foreground_default->Enable(false);
		cb_italic_default->Enable(false);
		cb_underlined_default->Enable(false);
	}
	else {
		cb_background_default->Enable(true);
		cb_bold_default->Enable(true);
		cb_font_default->Enable(true);
		cb_foreground_default->Enable(true);
		cb_italic_default->Enable(true);
		cb_underlined_default->Enable(true);
	}


	// Update font face+size
	string fontface = style->getFontFace();
	if (fontface.IsEmpty()) {
		fontface = style_default->getFontFace();
		cb_font_default->SetValue(true);
		fp_font->Enable(false);
	}
	else {
		cb_font_default->SetValue(false);
		fp_font->Enable(true);
	}

	int fontsize = style->getFontSize();
	if (fontsize <= 0)
		fontsize = style_default->getFontSize();

	wxFont font;
	font.SetFaceName(fontface);
	font.SetPointSize(fontsize);
	fp_font->SetSelectedFont(font);


	// Update bold checkbox
	int bold = style->getBold();
	if (bold < 0) {
		bold = style_default->getBold();
		cb_bold_default->SetValue(true);
		cb_bold->Enable(false);
	}
	else {
		cb_bold_default->SetValue(false);
		cb_bold->Enable(true);
	}

	cb_bold->SetValue((bool)bold);


	// Update italic checkbox
	int italic = style->getItalic();
	if (italic < 0) {
		italic = style_default->getItalic();
		cb_italic_default->SetValue(true);
		cb_italic->Enable(false);
	}
	else {
		cb_italic_default->SetValue(false);
		cb_italic->Enable(true);
	}

	cb_italic->SetValue((bool)italic);


	// Update underlined checkbox
	int underlined = style->getUnderlined();
	if (underlined < 0) {
		underlined = style_default->getUnderlined();
		cb_underlined_default->SetValue(true);
		cb_underlined->Enable(false);
	}
	else {
		cb_underlined_default->SetValue(false);
		cb_underlined->Enable(true);
	}

	cb_underlined->SetValue((bool)underlined);


	// Update foreground colour
	rgba_t fgcol;
	if (style->hasForeground()) {
		cb_foreground_default->SetValue(false);
		fgcol = style->getForeground();
		cp_foreground->Enable(true);
	}
	else {
		cb_foreground_default->SetValue(true);
		fgcol = style_default->getForeground();
		cp_foreground->Enable(false);
	}
	cp_foreground->SetColour(WXCOL(fgcol));


	// Update background colour
	rgba_t bgcol;
	if (style->hasBackground()) {
		cb_background_default->SetValue(false);
		bgcol = style->getBackground();
		cp_background->Enable(true);
	}
	else {
		cb_background_default->SetValue(true);
		bgcol = style_default->getBackground();
		cp_background->Enable(false);
	}
	cp_background->SetColour(WXCOL(bgcol));
}

void TextStylePrefsPanel::onCBFontDefaultChanged(wxCommandEvent& e) {
	fp_font->Enable(!cb_font_default->GetValue());
}

void TextStylePrefsPanel::onCBBoldDefaultChanged(wxCommandEvent& e) {
	cb_bold->Enable(!cb_bold_default->GetValue());
}

void TextStylePrefsPanel::onCBItalicDefaultChanged(wxCommandEvent& e) {
	cb_italic->Enable(!cb_italic_default->GetValue());
}

void TextStylePrefsPanel::onCBUnderlinedDefaultChanged(wxCommandEvent& e) {
	cb_underlined->Enable(!cb_underlined_default->GetValue());
}

void TextStylePrefsPanel::onCBForegroundDefaultChanged(wxCommandEvent& e) {
	cp_foreground->Enable(!cb_foreground_default->GetValue());
}

void TextStylePrefsPanel::onCBBackgroundDefaultChanged(wxCommandEvent& e) {
	cp_background->Enable(!cb_background_default->GetValue());
}
