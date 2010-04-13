
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


class TextureXEntryPanel : public EntryPanel {
private:
	TextureXList	texturex;
	tx_texture_t	tex_current;
	wxAuiNotebook*	tabs;

	ArchiveEntry*	entry_texturex;
	ArchiveEntry*	entry_pnames;

	// TEXTUREx editor
	ListView*		list_textures;
	ListView*		list_patches;
	CTextureCanvas*	tex_canvas;
	PaletteChooser*	combo_palette;
	wxSlider*		slider_zoom;
	wxCheckBox*		cb_draw_outside;
	wxStaticText*	label_current_zoom;
	GfxCanvas*		gfx_patch_preview;
	wxTextCtrl*		text_tex_name;
	wxSpinCtrl*		spin_tex_width;
	wxSpinCtrl*		spin_tex_height;
	wxSpinCtrl*		spin_tex_scalex;
	wxSpinCtrl*		spin_tex_scaley;
	wxStaticText*	label_scaled_size;
	ListView*		list_tex_patches;
	wxSpinCtrl*		spin_patch_xpos;
	wxSpinCtrl*		spin_patch_ypos;
	wxButton*		btn_patch_remove;
	wxButton*		btn_patch_forward;
	wxButton*		btn_patch_back;
	wxButton*		btn_patch_replace;

	// Patch table (PNAMES) editor
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

	// Events
	void	onTextureListSelect(wxListEvent& e);
	void	onPatchesListSelect(wxListEvent& e);
	void	onTexPatchesListSelect(wxListEvent& e);
	void	onTexPatchesListDeSelect(wxListEvent& e);
	void	onPaletteChanged(wxCommandEvent& e);
	void	onZoomChanged(wxCommandEvent& e);
	void	onDrawOutsideChanged(wxCommandEvent& e);
	void	onTexCanvasMouseLeftDown(wxMouseEvent& e);
};

#endif//__TEXTUREXENTRYPANEL_H__
