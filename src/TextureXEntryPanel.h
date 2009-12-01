
#ifndef __TEXTUREXENTRYPANEL_H__
#define __TEXTUREXENTRYPANEL_H__

#include "EntryPanel.h"
#include "CompositeTexture.h"

class TextureXEntryPanel : public EntryPanel {
private:
	vector<CompositeTexture*>	textures;

public:
	TextureXEntryPanel(wxWindow* parent);
	~TextureXEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
};

#endif//__TEXTUREXENTRYPANEL_H__
