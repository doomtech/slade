
#ifndef __TEXTUREXENTRYPANEL_H__
#define __TEXTUREXENTRYPANEL_H__

#include "EntryPanel.h"
#include "CTextureCanvas.h"
#include "ListView.h"
#include "TextureXList.h"
#include "PaletteChooser.h"
#include "GfxCanvas.h"
#include <wx/aui/auibook.h>


class TextureXEntryPanel : public EntryPanel {
private:
	TextureXList	texturex;
	wxAuiNotebook*	tabs;

	ArchiveEntry*	entry_texturex;
	ArchiveEntry*	entry_pnames;

	// TEXTUREx editor
	ListView*		list_textures;
	ListView*		list_patches;
	CTextureCanvas*	tex_canvas;
	PaletteChooser*	combo_palette;
	GfxCanvas*		gfx_patch_preview;

	// Patch table (PNAMES) editor
	ListView*		list_pnames;

public:
	TextureXEntryPanel(wxWindow* parent);
	~TextureXEntryPanel();

	wxPanel*	initTexArea();
	wxPanel*	initPnamesArea();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	void	populateTextureList();
	void	populatePatchesList();
	void	populatePnamesList();
	void	updateImagePalette();

	// Events
	void	onTextureListSelect(wxListEvent& e);
	void	onPatchesListSelect(wxListEvent& e);
	void	onPaletteChanged(wxCommandEvent& e);
};

#endif//__TEXTUREXENTRYPANEL_H__
