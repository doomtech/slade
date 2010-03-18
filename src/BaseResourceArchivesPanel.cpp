
#include "Main.h"
#include "WxStuff.h"
#include "BaseResourceArchivesPanel.h"
#include "ArchiveManager.h"

BaseResourceArchivesPanel::BaseResourceArchivesPanel(wxWindow* parent)
: wxPanel(parent, -1) {
	// Setup sizer
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	// Init paths list
	list_base_archive_paths = new ListView(this, -1);
	for (size_t a = 0; a < theArchiveManager->baseResourceListLength(); a++)
		list_base_archive_paths->addItem(a, theArchiveManager->baseResourcePath(a));

	// Add paths list
	vbox->Add(list_base_archive_paths, 1, wxEXPAND|wxALL, 4);

	// Setup buttons
	btn_add = new wxButton(this, -1, _T("Add Archive"));
	btn_remove = new wxButton(this, -1, _T("Remove Archive"));
	btn_edit = new wxButton(this, -1, _T("Edit Path"));

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	//hbox->AddStretchSpacer();
	hbox->Add(btn_add, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_remove, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_edit, 0, wxEXPAND, 0);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	// Init layout
	Layout();
}

BaseResourceArchivesPanel::~BaseResourceArchivesPanel() {
}
