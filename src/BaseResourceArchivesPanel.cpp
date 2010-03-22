
#include "Main.h"
#include "WxStuff.h"
#include "BaseResourceArchivesPanel.h"
#include "ArchiveManager.h"

BaseResourceArchivesPanel::BaseResourceArchivesPanel(wxWindow* parent)
: wxPanel(parent, -1) {
	// Setup sizer
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	// Init paths list
	int sel_index = -1;
	list_base_archive_paths = new wxListBox(this, -1);
	for (size_t a = 0; a < theArchiveManager->baseResourceListLength(); a++) {
		list_base_archive_paths->Append(theArchiveManager->baseResourcePath(a));

		if (theArchiveManager->baseResourceArchive() && !theArchiveManager->baseResourceArchive()->getFileName(true).Cmp(theArchiveManager->baseResourcePath(a)))
			sel_index = a;
	}

	// Select the currently open base archive if any
	if (sel_index >= 0)
		list_base_archive_paths->Select(sel_index);

	// Add paths list
	hbox->Add(list_base_archive_paths, 1, wxEXPAND|wxALL, 4);

	// Setup buttons
	btn_add = new wxButton(this, -1, _T("Add Archive"));
	btn_remove = new wxButton(this, -1, _T("Remove Archive"));
	btn_edit = new wxButton(this, -1, _T("Edit Path"));

	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	vbox->Add(btn_add, 0, wxEXPAND|wxBOTTOM, 4);
	vbox->Add(btn_remove, 0, wxEXPAND|wxBOTTOM, 4);
	vbox->Add(btn_edit, 0, wxEXPAND, 0);
	hbox->Add(vbox, 0, wxEXPAND|wxALL, 4);

	// Init layout
	Layout();
}

BaseResourceArchivesPanel::~BaseResourceArchivesPanel() {
}

void BaseResourceArchivesPanel::onBtnAdd(wxCommandEvent& e) {
}

void BaseResourceArchivesPanel::onBtnRemove(wxCommandEvent& e) {
}

void BaseResourceArchivesPanel::onBtnEdit(wxCommandEvent& e) {
}
