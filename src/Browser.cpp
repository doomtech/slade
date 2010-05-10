
#include "Main.h"
#include "WxStuff.h"
#include "Browser.h"
#include "Console.h"

Browser::Browser(wxWindow* parent) : wxPanel(parent, -1) {
	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Add tree
	//tree_items = new wxTreeCtrl(this, -1);
	//sizer->Add(tree_items, 0, wxEXPAND|wxALL, 4);

	// Add canvas
	browser_canvas = new BrowserCanvas(this);
	sizer->Add(browser_canvas, 1, wxEXPAND|wxALL, 4);

	// Temp
	for (int a = 0; a < 1234; a++)
		browser_canvas->addItem(browser_item_t());

	// Init layout
	Layout();
}

Browser::~Browser() {
}





void c_test_browser(vector<string> args) {
	wxDialog dlg(NULL, -1, _T("Browser!"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	dlg.SetSizer(hbox);

	Browser* browser = new Browser(&dlg);
	hbox->Add(browser, 1, wxEXPAND|wxALL, 4);

	dlg.ShowModal();
}
ConsoleCommand test_browser(_T("test_browser"), &c_test_browser, 0);
