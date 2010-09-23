
#include "Main.h"
#include "WxStuff.h"
#include "AudioEntryPanel.h"
#include "Icons.h"
#include <wx/filename.h>

AudioDevicePtr AudioEntryPanel::device = NULL;

AudioEntryPanel::AudioEntryPanel(wxWindow* parent) : EntryPanel(parent, "audio") {
	// Init variables
	timer_seek = new wxTimer(this);

	// Add play controls
	btn_play = new wxBitmapButton(this, -1, getIcon("i_play"));
	sizer_bottom->Add(btn_play, 0, wxEXPAND|wxRIGHT, 4);
	btn_pause = new wxBitmapButton(this, -1, getIcon("i_pause"));
	sizer_bottom->Add(btn_pause, 0, wxEXPAND|wxRIGHT, 4);
	btn_stop = new wxBitmapButton(this, -1, getIcon("i_stop"));
	sizer_bottom->Add(btn_stop, 0, wxEXPAND|wxRIGHT, 4);

	// Add seekbar
	slider_seek = new wxSlider(this, -1, 0, 0, 0);
	sizer_bottom->Add(slider_seek, 1, wxEXPAND);

	// Open audio device
	if (!device)
		device = OpenDevice();
	if (!device)
		wxLogMessage("Error: Unable to open audio device, sound playback disabled");

	// Bind events
	btn_play->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnPlay, this);
	btn_pause->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnPause, this);
	btn_stop->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnStop, this);
	slider_seek->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &AudioEntryPanel::onSliderSeekChanged, this);
	Bind(wxEVT_TIMER, &AudioEntryPanel::onTimer, this);

	Layout();
}

AudioEntryPanel::~AudioEntryPanel() {
	// Stop the timer to avoid crashes
	timer_seek->Stop();
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
	prevfile = path.GetFullPath();

	// Reset seek slider
	slider_seek->SetValue(0);

	// Open it
	stream = OpenSound(device, path.GetFullPath(), true);
	if (stream) {
		// Check if the audio is seekable
		int length = stream->getLength();
		if (length == 0)
			slider_seek->Enable(false);	// Not seekable, disable seek control
		else {
			slider_seek->Enable();
			slider_seek->SetRange(0, length);
		}

		// Enable play controls
		btn_play->Enable();
		btn_pause->Enable();
		btn_stop->Enable();
	}
	else {
		// Unable to open audio, disable play controls
		btn_play->Enable(false);
		btn_pause->Enable(false);
		btn_stop->Enable(false);
	}

	return true;
}

bool AudioEntryPanel::saveEntry() {
	return true;
}


void AudioEntryPanel::onBtnPlay(wxCommandEvent& e) {
	if (stream->isPlaying()) {
		// Reset
		stream->reset();
		slider_seek->SetValue(0);
	}

	// Play
	stream->play();
	timer_seek->Start(10);
}

void AudioEntryPanel::onBtnPause(wxCommandEvent& e) {
	if (stream) {
		stream->stop();
		timer_seek->Stop();
	}
}

void AudioEntryPanel::onBtnStop(wxCommandEvent& e) {
	if (stream) {
		stream->stop();
		stream->reset();
		timer_seek->Stop();
		slider_seek->SetValue(0);
	}
}

void AudioEntryPanel::onTimer(wxTimerEvent& e) {
	if (stream && stream->isPlaying() && stream->isSeekable())
		slider_seek->SetValue(stream->getPosition());
}

void AudioEntryPanel::onSliderSeekChanged(wxCommandEvent& e) {
	if (stream && stream->isSeekable()) {
		stream->setPosition(slider_seek->GetValue());
	}
}
