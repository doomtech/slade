
#ifndef __WADPANEL_H__
#define __WADPANEL_H__

#include "Archive.h"
#include "EntryListPanel.h"
#include "LumpArea.h"
#include "ListenerAnnouncer.h"

class WadPanel : public wxPanel, Listener {
private:
	Archive*		wad;
	EntryListPanel*	lump_list;

	// Lump areas
	LumpArea*		cur_area;
	LumpArea*		lump_area;
	TextLumpArea*	text_area;

public:
	enum {
		ENTRY_LIST_PANEL,
	};

	WadPanel(wxWindow *parent, Archive* wad);
	~WadPanel();

	Archive*	getWad() { return wad; }
	void		save();
	void		saveAs();

	void onAnnouncement(string event_name, MemChunk& event_data);

	// Events
	void	onEntryListChange(wxListEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__WADPANEL_H__
