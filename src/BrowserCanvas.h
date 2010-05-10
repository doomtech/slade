
#ifndef __BROWSER_CANVAS_H__
#define __BROWSER_CANVAS_H__

#include "OGLCanvas.h"
#include "GLTexture.h"

struct browser_item_t {
	GLTexture	texture;
	string		name;
	int			usage;
};

class BrowserCanvas : public OGLCanvas {
private:
	vector<browser_item_t>	items;

public:
	BrowserCanvas(wxWindow* parent);
	~BrowserCanvas();

	void	draw();

	void	clearItems() { items.clear(); }
	void	addItem(browser_item_t item) { items.push_back(item); }
};

#endif//__BROWSER_CANVAS_H__
