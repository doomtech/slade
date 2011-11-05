
#include "Main.h"
#include "WxStuff.h"
#include "ArchiveListView.h"
#include "ArchiveManager.h"

ArchiveListView::ArchiveListView(wxWindow* parent, Archive* ignore) : wxDataViewListCtrl(parent, -1) {
	// Init variables
	this->ignore = ignore;

	// Add columns
	//SetExpanderColumn(NULL);
	AppendToggleColumn("Resource");
	AppendTextColumn("Archive", wxDATAVIEW_CELL_INERT, -1);
	GetColumn(1)->SetWidth(200);

	// Add archives
	populateList();

	// Bind events
	Bind(wxEVT_SIZE, &ArchiveListView::onSize, this);

	listenTo(theArchiveManager);
}

ArchiveListView::~ArchiveListView() {
}

void ArchiveListView::populateList() {
	// Clear
	DeleteAllItems();

	// Populate
	for (unsigned a = 0; a < theArchiveManager->numArchives(); a++) {
		Archive* archive = theArchiveManager->getArchive(a);
		if (archive == ignore) continue;

		wxVector<wxVariant> item;
		item.push_back(theArchiveManager->archiveIndex(archive));
		item.push_back(archive->getFilename(false));
		AppendItem(item);
	}
}

void ArchiveListView::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Refresh list on any announcement
	populateList();
}



void ArchiveListView::onSize(wxSizeEvent& e) {
	// Resize columns (expand archive column)
	int width = GetSize().x;
	int cwidth = width - GetColumn(0)->GetWidth();
	GetColumn(1)->SetWidth(cwidth);
}
