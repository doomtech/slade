
#ifndef __PATCH_BROWSER_H__
#define __PATCH_BROWSER_H__

#include "BrowserWindow.h"
#include "ListenerAnnouncer.h"
#include "PatchTable.h"

class PatchBrowserItem : public BrowserItem {
private:
	ArchiveEntry*	entry;
	Palette8bit*	palette;

public:
	PatchBrowserItem(string name, ArchiveEntry* entry = NULL, unsigned index = 0);
	~PatchBrowserItem();

	void	setPalette(Palette8bit* pal) { this->palette = pal; }
	void	setEntry(ArchiveEntry* entry) { this->entry = entry; }
	bool	loadImage();
};

class PatchBrowser : public BrowserWindow, Listener {
private:
	PatchTable*		patch_table;

public:
	PatchBrowser(wxWindow* parent);
	~PatchBrowser();

	bool	openPatchTable(PatchTable* table);
	int		getSelectedPatch();
	void	updateItems(BrowserTreeNode* node = NULL);

	// Events
	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

#endif//__PATCH_BROWSER_H__
