
#ifndef __ZTEXTURE_EDITOR_PANEL_H__
#define __ZTEXTURE_EDITOR_PANEL_H__

#include "TextureEditorPanel.h"

class ZTextureEditorPanel : public TextureEditorPanel {
private:
	wxSpinCtrlDouble*	spin_tex_scalex;
	wxSpinCtrlDouble*	spin_tex_scaley;
	wxSpinCtrl*			spin_tex_offsetx;
	wxSpinCtrl*			spin_tex_offsety;
	wxChoice*			choice_type;
	wxCheckBox*			cb_optional;
	wxCheckBox*			cb_worldpanning;
	wxCheckBox*			cb_nodecals;
	wxCheckBox*			cb_nulltexture;

public:
	ZTextureEditorPanel(wxWindow* parent, TextureXEditor* tx_editor);
	~ZTextureEditorPanel();

	// UI Stuff
	wxPanel*	createTextureControls(wxWindow* parent);
	void		updateTextureControls();
	//wxPanel*	createPatchControls(wxWindow* parent);
	//void		populatePatchList();
	//void		updatePatchControls();

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
