
#ifndef __BROWSER_WINDOW_H__
#define __BROWSER_WINDOW_H__

#include "Tree.h"
#include "OGLCanvas.h"
#include "BrowserItem.h"

class BrowserTreeNode : public STreeNode {
private:
	string 					name;
	vector<BrowserItem*>	items;

	STreeNode* createChild(string name) {
		BrowserTreeNode* node = new BrowserTreeNode();
		node->name = name;
		return node;
	}

public:
	BrowserTreeNode();
	~BrowserTreeNode();

	string	getName() { return name; }
	void	setName(string name) { this->name = name; }
};

class BrowserCanvas : public OGLCanvas {
private:
	BrowserTreeNode*	items_root;

public:
	BrowserCanvas();
	~BrowserCanvas();

	void draw();
};

class BrowserWindow : public wxFrame {
private:
	BrowserTreeNode*	items_root;
	BrowserCanvas*		canvas;

public:
	BrowserWindow();
	~BrowserWindow();
};

#endif//__BROWSER_WINDOW_H__
