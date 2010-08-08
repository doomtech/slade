
#include "Main.h"
#include "BrowserWindow.h"
#include "WxStuff.h"
#include "Console.h"


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



BrowserCanvas::BrowserCanvas(wxWindow* parent) : OGLCanvas(parent, -1) {
}

BrowserCanvas::~BrowserCanvas() {
}

void BrowserCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


	// Swap Buffers
	SwapBuffers();
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


BrowserWindow::BrowserWindow() : wxFrame(NULL, -1, "Browser") {
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

	// Browser canvas
	canvas = new BrowserCanvas(this);
	m_hbox->Add(canvas, 1, wxEXPAND|wxALL, 4);

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

void BrowserWindow::populateItemTree() {
	// Clear current tree
	tree_items->DeleteAllItems();

	// Add root item
	wxTreeItemId item = tree_items->AddRoot("All");

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




CONSOLE_COMMAND(test_browser, 0) {
	BrowserWindow* win = new BrowserWindow();

	// Test
	BrowserItem* bi = new BrowserItem("test");
	win->addItem(bi, "Patches");
	win->addItem(bi, "Patches/IWAD");
	win->addItem(bi, "Patches/Custom");
	win->addItem(bi, "Patches/Unknown");

	win->populateItemTree();
	win->Show(true);
}
