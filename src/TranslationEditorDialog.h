
#ifndef __TRANSLATION_EDITOR_DIALOG_H__
#define __TRANSLATION_EDITOR_DIALOG_H__

#include <wx/dialog.h>
#include <wx/clrpicker.h>
#include "PaletteCanvas.h"
#include "Translation.h"
#include "GfxCanvas.h"

class GradientBox : public OGLCanvas
{
private:
	rgba_t	col_start;
	rgba_t	col_end;
	int		steps;

public:
	GradientBox(wxWindow* parent, int steps = -1);
	~GradientBox();

	void	setStartCol(rgba_t col) { col_start.set(col.r, col.g, col.b, 255); }
	void	setEndCol(rgba_t col) { col_end.set(col.r, col.g, col.b, 255); }
	void	setSteps(int steps) { this->steps = steps; }

	void	draw();
};

class ArchiveEntry;
class TranslationEditorDialog : public wxDialog
{
private:
	Palette8bit*	palette;
	Translation		translation;
	//ArchiveEntry*	entry_preview;
	SImage			image_preview;

	PaletteCanvas*	pal_canvas_original;
	wxListBox*		list_translations;
	wxButton*		btn_add;
	wxButton*		btn_remove;
	wxButton*		btn_up;
	wxButton*		btn_down;
	wxButton*		btn_load;
	wxButton*		btn_save;
	wxRadioButton*	rb_type_palette;
	wxRadioButton*	rb_type_colour;
	wxRadioButton*	rb_type_desaturate;
	wxTextCtrl*		text_string;

	// Paletted target range
	wxPanel*		panel_target_palette;
	PaletteCanvas*	pal_canvas_target;
	wxCheckBox*		cb_target_reverse;

	// Colour gradient target range
	wxPanel*			panel_target_gradient;
	wxColourPickerCtrl*	cp_range_begin;
	wxColourPickerCtrl*	cp_range_end;
	GradientBox*		gb_gradient;

	// Preview
	PaletteCanvas*		pal_canvas_preview;
	GfxCanvas*			gfx_preview;

public:
	TranslationEditorDialog(wxWindow* parent, Palette8bit* pal, string title = "Edit Translation", SImage* preview_image = NULL);
	~TranslationEditorDialog();

	Translation&	getTranslation() { return translation; }

	void	openTranslation(Translation& trans);
	void	openRange(int index);
	void	updateListItem(int index);
	void	setStartColour(rgba_t col);
	void	setEndColour(rgba_t col);
	void	showPaletteTarget();
	void	showGradientTarget();
	void	updatePreviews();

	// Events
	void	onSize(wxSizeEvent& e);
	void	onTranslationListItemSelected(wxCommandEvent& e);
	void	onRBPaletteSelected(wxCommandEvent& e);
	void	onRBColourSelected(wxCommandEvent& e);
	void	onRBDesaturateSelected(wxCommandEvent& e);
	void	onBeginColourChanged(wxColourPickerEvent& e);
	void	onEndColourChanged(wxColourPickerEvent& e);
	void	onPalOriginLeftUp(wxMouseEvent& e);
	void	onPalTargetLeftUp(wxMouseEvent& e);
	void	onBtnRemove(wxCommandEvent& e);
	void	onBtnAdd(wxCommandEvent& e);
	void	onBtnUp(wxCommandEvent& e);
	void	onBtnDown(wxCommandEvent& e);
	void	onBtnLoad(wxCommandEvent& e);
	void	onBtnSave(wxCommandEvent& e);
	void	onGfxPreviewMouseMotion(wxMouseEvent& e);
	void	onCBTargetReverse(wxCommandEvent& e);
};

#endif//__TRANSLATION_EDITOR_DIALOG_H__
