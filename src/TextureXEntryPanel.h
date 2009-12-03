
#ifndef __TEXTUREXENTRYPANEL_H__
#define __TEXTUREXENTRYPANEL_H__

#include "EntryPanel.h"
#include "CompositeTextureCanvas.h"
#include "ListView.h"

class TextureXEntryPanel : public EntryPanel {
private:
	vector<CompositeTexture*>	textures;

	ListView*				list_textures;
	ListView*				list_patches;
	CompositeTextureCanvas*	tex_canvas;

public:
	TextureXEntryPanel(wxWindow* parent);
	~TextureXEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	void	populateTextureList();

	// Events
	void	onTextureListSelect(wxListEvent& e);
};

#endif//__TEXTUREXENTRYPANEL_H__
