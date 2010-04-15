
#ifndef __TEXTUREXENTRYPANEL_H__
#define __TEXTUREXENTRYPANEL_H__

#include "EntryPanel.h"
#include "CTextureCanvas.h"
#include "ListView.h"
#include "TextureXList.h"
#include "PaletteChooser.h"
#include "GfxCanvas.h"
#include <wx/aui/auibook.h>
#include <wx/spinctrl.h>

// TODO/Ideas:
// - Layout change to accomodate zdoom TEXTURES editing:
//   - Remove 'available patches' list
//   - Move texture patch editing controls to where available patches list was
//     (this will give more room for editing controls, vertically)
//   - More bitmap buttons for patch operations:
//     - Add patch - will open a patch browser (similar to SLADE2/DB texture browser) to select the patch
//       *Having a patch browser allows a nicer way to sort patches when it comes to TEXTURES, since that
//       can have any existing patch/sprite/texture as a patch, rather than being confined to a PNAMES list entry*
//     - Replace patch(es) - replace the currently selected patches with browser selection
//     - Duplicate patch - create duplicate copies of selected patches on top and slightly down+left of each
// - 'New Texture' button/option - Will give the option of creating a new blank texture, or texture from patch
class TextureXEntryPanel : public EntryPanel {
private:
	TextureXList	texturex;
	wxAuiNotebook*	tabs;

	ArchiveEntry*	entry_texturex;
	ArchiveEntry*	entry_pnames;

	// -- TEXTUREx editor --
	ListView*		list_textures;
	ListView*		list_patches;
	CTextureCanvas*	tex_canvas;
	PaletteChooser*	combo_palette;
	wxSlider*		slider_zoom;
	wxCheckBox*		cb_draw_outside;
	wxStaticText*	label_current_zoom;
	GfxCanvas*		gfx_patch_preview;
	wxStaticText*	label_patch_size;
	wxButton*		btn_add_patch;
	// Texture Controls
	wxTextCtrl*		text_tex_name;
	wxSpinCtrl*		spin_tex_width;
	wxSpinCtrl*		spin_tex_height;
	wxSpinCtrl*		spin_tex_scalex;
	wxSpinCtrl*		spin_tex_scaley;
	wxStaticText*	label_scaled_size;
	ListView*		list_tex_patches;
	// Patch controls
	wxSpinCtrl*		spin_patch_xpos;
	wxSpinCtrl*		spin_patch_ypos;
	wxButton*		btn_patch_remove;
	wxButton*		btn_patch_forward;
	wxButton*		btn_patch_back;
	wxButton*		btn_patch_replace;

	// -- Patch table (PNAMES) editor --
	ListView*		list_pnames;

public:
	TextureXEntryPanel(wxWindow* parent);
	~TextureXEntryPanel();

	wxPanel*	initTexArea(wxWindow* parent);
	wxPanel*	initTexControls(wxWindow* parent);
	wxPanel*	initPnamesArea(wxWindow* parent);

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	void	populateTextureList();
	void	populatePatchesList();
	void	populatePnamesList();
	void	updateImagePalette();
	void	updateTextureControls();
	void	updateTextureScaleLabel();

	// Events
	void	onTextureListSelect(wxListEvent& e);
	void	onPatchesListSelect(wxListEvent& e);
	void	onTexPatchesListSelect(wxListEvent& e);
	void	onTexPatchesListDeSelect(wxListEvent& e);
	void	onPaletteChanged(wxCommandEvent& e);
	void	onZoomChanged(wxCommandEvent& e);
	void	onDrawOutsideChanged(wxCommandEvent& e);
	void	onTexCanvasMouseEvent(wxMouseEvent& e);
	void	onTexCanvasDragEnd(wxCommandEvent& e);
	void	onTexWidthChanged(wxSpinEvent& e);
	void	onTexHeightChanged(wxSpinEvent& e);
	void	onTexScaleXChanged(wxSpinEvent& e);
	void	onTexScaleYChanged(wxSpinEvent& e);
	void	onPatchLeftChanged(wxSpinEvent& e);
	void	onPatchTopChanged(wxSpinEvent& e);
	void	onBtnPatchForward(wxCommandEvent& e);
	void	onBtnPatchBack(wxCommandEvent& e);
};

#endif//__TEXTUREXENTRYPANEL_H__
