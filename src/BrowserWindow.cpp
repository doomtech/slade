
#include "Main.h"
#include "BrowserWindow.h"
#include "WxStuff.h"


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





BrowserWindow::BrowserWindow() : wxFrame(NULL, -1, "Browser") {
	// Setup layout
	wxBoxSizer* m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Browser tree
	tree_items = new wxTreeCtrl(this, -1);
	m_hbox->Add(tree_items, 0, wxEXPAND|wxALL, 4);

	// Browser canvas
	canvas = new BrowserCanvas(this);
	m_hbox->Add(canvas, 1, wxEXPAND|wxALL, 4);

	Layout();
}

BrowserWindow::~BrowserWindow() {
}
