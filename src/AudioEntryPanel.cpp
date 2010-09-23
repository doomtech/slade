
#include "Main.h"
#include "WxStuff.h"
#include "AudioEntryPanel.h"
#include "Icons.h"
#include <wx/filename.h>
#include <wx/gbsizer.h>
#include <wx/statline.h>

CVAR(Int, snd_volume, 100, CVAR_SAVE)

AudioDevicePtr AudioEntryPanel::device = NULL;
MIDIDevicePtr AudioEntryPanel::device_midi = NULL;

AudioEntryPanel::AudioEntryPanel(wxWindow* parent) : EntryPanel(parent, "audio") {
	// Init variables
	timer_seek = new wxTimer(this);

	// Setup sizer
	wxGridBagSizer* sizer_gb = new wxGridBagSizer(4, 4);
	sizer_main->AddStretchSpacer();
	sizer_main->Add(sizer_gb, 0, wxALIGN_CENTER);
	sizer_main->AddStretchSpacer();

	// Add seekbar
	slider_seek = new wxSlider(this, -1, 0, 0, 0);
	sizer_gb->Add(slider_seek, wxGBPosition(0, 0), wxGBSpan(1, 6), wxEXPAND);

	// Add play controls
	btn_play = new wxBitmapButton(this, -1, getIcon("i_play"));
	sizer_gb->Add(btn_play, wxGBPosition(1, 0));
	btn_pause = new wxBitmapButton(this, -1, getIcon("i_pause"));
	sizer_gb->Add(btn_pause, wxGBPosition(1, 1));
	btn_stop = new wxBitmapButton(this, -1, getIcon("i_stop"));
	sizer_gb->Add(btn_stop, wxGBPosition(1, 2));

	// Separator
	sizer_gb->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), wxGBPosition(1, 3), wxDefaultSpan, wxEXPAND|wxLEFT|wxRIGHT, 8);

	// Add volume slider
	sizer_gb->Add(new wxStaticText(this, -1, "Volume:"), wxGBPosition(1, 4), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	slider_volume = new wxSlider(this, -1, 0, 0, 100);
	slider_volume->SetValue(snd_volume);
	sizer_gb->Add(slider_volume, wxGBPosition(1, 5));

	// Open audio device
	if (!device)
		device = OpenDevice();
	if (!device)
		wxLogMessage("Error: Unable to open audio device, sound playback disabled");

	// Open midi device
	if (!device_midi)
		device_midi = OpenMIDIDevice("");
	if (!device_midi)
		wxLogMessage("Error: Unable to open midi device, midi playback disabled");

	// Disable general entrypanel buttons
	btn_save->Disable();
	btn_revert->Disable();

	// Bind events
	btn_play->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnPlay, this);
	btn_pause->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnPause, this);
	btn_stop->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AudioEntryPanel::onBtnStop, this);
	slider_seek->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &AudioEntryPanel::onSliderSeekChanged, this);
	slider_volume->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &AudioEntryPanel::onSliderVolumeChanged, this);
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
	if (entry->getType()->getFormat() == "midi") {
		openMidi(path.GetFullPath());
		midi = true;
	}
	else {
		openAudio(path.GetFullPath());
		midi = false;
	}

	return true;
}

bool AudioEntryPanel::saveEntry() {
	return true;
}

bool AudioEntryPanel::openAudio(string filename) {
	stream = OpenSound(device, chr(filename), true);
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

		// Set volume
		stream->setVolume((float)snd_volume * 0.01f);

		return true;
	}
	else {
		// Unable to open audio, disable play controls
		btn_play->Enable(false);
		btn_pause->Enable(false);
		btn_stop->Enable(false);

		return false;
	}
}

bool AudioEntryPanel::openMidi(string filename) {
	stream_midi = device_midi->openStream(chr(filename));
	if (stream_midi) {
		// Update seekbar
		slider_seek->Enable();
		slider_seek->SetRange(0, stream_midi->getLength());

		// Enable play controls
		btn_play->Enable();
		btn_pause->Enable();
		btn_stop->Enable();

		return true;
	}
	else {
		// Unable to open midi, disable play controls
		btn_play->Enable(false);
		btn_pause->Enable(false);
		btn_stop->Enable(false);

		return false;
	}
}

void AudioEntryPanel::startStream() {
	if (midi && stream_midi)
		stream_midi->play();
	else if (stream)
		stream->play();
}

void AudioEntryPanel::stopStream() {
	if (midi && stream_midi)
		stream_midi->pause();
	else if (stream)
		stream->stop();
}

void AudioEntryPanel::resetStream() {
	if (midi && stream_midi)
		stream_midi->setPosition(0);
	else if (stream)
		stream->reset();
}


void AudioEntryPanel::onBtnPlay(wxCommandEvent& e) {
	startStream();
	timer_seek->Start(10);
}

void AudioEntryPanel::onBtnPause(wxCommandEvent& e) {
	// Stop playing (no reset)
	stopStream();
	timer_seek->Stop();
}

void AudioEntryPanel::onBtnStop(wxCommandEvent& e) {
	// Stop playing
	stopStream();
	timer_seek->Stop();

	// Reset
	resetStream();
	slider_seek->SetValue(0);
}

void AudioEntryPanel::onTimer(wxTimerEvent& e) {
	// Get current playback position
	int pos = 0;
	if (midi && stream_midi)
		pos = stream_midi->getPosition();
	else if (stream && stream->isSeekable())
		pos = stream->getPosition();

	// Set slider
	slider_seek->SetValue(pos);

	// Stop the timer if playback has reached the end
	if (pos == slider_seek->GetMax())
		timer_seek->Stop();
}

void AudioEntryPanel::onSliderSeekChanged(wxCommandEvent& e) {
	if (midi && stream_midi)
		stream_midi->setPosition(slider_seek->GetValue());
	else if (stream && stream->isSeekable())
		stream->setPosition(slider_seek->GetValue());
}

void AudioEntryPanel::onSliderVolumeChanged(wxCommandEvent& e) {
	snd_volume = slider_volume->GetValue();
	if (stream)
		stream->setVolume((float)snd_volume * 0.01f);
}
