
#ifndef __THING_TYPE_BROWSER_H__
#define __THING_TYPE_BROWSER_H__

#include "BrowserWindow.h"

class ThingType;
class ThingBrowserItem : public BrowserItem {
private:
	ThingType*	type;

public:
	ThingBrowserItem(string name, ThingType* type, unsigned index);
	~ThingBrowserItem();

	bool	loadImage();
};

class ThingTypeBrowser : public BrowserWindow {
private:

public:
	ThingTypeBrowser(wxWindow* parent, int type = -1);
	~ThingTypeBrowser();

	int	getSelectedType();
};

#endif//__THING_TYPE_BROWSER_H__
