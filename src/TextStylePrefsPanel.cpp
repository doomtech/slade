
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
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	hbox->Add(vbox, 1, wxEXPAND|wxALL, 4);

	// Font picker
	vbox->Add(new wxStaticText(this, -1, "Font:"), 0, wxBOTTOM, 2);
	fp_font = new wxFontPickerCtrl(this, -1);
	vbox->Add(fp_font, 0, wxEXPAND|wxBOTTOM, 4);

	// Font property overrides
	vbox->Add(new wxStaticText(this, -1, "Override default font properties:"), 0, wxBOTTOM, 4);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	cb_override_font_face = new wxCheckBox(this, -1, "Font Face");
	cb_override_font_size = new wxCheckBox(this, -1, "Font Size");
	hbox->Add(cb_override_font_face, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(cb_override_font_size, 0, wxEXPAND);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	cb_override_font_bold = new wxCheckBox(this, -1, "Bold");
	cb_override_font_italic = new wxCheckBox(this, -1, "Italic");
	cb_override_font_underlined = new wxCheckBox(this, -1, "Underlined");
	hbox->Add(cb_override_font_bold, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(cb_override_font_italic, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(cb_override_font_underlined, 0, wxEXPAND);


	// Foreground colour
	vbox->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(new wxStaticText(this, -1, "Foreground:"), 1, wxALIGN_CENTER_VERTICAL);
	cb_override_foreground = new wxCheckBox(this, -1, "Override Default");
	hbox->Add(cb_override_foreground, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
	cp_foreground = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	vbox->Add(cp_foreground, 0, wxEXPAND|wxBOTTOM, 8);

	// Background colour
	vbox->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(new wxStaticText(this, -1, "Background:"), 1, wxALIGN_CENTER_VERTICAL);
	cb_override_background = new wxCheckBox(this, -1, "Override Default");
	hbox->Add(cb_override_background, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
	cp_background = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	vbox->Add(cp_background, 0, wxEXPAND);


	// Bind events
	list_styles->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &TextStylePrefsPanel::onStyleSelected, this);

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
}
/*
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

	cb_bold->SetValue(!!bold);


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

	cb_italic->SetValue(!!italic);


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

	cb_underlined->SetValue(!!underlined);


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
*/
