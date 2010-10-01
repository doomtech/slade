
#ifndef __BROWSER_ITEM_H__
#define __BROWSER_ITEM_H__

#include "GLTexture.h"

class BrowserItem {
protected:
	string		name;
	unsigned	index;
	GLTexture	image;

public:
	BrowserItem(string name, unsigned index = 0);
	~BrowserItem();

	string		getName() { return name; }
	unsigned	getIndex() { return index; }

	virtual bool	loadImage();
	void			draw(int size);
};

#endif//__BROWSER_ITEM_H__
