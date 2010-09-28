
#ifndef __TEXT_STYLE_PREFS_PANEL_H__
#define __TEXT_STYLE_PREFS_PANEL_H__

#include <wx/clrpicker.h>
#include <wx/fontpicker.h>

class TextStylePrefsPanel : public wxPanel {
private:
	wxChoice*			choice_styleset;
	wxButton*			btn_savestyleset;
	wxListBox*			list_styles;

	wxColourPickerCtrl*	cp_foreground;
	wxColourPickerCtrl*	cp_background;
	wxFontPickerCtrl*	fp_font;
	wxCheckBox*			cb_override_font_face;
	wxCheckBox*			cb_override_font_size;
	wxCheckBox*			cb_override_font_bold;
	wxCheckBox*			cb_override_font_italic;
	wxCheckBox*			cb_override_font_underlined;
	wxCheckBox*			cb_override_foreground;
	wxCheckBox*			cb_override_background;

public:
	TextStylePrefsPanel(wxWindow* parent);
	~TextStylePrefsPanel();

	void	onStyleSelected(wxCommandEvent& e);
};

#endif//__TEXT_STYLE_PREFS_PANEL_H__
