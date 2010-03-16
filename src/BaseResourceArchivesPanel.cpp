
#include "Main.h"
#include "WxStuff.h"
#include "BaseResourceArchivesPanel.h"

BaseResourceArchivesPanel::BaseResourceArchivesPanel(wxWindow* parent)
: wxPanel(parent, -1) {
	// Setup sizer
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);
}

BaseResourceArchivesPanel::~BaseResourceArchivesPanel() {
}
