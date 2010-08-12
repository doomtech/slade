
#ifndef __PATCH_BROWSER_H__
#define __PATCH_BROWSER_H__

#include "BrowserWindow.h"
#include "ListenerAnnouncer.h"
#include "PatchTable.h"

class PatchBrowserItem : public BrowserItem {
private:
	ArchiveEntry*	entry;

public:
	PatchBrowserItem(string name, ArchiveEntry* entry = NULL);
	~PatchBrowserItem();

	bool loadImage();
};

class PatchBrowser : public BrowserWindow, Listener {
private:
	PatchTable*	patch_table;

public:
	PatchBrowser(wxWindow* parent);
	~PatchBrowser();

	bool	openPatchTable(PatchTable* table);
};

#endif//__PATCH_BROWSER_H__
