
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
	wxCheckBox*			cb_foreground_default;
	wxColourPickerCtrl*	cp_background;
	wxCheckBox*			cb_background_default;
	wxFontPickerCtrl*	fp_font;
	wxCheckBox*			cb_font_default;
	wxCheckBox*			cb_bold;
	wxCheckBox*			cb_bold_default;
	wxCheckBox*			cb_italic;
	wxCheckBox*			cb_italic_default;
	wxCheckBox*			cb_underlined;
	wxCheckBox*			cb_underlined_default;

public:
	TextStylePrefsPanel(wxWindow* parent);
	~TextStylePrefsPanel();

	void	onStyleSelected(wxCommandEvent& e);
	void	onCBFontDefaultChanged(wxCommandEvent& e);
	void	onCBBoldDefaultChanged(wxCommandEvent& e);
	void	onCBItalicDefaultChanged(wxCommandEvent& e);
	void	onCBUnderlinedDefaultChanged(wxCommandEvent& e);
	void	onCBForegroundDefaultChanged(wxCommandEvent& e);
	void	onCBBackgroundDefaultChanged(wxCommandEvent& e);
};

#endif//__TEXT_STYLE_PREFS_PANEL_H__
