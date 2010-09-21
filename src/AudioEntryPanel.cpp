
#include "Main.h"
#include "WxStuff.h"
#include "AudioEntryPanel.h"
#include "Icons.h"
#include <wx/filename.h>

AudioEntryPanel::AudioEntryPanel(wxWindow* parent) : EntryPanel(parent, "audio") {
	// Add media player control
	media_player = new wxMediaCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
	media_player->ShowPlayerControls();
	sizer_main->Add(media_player, 1, wxEXPAND);

	// Add play controls if needed
#ifdef __WXGTK__
	btn_play = new wxBitmapButton(this, -1, getIcon("i_play"));
	sizer_bottom->Add(btn_play, 0, wxEXPAND|wxRIGHT, 4);
	btn_pause = new wxBitmapButton(this, -1, getIcon("i_pause"));
	sizer_bottom->Add(btn_pause, 0, wxEXPAND|wxRIGHT, 4);
	btn_stop = new wxBitmapButton(this, -1, getIcon("i_stop"));
	sizer_bottom->Add(btn_stop, 0, wxEXPAND|wxRIGHT, 4);

	btn_play->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnPlay, this);
	btn_pause->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnPause, this);
	btn_stop->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnStop, this);
#endif

	Layout();
}

AudioEntryPanel::~AudioEntryPanel() {
}

bool AudioEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Delete previous temp file
	if (wxFileExists(prevfile))
		wxRemoveFile(prevfile);

	// Dump entry to temp file
	wxFileName path(appPath(entry->getName(), DIR_TEMP));

	// Add extension if missing
	if (path.GetExt().IsEmpty())
		path.SetExt(entry->getType()->getExtension());

	entry->exportFile(path.GetFullPath());
	media_player->Load(path.GetFullPath());
	prevfile = path.GetFullPath();

	return true;
}

bool AudioEntryPanel::saveEntry() {
	return true;
}


void AudioEntryPanel::onBtnPlay(wxCommandEvent& e) {
	media_player->Play();
}

void AudioEntryPanel::onBtnPause(wxCommandEvent& e) {
	media_player->Pause();
}

void AudioEntryPanel::onBtnStop(wxCommandEvent& e) {
	media_player->Stop();
}
