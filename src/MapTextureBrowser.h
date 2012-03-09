
#ifndef __MAP_TEXTURE_BROWSER_H__
#define __MAP_TEXTURE_BROWSER_H__

#include "BrowserWindow.h"

class MapTexBrowserItem : public BrowserItem {
private:

public:
	MapTexBrowserItem(string name);
	~MapTexBrowserItem();

	bool	loadImage();
};

class MapTextureBrowser : public BrowserWindow {
private:

public:
	MapTextureBrowser(wxWindow* parent);
	~MapTextureBrowser();
};

#endif//__MAP_TEXTURE_BROWSER_H__
