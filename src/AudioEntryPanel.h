
#ifndef __AUDIO_ENTRY_PANEL_H__
#define __AUDIO_ENTRY_PANEL_H__

#include "EntryPanel.h"
#include <wx/mediactrl.h>

class AudioEntryPanel : public EntryPanel {
private:
	wxMediaCtrl*	media_player;
	string			prevfile;

	wxBitmapButton*	btn_play;
	wxBitmapButton*	btn_pause;
	wxBitmapButton*	btn_stop;

public:
	AudioEntryPanel(wxWindow* parent);
	~AudioEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	// Events
	void	onBtnPlay(wxCommandEvent& e);
	void	onBtnPause(wxCommandEvent& e);
	void	onBtnStop(wxCommandEvent& e);
};

#endif//__AUDIO_ENTRY_PANEL_H__
