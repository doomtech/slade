
#ifndef __BROWSER_WINDOW_H__
#define __BROWSER_WINDOW_H__

#include "Tree.h"
#include "OGLCanvas.h"
#include "BrowserItem.h"
#include <wx/treectrl.h>
#include <wx/frame.h>

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
	BrowserTreeNode(BrowserTreeNode* parent = NULL);
	~BrowserTreeNode();

	string	getName() { return name; }
	void	setName(string name) { this->name = name; }

	void			clearItems();
	unsigned		nItems() { return items.size(); }
	BrowserItem*	getItem(unsigned index);
	void			addItem(BrowserItem* item, unsigned index = 0xFFFFFFFF);
};

class BrowserCanvas : public OGLCanvas {
private:
	BrowserTreeNode*	items_root;

public:
	BrowserCanvas(wxWindow* parent);
	~BrowserCanvas();

	void draw();
};

class BrowserWindow : public wxFrame {
private:
	BrowserTreeNode*	items_root;
	BrowserCanvas*		canvas;
	wxTreeCtrl*			tree_items;

public:
	BrowserWindow();
	~BrowserWindow();

	bool	addItem(BrowserItem* item, string where = "");

	void	populateItemTree();
	void	addItemTree(BrowserTreeNode* node, wxTreeItemId& item);
};

#endif//__BROWSER_WINDOW_H__
