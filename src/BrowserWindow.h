
#ifndef __BROWSER_WINDOW_H__
#define __BROWSER_WINDOW_H__

#include "Tree.h"
#include "BrowserItem.h"
#include "BrowserCanvas.h"
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

	string		getName() { return name; }
	void		setName(string name) { this->name = name; }

	void			clearItems();
	unsigned		nItems() { return items.size(); }
	BrowserItem*	getItem(unsigned index);
	void			addItem(BrowserItem* item, unsigned index = 0xFFFFFFFF);
};


class BrowserWindow : public wxDialog {
private:
	BrowserCanvas*		canvas;
	wxTreeCtrl*			tree_items;
	wxChoice*			choice_sort;

protected:
	BrowserTreeNode*	items_root;
	wxBoxSizer*			sizer_bottom;

public:
	BrowserWindow(wxWindow* parent);
	~BrowserWindow();

	bool			addItem(BrowserItem* item, string where = "");
	void			clearItems(BrowserTreeNode* node = NULL);
	void			reloadItems(BrowserTreeNode* node = NULL);
	BrowserItem*	getSelectedItem() { return canvas->getSelectedItem(); }

	unsigned		addSortType(string name);
	virtual void	doSort(unsigned sort_type = 0);

	void	openTree(BrowserTreeNode* node, bool clear = true);

	void	populateItemTree();
	void	addItemTree(BrowserTreeNode* node, wxTreeItemId& item);

	// Events
	void	onTreeItemSelected(wxTreeEvent& e);
	void	onChoiceSortChanged(wxCommandEvent& e);
	void	onCanvasDClick(wxMouseEvent& e);
};

#endif//__BROWSER_WINDOW_H__
