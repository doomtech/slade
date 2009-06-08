
#ifndef __ZIPARCHIVEPANEL_H__
#define	__ZIPARCHIVEPANEL_H__

#include "ArchivePanel.h"

class ZipArchivePanel : public ArchivePanel {
private:

public:
	ZipArchivePanel(wxWindow *parent, Archive* archive);
	~ZipArchivePanel();

	void			init();

	bool	newEntry();
	bool	newEntryFromFile();

	void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

#endif //__ZIPARCHIVEPANEL_H__
