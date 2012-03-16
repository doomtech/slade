
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    BrowserWindow.cpp
 * Description: The browser window implementation. A frame that
 *              contains a tree of item categories/subcategories,
 *              and an OGLCanvas where the browser items under
 *              the currently selected category are displayed
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "BrowserWindow.h"
#include "Console.h"
#include <algorithm>
#include <wx/splitter.h>


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Int, browser_item_size)


/*******************************************************************
 * BROWSERTREENODE CLASS FUNCTIONS
 *******************************************************************/

/* BrowserTreeNode::BrowserTreeNode
 * BrowserTreeNode class constructor
 *******************************************************************/
BrowserTreeNode::BrowserTreeNode(BrowserTreeNode* parent) : STreeNode(parent) {
}

/* BrowserTreeNode::~BrowserTreeNode
 * BrowserTreeNode class destructor
 *******************************************************************/
BrowserTreeNode::~BrowserTreeNode() {
	clearItems();
}

/* BrowserTreeNode::clearItems
 * Clears all items in the node
 *******************************************************************/
void BrowserTreeNode::clearItems() {
	for (unsigned a = 0; a < items.size(); a++)
		delete items[a];
	items.clear();
}

/* BrowserTreeNode::getItem
 * Gets the item at [index], or NULL if [index] is out of bounds
 *******************************************************************/
BrowserItem* BrowserTreeNode::getItem(unsigned index) {
	// Check index
	if (index >= items.size())
		return NULL;

	// Return the item
	return items[index];
}

/* BrowserTreeNode::addItem
 * Adds [item] to the node at [index], or at the end if [index] is
 * out of bounds
 *******************************************************************/
void BrowserTreeNode::addItem(BrowserItem* item, unsigned index) {
	// Check where to add item
	if (index >= items.size())
		items.push_back(item);
	else
		items.insert(items.begin() + index, item);
}


/*******************************************************************
 * BROWSERTREEITEMDATA CLASS
 *******************************************************************
 * wxTreeItemData class needed to associate BrowserTreeNodes with tree items
 */
class BrowserTreeItemData : public wxTreeItemData {
private:
	BrowserTreeNode*	node;

public:
	BrowserTreeItemData(BrowserTreeNode* node = NULL) { this->node = node; }
	~BrowserTreeItemData() {}

	BrowserTreeNode*	getNode() { return node; }
};


/*******************************************************************
 * BROWSERWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* BrowserWindow::BrowserWindow
 * BrowserWindow class constructor
 *******************************************************************/
