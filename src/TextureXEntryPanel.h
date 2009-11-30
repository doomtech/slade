
#ifndef __TEXTUREXENTRYPANEL_H__
#define __TEXTUREXENTRYPANEL_H__

#include "EntryPanel.h"

class TextureXEntryPanel : public EntryPanel {
private:

public:
	TextureXEntryPanel(wxWindow* parent);
	~TextureXEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
};

#endif//__TEXTUREXENTRYPANEL_H__
