
#ifndef __AUDIO_ENTRY_PANEL_H__
#define __AUDIO_ENTRY_PANEL_H__

#include "EntryPanel.h"
#include <wx/mediactrl.h>

class AudioEntryPanel : public EntryPanel {
private:
	wxMediaCtrl*	media_player;
	string			prevfile;

public:
	AudioEntryPanel(wxWindow* parent);
	~AudioEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
};

#endif//__AUDIO_ENTRY_PANEL_H__
