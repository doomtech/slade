
#include "Main.h"
#include "WxStuff.h"
#include "BrowserWindow.h"
#include "Console.h"
#include <algorithm>


BrowserTreeNode::BrowserTreeNode(BrowserTreeNode* parent) : STreeNode(parent) {
}

BrowserTreeNode::~BrowserTreeNode() {
	clearItems();
}

void BrowserTreeNode::clearItems() {
	for (unsigned a = 0; a < items.size(); a++)
		delete items[a];
	items.clear();
}

BrowserItem* BrowserTreeNode::getItem(unsigned index) {
	// Check index
	if (index >= items.size())
		return NULL;

	// Return the item
	return items[index];
}

void BrowserTreeNode::addItem(BrowserItem* item, unsigned index) {
	// Check where to add item
	if (index >= items.size())
		items.push_back(item);
	else
		items.insert(items.begin() + index, item);
}



// wxTreeItemData class needed to associate BrowserTreeNodes with tree items
class BrowserTreeItemData : public wxTreeItemData {
private:
	BrowserTreeNode*	node;

public:
	BrowserTreeItemData(BrowserTreeNode* node = NULL) { this->node = node; }
	~BrowserTreeItemData() {}

	BrowserTreeNode*	getNode() { return node; }
};


BrowserWindow::BrowserWindow(wxWindow* parent) : wxDialog(parent, -1, "Browser", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX) {
	// Init variables
	items_root = new BrowserTreeNode();
	items_root->setName("All");

	// Setup layout
	wxBoxSizer* m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Browser tree
	tree_items = new wxTreeCtrl(this, -1, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
	tree_items->SetSizeHints(wxSize(100, -1));
	m_hbox->Add(tree_items, 0, wxEXPAND|wxALL, 4);

	// Browser area
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	m_hbox->Add(vbox, 1, wxEXPAND|wxALL, 4);

	// Sorting
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);
	choice_sort = new wxChoice(this, -1);
	hbox->AddStretchSpacer();
	hbox->Add(new wxStaticText(this, -1, "Sort:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	hbox->Add(choice_sort, 0, wxEXPAND);

	// Browser canvas
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 1, wxEXPAND|wxBOTTOM, 4);
	canvas = new BrowserCanvas(this);
	hbox->Add(canvas, 1, wxEXPAND);
	// Canvas scrollbar
	wxScrollBar* scrollbar = new wxScrollBar(this, -1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
	hbox->Add(scrollbar, 0, wxEXPAND);
	canvas->setScrollBar(scrollbar);

	// Bottom sizer
	sizer_bottom = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(sizer_bottom, 0, wxEXPAND|wxBOTTOM, 4);

	// Buttons
	vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND);

	// Setup sorting options
	addSortType("Index");
	addSortType("Name (Alphabetical)");
	choice_sort->SetSelection(0);

	// Bind events
	tree_items->Bind(wxEVT_COMMAND_TREE_SEL_CHANGED, &BrowserWindow::onTreeItemSelected, this);
	choice_sort->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &BrowserWindow::onChoiceSortChanged, this);
	canvas->Bind(wxEVT_LEFT_DCLICK, &BrowserWindow::onCanvasDClick, this);

	Layout();
	SetInitialSize(wxSize(600, 400));
	CenterOnScreen();
}

BrowserWindow::~BrowserWindow() {
}

bool BrowserWindow::addItem(BrowserItem* item, string where) {
	BrowserTreeNode* target = (BrowserTreeNode*)items_root->addChild(where);
	target->addItem(item);
	return true;
}

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

// Sorting functions
bool sortBIIndex(BrowserItem* left, BrowserItem* right) {
	return left->getIndex() < right->getIndex();
}
bool sortBIName(BrowserItem* left, BrowserItem* right) {
	return left->getName() < right->getName();
}

unsigned BrowserWindow::addSortType(string name) {
	choice_sort->AppendString(name);
	return choice_sort->GetCount() - 1;
}

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
	canvas->Refresh();
}

void BrowserWindow::openTree(BrowserTreeNode* node, bool clear) {
	// Get item list from canvas
	vector<BrowserItem*>& list = canvas->itemList();

	// Clear it if needed
	if (clear)
		list.clear();

	// Add all items in the node
	for (unsigned a = 0; a < node->nItems(); a++)
		list.push_back(node->getItem(a));

	// Add all child nodes' items
	for (unsigned a = 0; a < node->nChildren(); a++)
		openTree((BrowserTreeNode*)node->getChild(a), false);

	// If the list was cleared, sort it and update the canvas scrollbar
	if (clear) {
		doSort(choice_sort->GetSelection());
		canvas->updateScrollBar();
	}
}

void BrowserWindow::populateItemTree() {
	// Clear current tree
	tree_items->DeleteAllItems();

	// Add root item
	wxTreeItemId item = tree_items->AddRoot("All");
	tree_items->SetItemData(item, new BrowserTreeItemData(items_root));

	// Add tree
	addItemTree(items_root, item);
}

void BrowserWindow::addItemTree(BrowserTreeNode* node, wxTreeItemId& item) {
	// Go through child items
	for (unsigned a = 0; a < node->nChildren(); a++) {
		// Add tree item
		BrowserTreeNode* child = (BrowserTreeNode*)node->getChild(a);
		wxTreeItemId i = tree_items->AppendItem(item, child->getName(), -1, -1, new BrowserTreeItemData(child));

		// Add children
		addItemTree(child, i);
	}
}



void BrowserWindow::onTreeItemSelected(wxTreeEvent& e) {
	BrowserTreeItemData* data = (BrowserTreeItemData*)tree_items->GetItemData(e.GetItem());
	openTree(data->getNode());
	canvas->Refresh();
}

void BrowserWindow::onChoiceSortChanged(wxCommandEvent& e) {
	// Re-sort items
	doSort(choice_sort->GetSelection());
}

void BrowserWindow::onCanvasDClick(wxMouseEvent& e) {
	// End modal if an item was double-clicked
	EndModal(wxID_OK);
}



CONSOLE_COMMAND(test_browser, 0) {
	BrowserWindow* win = new BrowserWindow(NULL);

	// Test
	BrowserItem* bi = new BrowserItem("test");
	win->addItem(bi, "Patches");
	win->addItem(bi, "Patches/IWAD");
	win->addItem(bi, "Patches/Custom");
	win->addItem(bi, "Patches/Unknown");

	win->populateItemTree();
	win->Show(true);
}
