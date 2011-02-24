
#ifndef __ZTEXTURE_EDITOR_PANEL_H__
#define __ZTEXTURE_EDITOR_PANEL_H__

#include "TextureEditorPanel.h"
#include <wx/clrpicker.h>

class ZTextureEditorPanel : public TextureEditorPanel {
private:
	// Texture properties
	wxSpinCtrlDouble*	spin_tex_scalex;
	wxSpinCtrlDouble*	spin_tex_scaley;
	wxSpinCtrl*			spin_tex_offsetx;
	wxSpinCtrl*			spin_tex_offsety;
	wxChoice*			choice_type;
	wxCheckBox*			cb_optional;
	wxCheckBox*			cb_worldpanning;
	wxCheckBox*			cb_nodecals;
	wxCheckBox*			cb_nulltexture;

	// Patch properties
	wxCheckBox*			cb_flipx;
	wxCheckBox*			cb_flipy;
	wxChoice*			choice_rotation;
	wxSpinCtrlDouble*	spin_alpha;
	wxChoice*			choice_style;

	wxRadioButton*		rb_pc_normal;
	wxRadioButton*		rb_pc_translation;
	wxRadioButton*		rb_pc_blend;
	wxRadioButton*		rb_pc_tint;
	wxColourPickerCtrl*	cp_blend_col;
	wxSpinCtrlDouble*	spin_tint_amount;
	wxTextCtrl*			text_translation;
	wxButton*			btn_edit_translation;

public:
	ZTextureEditorPanel(wxWindow* parent, TextureXEditor* tx_editor);
	~ZTextureEditorPanel();

	// UI Stuff
	wxPanel*	createTextureControls(wxWindow* parent);
	void		updateTextureControls();
	wxPanel*	createPatchControls(wxWindow* parent);
	//void		populatePatchList();
	void		updatePatchControls();

	void	enableTranslationControls(bool enable) { text_translation->Enable(enable); btn_edit_translation->Enable(enable); }
	void	enableBlendControls(bool enable, bool tint = false) { cp_blend_col->Enable(enable); spin_tint_amount->Enable(enable && tint); }

	// Events
	void	onTexScaleXChanged(wxSpinDoubleEvent& e);
	void	onTexScaleYChanged(wxSpinDoubleEvent& e);
	void	onTexOffsetXChanged(wxSpinEvent& e);
	void	onTexOffsetYChanged(wxSpinEvent& e);
	void	onTexTypeChanged(wxCommandEvent& e);
	void	onTexOptionalChanged(wxCommandEvent& e);
	void	onTexWorldPanningChanged(wxCommandEvent& e);
	void	onTexNoDecalsChanged(wxCommandEvent& e);
	void	onTexNullTextureChanged(wxCommandEvent& e);
};

#endif//__ZTEXTURE_EDITOR_PANEL_H__
