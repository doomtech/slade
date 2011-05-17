
#ifndef __BROWSER_ITEM_H__
#define __BROWSER_ITEM_H__

#include "GLTexture.h"

class BrowserWindow;
class BrowserItem {
friend class BrowserWindow;
protected:
	string			type;
	string			name;
	unsigned		index;
	GLTexture		image;
	BrowserWindow*	parent;

public:
	BrowserItem(string name, unsigned index = 0, string type = "item");
	~BrowserItem();

	string		getName() { return name; }
	unsigned	getIndex() { return index; }

	virtual bool	loadImage();
	void			draw(int size);
	void			clearImage();
};

#endif//__BROWSER_ITEM_H__