BrowserWindow::BrowserWindow(wxWindow* parent) : wxDialog(parent, -1, "Browser", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX) {
	// Init variables
	items_root = new BrowserTreeNode();
	items_root->setName("All");

	// Setup layout
	wxBoxSizer* m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	wxSplitterWindow* swin = new wxSplitterWindow(this, -1);
	m_vbox->Add(swin, 1, wxEXPAND|wxALL, 4);

	// Browser tree
	tree_items = new wxTreeCtrl(swin, -1, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);

	// Browser area
	wxPanel* panel_browser_area = new wxPanel(swin, -1);
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	panel_browser_area->SetSizer(vbox);

	// Zoom
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	slider_zoom = new wxSlider(panel_browser_area, -1, browser_item_size, 64, 256);
	slider_zoom->SetLineSize(16);
	slider_zoom->SetPageSize(32);
	hbox->Add(new wxStaticText(panel_browser_area, -1, "Zoom:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	hbox->Add(slider_zoom, 1, wxEXPAND);

	// Sorting
	choice_sort = new wxChoice(panel_browser_area, -1);
	hbox->AddStretchSpacer();
	hbox->Add(new wxStaticText(panel_browser_area, -1, "Sort:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	hbox->Add(choice_sort, 0, wxEXPAND|wxRIGHT, 4);

	// Filter
	text_filter = new wxTextCtrl(panel_browser_area, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	hbox->Add(new wxStaticText(panel_browser_area, -1, "Filter:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	hbox->Add(text_filter, 0, wxEXPAND);

	// Browser canvas
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 1, wxEXPAND|wxBOTTOM, 4);
	canvas = new BrowserCanvas(panel_browser_area);
	hbox->Add(canvas, 1, wxEXPAND);
	// Canvas scrollbar
	wxScrollBar* scrollbar = new wxScrollBar(panel_browser_area, -1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
	hbox->Add(scrollbar, 0, wxEXPAND);
	canvas->setScrollBar(scrollbar);

	swin->SplitVertically(tree_items, panel_browser_area, 150);

	// Bottom sizer
	sizer_bottom = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(sizer_bottom, 0, wxEXPAND|wxBOTTOM, 4);

	// Buttons
	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxBOTTOM, 4);

	// Setup sorting options
	addSortType("Index");
	addSortType("Name (Alphabetical)");
	choice_sort->SetSelection(0);

	// Bind events
	tree_items->Bind(wxEVT_COMMAND_TREE_SEL_CHANGED, &BrowserWindow::onTreeItemSelected, this);
	choice_sort->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &BrowserWindow::onChoiceSortChanged, this);
	canvas->Bind(wxEVT_LEFT_DCLICK, &BrowserWindow::onCanvasDClick, this);
	text_filter->Bind(wxEVT_COMMAND_TEXT_UPDATED, &BrowserWindow::onTextFilterChanged, this);
	slider_zoom->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &BrowserWindow::onZoomChanged, this);

	Layout();
	SetInitialSize(wxSize(768, 600));
	SetMinSize(wxSize(540, 400));
	CenterOnScreen();
}

/* BrowserWindow::~BrowserWindow
 * BrowserWindow class destructor
 *******************************************************************/
BrowserWindow::~BrowserWindow() {
}

/* BrowserWindow::addItem
 * Adds [item] to the browser tree at the tree path [where]. This
 * will be created if it doesn't exist
 *******************************************************************/
bool BrowserWindow::addItem(BrowserItem* item, string where) {
	BrowserTreeNode* target = (BrowserTreeNode*)items_root->addChild(where);
	target->addItem(item);
	return true;
}

/* BrowserWindow::clearItems
 * Removes all items from [node] and its children recursively
 *******************************************************************/
void BrowserWindow::clearItems(BrowserTreeNode* node) {
	// Check node was given to begin clear
	if (!node)
		node = items_root;

	// Clear all items from node
	node->clearItems();

	// Clear all child nodes
	while (node->nChildren() > 0) {
		BrowserTreeNode* child = (BrowserTreeNode*)node->getChild(0);
		clearItems(child);
		node->removeChild(child);
		delete child;
	}
}

/* BrowserWindow::reloadItems
 * Reloads (clears) all item images in [node] and its children
 * recursively
 *******************************************************************/
void BrowserWindow::reloadItems(BrowserTreeNode* node) {
	// Check node was given to begin reload
	if (!node)
		node = items_root;

	// Go through items in this node
	for (unsigned a = 0; a < node->nItems(); a++)
		node->getItem(a)->clearImage();

	// Go through child nodes
	for (unsigned a = 0; a < node->nChildren(); a++)
		reloadItems((BrowserTreeNode*)node->getChild(a));
}

/* BrowserWindow::selectItem
 * Finds the item matching [name] in the tree, starting from [node].
 * If the item is found, its parent node is opened in the browser
 * and the item is selected
 *******************************************************************/
bool BrowserWindow::selectItem(string name, BrowserTreeNode* node) {
	// Check node was given, if not start from root
	if (!node)
		node = items_root;

	// Go through all items in this node
	for (unsigned a = 0;  a < node->nItems(); a++) {
		// Check for name match (not case-sensitive)
		if (S_CMPNOCASE(node->getItem(a)->getName(), name)) {
			// Open this node in the browser and select the item
			openTree(node);
			canvas->selectItem(node->getItem(a));
			canvas->showSelectedItem();
			tree_items->SelectItem(node->getTreeId());
			tree_items->Expand(node->getTreeId());
			return true;
		}
	}

	// Item not found in this one, try its child nodes
	for (unsigned a = 0; a < node->nChildren(); a++) {
		if (selectItem(name, (BrowserTreeNode*)node->getChild(a)))
			return true;
	}

	// Item not found
	return false;
}

// Sorting functions
bool sortBIIndex(BrowserItem* left, BrowserItem* right) {
	return left->getIndex() < right->getIndex();
}
bool sortBIName(BrowserItem* left, BrowserItem* right) {
	return left->getName() < right->getName();
}

/* BrowserWindow::addSortType
 * Adds a sorting type [name] to the window
 *******************************************************************/
unsigned BrowserWindow::addSortType(string name) {
	choice_sort->AppendString(name);
	return choice_sort->GetCount() - 1;
}

/* BrowserWindow::doSort
 * Performs sorting of the items currently being browsed, the sort
 * criteria depending on [sort_type]. Default sorting types are by
 * index (0) and by name (1), more can be added to BrowserWindow
 * child classes if needed
 *******************************************************************/
void BrowserWindow::doSort(unsigned sort_type) {
	// Get item list
	vector<BrowserItem*>& items = canvas->itemList();

	// Do sorting

	// 0: By Index
	if (sort_type == 0)
		std::sort(items.begin(), items.end(), sortBIIndex);

	// 1: By Name (Alphabetical)
	else if (sort_type == 1)
		std::sort(items.begin(), items.end(), sortBIName);

	// Refresh canvas
	canvas->showSelectedItem();
	canvas->Refresh();
}

/* BrowserWindow::setSortType
 * Sets the current sorting method to [type]
 *******************************************************************/
void BrowserWindow::setSortType(int type) {
	// Check type index
	if (type < 0 || type >= choice_sort->GetCount())
		return;

	// Select sorting type
	choice_sort->SetSelection(type);

	// Do sorting
	doSort(type);
}

/* BrowserWindow::openTree
 * 'Opens' the items in [node] and all its children, adding them
 * to the browser canvas' list of items. If [clear] is true, the
 * current list contents will be cleared
 *******************************************************************/
void BrowserWindow::openTree(BrowserTreeNode* node, bool clear) {
	// Get item list from canvas
	vector<BrowserItem*>& list = canvas->itemList();

	// Clear it if needed
	if (clear)
		list.clear();

	// Add all items in the node
	for (unsigned a = 0; a < node->nItems(); a++) {
		list.push_back(node->getItem(a));
		node->getItem(a)->parent = this;
	}

	// Add all child nodes' items
	for (unsigned a = 0; a < node->nChildren(); a++)
		openTree((BrowserTreeNode*)node->getChild(a), false);

	// If the list was cleared, sort it, filter it and update the canvas scrollbar
	if (clear) {
		doSort(choice_sort->GetSelection());
		canvas->updateScrollBar();
		canvas->filterItems(text_filter->GetValue());
	}

	// Clear item selection
	canvas->selectItem(-1);
}

/* BrowserWindow::populateItemTree
 * Populates the wxTreeCtrl with the contents of the browser item
 * category tree
 *******************************************************************/
void BrowserWindow::populateItemTree() {
	// Clear current tree
	tree_items->DeleteAllItems();

	// Add root item
	wxTreeItemId item = tree_items->AddRoot("All");
	tree_items->SetItemData(item, new BrowserTreeItemData(items_root));

	// Add tree
	addItemTree(items_root, item);

	// Update window layout
	tree_items->ExpandAll();
	tree_items->SetInitialSize(wxSize(130, -1));
	Layout();
	tree_items->CollapseAll();
}

/* BrowserWindow::addItemTree
 * Adds [node] to the wxTreeCtrl after [item]
 *******************************************************************/
void BrowserWindow::addItemTree(BrowserTreeNode* node, wxTreeItemId& item) {
	// Go through child items
	for (unsigned a = 0; a < node->nChildren(); a++) {
		// Add tree item
		BrowserTreeNode* child = (BrowserTreeNode*)node->getChild(a);
		wxTreeItemId i = tree_items->AppendItem(item, child->getName(), -1, -1, new BrowserTreeItemData(child));
		child->setTreeId(i);

		// Add children
		addItemTree(child, i);
	}
}

/* BrowserWindow::setFont
 * Sets the font to be used for item names
 *******************************************************************/
void BrowserWindow::setFont(int font) {
	canvas->setFont(font);
}

/* BrowserWindow::showSelectedNameOnly
 * Sets whether to only show the name for the currently selected item
 * (true) or all items (false)
 *******************************************************************/
void BrowserWindow::showNames(int show) {
	canvas->showNames(show);
}

/* BrowserWindow::setItemSize
 * Sets the item size (0 or less to use zoom slider)
 *******************************************************************/
void BrowserWindow::setItemSize(int size) {
	canvas->setItemSize(size);

	if (size <= 0)
		slider_zoom->Enable(true);
	else
		slider_zoom->Enable(false);

	Layout();
	Refresh();
}


/*******************************************************************
 * BROWSERWINDOW CLASS EVENTS
 *******************************************************************/

/* BrowserWindow::onTreeItemSelected
 * Called when an item on the category wxTreeCtrl is selected
 *******************************************************************/
void BrowserWindow::onTreeItemSelected(wxTreeEvent& e) {
	BrowserTreeItemData* data = (BrowserTreeItemData*)tree_items->GetItemData(e.GetItem());
	openTree(data->getNode());
	canvas->Refresh();
}

/* BrowserWindow::onChoiceSortChanged
 * Called when the 'Sort By' dropdown selection is changed
 *******************************************************************/
void BrowserWindow::onChoiceSortChanged(wxCommandEvent& e) {
	// Re-sort items
	doSort(choice_sort->GetSelection());
}

/* BrowserWindow::onCanvasDClick
 * Called when the browser canvas is double-clicked
 *******************************************************************/
void BrowserWindow::onCanvasDClick(wxMouseEvent& e) {
	// End modal if an item was double-clicked
	EndModal(wxID_OK);
}

/* BrowserWindow::onTextFilterChanged
 * Called when the name filter is changed
 *******************************************************************/
void BrowserWindow::onTextFilterChanged(wxCommandEvent& e) {
	// Filter canvas items
	canvas->filterItems(text_filter->GetValue());
}

/* BrowserWindow::sliderZoomChanged
 * Called when the zoom slider is changed
 *******************************************************************/
void BrowserWindow::onZoomChanged(wxCommandEvent& e) {
	// Get zoom value
	int item_size = slider_zoom->GetValue();

	// Lock to increments of 16
	int remainder = item_size % 16;
	item_size -= remainder;

	// Set slider value to locked increment
	slider_zoom->SetValue(item_size);

	// Update zoom label
	//label_current_zoom->SetLabel(S_FMT("%d%%", zoom_percent));

	// Update item size and refresh
	browser_item_size = item_size;
	canvas->updateScrollBar();
	canvas->Refresh();
}
