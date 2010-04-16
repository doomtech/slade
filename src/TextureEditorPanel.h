
#ifndef __TEXTURE_EDITOR_PANEL_H__
#define __TEXTURE_EDITOR_PANEL_H__

#include "CTextureCanvas.h"
#include "PatchTable.h"
#include "ListView.h"
#include <wx/spinctrl.h>

class TextureEditorPanel : public wxPanel {
private:
	PatchTable*		patch_table;

	// View controls
	wxSlider*		slider_zoom;
	wxStaticText*	label_current_zoom;
	wxCheckBox*		cb_draw_outside;
	CTextureCanvas*	tex_canvas;

	// Texture controls
	wxTextCtrl*		text_tex_name;
	wxSpinCtrl*		spin_tex_width;
	wxSpinCtrl*		spin_tex_height;
	wxSpinCtrl*		spin_tex_scalex;
	wxSpinCtrl*		spin_tex_scaley;
	wxStaticText*	label_scaled_size;

	// Texture patches list + related controls
	ListView*		list_patches;
	wxButton*		btn_patch_add;
	wxButton*		btn_patch_remove;
	wxButton*		btn_patch_back;
	wxButton*		btn_patch_forward;
	wxButton*		btn_patch_replace;
	wxButton*		btn_patch_duplicate;

	// Patch controls
	wxSpinCtrl*		spin_patch_left;
	wxSpinCtrl*		spin_patch_top;

public:
	TextureEditorPanel(wxWindow* parent, PatchTable* patch_table);
	~TextureEditorPanel();

	// UI Stuff
	wxPanel*	createTextureControls(wxWindow* parent);
	void		updateTextureControls();
	void		updateTextureScaleLabel();
	wxPanel*	createPatchControls(wxWindow* parent);
	void		populatePatchList();
	void		updatePatchControls();

	bool	openTexture(tx_texture_t& tex);
	void	setPalette(Palette8bit* pal);

	// Events
	void	onZoomChanged(wxCommandEvent& e);
	void	onDrawOutsideChanged(wxCommandEvent& e);
	void	onTexCanvasMouseEvent(wxMouseEvent& e);
	void	onTexCanvasDragEnd(wxCommandEvent& e);
	void	onTexNameChanged(wxCommandEvent& e);
	void	onTexWidthChanged(wxSpinEvent& e);
	void	onTexHeightChanged(wxSpinEvent& e);
	void	onTexScaleXChanged(wxSpinEvent& e);
	void	onTexScaleYChanged(wxSpinEvent& e);
	void	onPatchListSelect(wxListEvent& e);
	void	onPatchListDeSelect(wxListEvent& e);
	void	onBtnPatchBack(wxCommandEvent& e);
	void	onBtnPatchForward(wxCommandEvent& e);
	void	onPatchPositionXChanged(wxSpinEvent& e);
	void	onPatchPositionYChanged(wxSpinEvent& e);
};

#endif//__TEXTURE_EDITOR_PANEL_H__
