
#ifndef __TEXTURE_EDITOR_PANEL_H__
#define __TEXTURE_EDITOR_PANEL_H__

#include "CTextureCanvas.h"
#include "PatchTable.h"
#include "ListView.h"
#include "MainApp.h"
#include <wx/spinctrl.h>

class TextureXEditor;

class TextureEditorPanel : public wxPanel, SActionHandler {
protected:
	TextureXEditor*	tx_editor;
	CTexture*		tex_current;
	bool			tex_modified;

	// View controls
	wxSlider*		slider_zoom;
	wxStaticText*	label_current_zoom;
	wxCheckBox*		cb_draw_outside;
	wxCheckBox*		cb_blend_rgba;
	wxStaticText*	label_viewtype;
	wxChoice*		choice_viewtype;
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
	TextureEditorPanel(wxWindow* parent, TextureXEditor* tx_editor);
	~TextureEditorPanel();

	bool		texModified() { return tex_modified; }
	CTexture*	getTexture() { return tex_current; }

	// UI Stuff
	virtual void		setupLayout();
	virtual wxPanel*	createTextureControls(wxWindow* parent);
	virtual void		updateTextureControls();
	virtual void		updateTextureScaleLabel();
	virtual wxPanel*	createPatchControls(wxWindow* parent);
	virtual void		populatePatchList();
	virtual void		updatePatchControls();

	bool	openTexture(CTexture* tex, TextureXList* list);
	void	clearTexture();
	void	setPalette(Palette8bit* pal);

	// Editing
	virtual void	addPatch();
	void			removePatch();
	void			patchBack();
	void			patchForward();
	virtual void	replacePatch();
	void			duplicatePatch();

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// SAction handler
	bool	handleAction(string id);

	// Events
	void	onZoomChanged(wxCommandEvent& e);
	void	onDrawOutsideChanged(wxCommandEvent& e);
	void	onTexCanvasMouseEvent(wxMouseEvent& e);
	void	onTexCanvasDragEnd(wxCommandEvent& e);
	void	onTexCanvasKeyDown(wxKeyEvent& e);
	void	onTexNameChanged(wxCommandEvent& e);
	void	onTexWidthChanged(wxSpinEvent& e);
	void	onTexHeightChanged(wxSpinEvent& e);
	void	onTexScaleXChanged(wxSpinEvent& e);
	void	onTexScaleYChanged(wxSpinEvent& e);
	void	onPatchListSelect(wxListEvent& e);
	void	onPatchListDeSelect(wxListEvent& e);
	void	onBtnPatchAdd(wxCommandEvent& e);
	void	onBtnPatchRemove(wxCommandEvent& e);
	void	onBtnPatchBack(wxCommandEvent& e);
	void	onBtnPatchForward(wxCommandEvent& e);
	void	onBtnPatchReplace(wxCommandEvent& e);
	void	onBtnPatchDuplicate(wxCommandEvent& e);
	void	onPatchPositionXChanged(wxSpinEvent& e);
	void	onPatchPositionYChanged(wxSpinEvent& e);
	void	onContextMenu(wxCommandEvent& e);
};

#endif//__TEXTURE_EDITOR_PANEL_H__
