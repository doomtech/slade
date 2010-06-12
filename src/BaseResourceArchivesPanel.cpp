
#include "Main.h"
#include "WxStuff.h"
#include "BaseResourceArchivesPanel.h"
#include "ArchiveManager.h"

EXTERN_CVAR(Int, base_resource)

BaseResourceArchivesPanel::BaseResourceArchivesPanel(wxWindow* parent)
: wxPanel(parent, -1) {
	// Setup sizer
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	// Init paths list
	//int sel_index = -1;
	list_base_archive_paths = new wxListBox(this, -1);
	for (size_t a = 0; a < theArchiveManager->numBaseResourcePaths(); a++) {
		list_base_archive_paths->Append(theArchiveManager->getBaseResourcePath(a));

		//if (theArchiveManager->baseResourceArchive() && s_cmp(theArchiveManager->baseResourceArchive()->getFilename(true), theArchiveManager->baseResourcePath(a)))
		//	sel_index = a;
	}

	// Select the currently open base archive if any
	if (base_resource >= 0)
		list_base_archive_paths->Select(base_resource);
	//if (sel_index >= 0)
	//	list_base_archive_paths->Select(sel_index);

	// Add paths list
	hbox->Add(list_base_archive_paths, 1, wxEXPAND|wxALL, 4);

	// Setup buttons
	btn_add = new wxButton(this, -1, "Add Archive");
	btn_remove = new wxButton(this, -1, "Remove Archive");

	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	vbox->Add(btn_add, 0, wxEXPAND|wxBOTTOM, 4);
	vbox->Add(btn_remove, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(vbox, 0, wxEXPAND|wxALL, 4);

	// Bind events
	btn_add->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &BaseResourceArchivesPanel::onBtnAdd, this);
	btn_remove->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &BaseResourceArchivesPanel::onBtnRemove, this);

	// Init layout
	Layout();
}

BaseResourceArchivesPanel::~BaseResourceArchivesPanel() {
}

int BaseResourceArchivesPanel::getSelectedPath() {
	return list_base_archive_paths->GetSelection();
}

void BaseResourceArchivesPanel::onBtnAdd(wxCommandEvent& e) {
	// Create extensions string
	string extensions = theArchiveManager->getArchiveExtensionsString();

	// Open a file browser dialog that allows multiple selection
	wxFileDialog dialog_open(this, "Choose file(s) to open", wxEmptyString, wxEmptyString, extensions, wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		// Get an array of selected filenames
		wxArrayString files;
		dialog_open.GetPaths(files);

		// Add each to the paths list
		for (size_t a = 0; a < files.size(); a++) {
			theArchiveManager->addBaseResourcePath(files[a]);
			list_base_archive_paths->Append(files);
		}
	}
}

void BaseResourceArchivesPanel::onBtnRemove(wxCommandEvent& e) {
	// Get the selected item index and remove it
	int index = list_base_archive_paths->GetSelection();
	list_base_archive_paths->Delete(index);

	// Also remove it from the archive manager
	theArchiveManager->removeBaseResourcePath(index);
}
