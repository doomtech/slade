
#ifndef __PATCH_BROWSER_H__
#define __PATCH_BROWSER_H__

#include "BrowserWindow.h"
#include "ListenerAnnouncer.h"
#include "PatchTable.h"

class PatchBrowserItem : public BrowserItem {
private:
	Archive*	archive;
	uint8_t		type;		// 0=patch, 1=ctexture
	string		nspace;

public:
	PatchBrowserItem(string name, Archive* archive = NULL, uint8_t type = 0, string nspace = "", unsigned index = 0);
	~PatchBrowserItem();

	bool	loadImage();
};

class PatchBrowser : public BrowserWindow, Listener {
private:
	PatchTable*		patch_table;

public:
	PatchBrowser(wxWindow* parent);
	~PatchBrowser();

	bool	openPatchTable(PatchTable* table);
	bool	openArchive(Archive* archive);
	int		getSelectedPatch();
	void	updateItems(BrowserTreeNode* node = NULL);

	// Events
	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

#endif//__PATCH_BROWSER_H__
