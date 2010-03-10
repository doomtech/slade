
#ifndef __TEXTUREXENTRYPANEL_H__
#define __TEXTUREXENTRYPANEL_H__

#include "EntryPanel.h"
#include "CTextureCanvas.h"
#include "ListView.h"
#include "TextureXList.h"
#include "PaletteChooser.h"

class TextureXEntryPanel : public EntryPanel {
private:
	//vector<CTexture*>	textures;
	TextureXList	texturex;

	ListView*		list_textures;
	ListView*		list_patches;
	CTextureCanvas*	tex_canvas;
	PaletteChooser*	combo_palette;

public:
	TextureXEntryPanel(wxWindow* parent);
	~TextureXEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	void	populateTextureList();
	void	updateImagePalette();

	// Events
	void	onTextureListSelect(wxListEvent& e);
	void	onPaletteChanged(wxCommandEvent& e);
};

#endif//__TEXTUREXENTRYPANEL_H__
