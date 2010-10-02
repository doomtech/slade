
#ifndef __PATCH_BROWSER_H__
#define __PATCH_BROWSER_H__

#include "BrowserWindow.h"
#include "ListenerAnnouncer.h"
#include "PatchTable.h"
#include "PaletteChooser.h"

class PatchBrowserItem : public BrowserItem {
private:
	ArchiveEntry*	entry;
	Palette8bit*	palette;

public:
	PatchBrowserItem(string name, ArchiveEntry* entry = NULL, unsigned index = 0);
	~PatchBrowserItem();

	void	setPalette(Palette8bit* pal) { this->palette = pal; }
	bool	loadImage();
};

class PatchBrowser : public BrowserWindow, Listener {
private:
	PatchTable*		patch_table;
	PaletteChooser*	pal_chooser;

public:
	PatchBrowser(wxWindow* parent);
	~PatchBrowser();

	bool	openPatchTable(PatchTable* table);
	int		getSelectedPatch();
};

#endif//__PATCH_BROWSER_H__
