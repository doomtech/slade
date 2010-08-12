
#ifndef __BROWSER_ITEM_H__
#define __BROWSER_ITEM_H__

#include "GLTexture.h"

class BrowserItem {
protected:
	string		name;
	GLTexture	image;

public:
	BrowserItem(string name);
	~BrowserItem();

	virtual bool	loadImage();
	void			draw(int size);
};

#endif//__BROWSER_ITEM_H__
