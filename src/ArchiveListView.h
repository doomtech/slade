
#ifndef __ARCHIVE_LIST_VIEW_H__
#define __ARCHIVE_LIST_VIEW_H__

#include <wx/dataview.h>
#include "ListenerAnnouncer.h"

class Archive;
class ArchiveListView : public wxDataViewListCtrl, public Listener {
private:
	Archive*	ignore;

public:
	ArchiveListView(wxWindow* parent, Archive* ignore = NULL);
	~ArchiveListView();

	void	populateList();
	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	void	onSize(wxSizeEvent& e);
};

#endif//__ARCHIVE_LIST_VIEW_H__
