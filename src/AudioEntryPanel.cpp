
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    AudioEntryPanel.cpp
 * Description: AudioEntryPanel class. The UI for previewing supported
 *              audio entries
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "AudioEntryPanel.h"
#include "Icons.h"
#include "Conversions.h"
#include "MIDIPlayer.h"
#include <wx/filename.h>
#include <wx/gbsizer.h>
#include <wx/statline.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Int, snd_volume, 100, CVAR_SAVE)
CVAR(Bool, snd_autoplay, false, CVAR_SAVE)


/*******************************************************************
 * AUDIOENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* AudioEntryPanel::AudioEntryPanel
 * AudioEntryPanel class constructor
 *******************************************************************/
AudioEntryPanel::AudioEntryPanel(wxWindow* parent) : EntryPanel(parent, "audio") {
	// Init variables
	timer_seek = new wxTimer(this);
	sound_buffer = NULL;
	audio_type = AUTYPE_INVALID;

	// Create wxMediaCtrl
	media_ctrl = new wxMediaCtrl(this, -1);

	// Setup sizer
	wxGridBagSizer* sizer_gb = new wxGridBagSizer(4, 4);
	//sizer_main->AddStretchSpacer();
	sizer_main->Add(media_ctrl, 1, wxBOTTOM|wxALIGN_CENTER, 4);
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
	slider_volume = new wxSlider(this, -1, 0, 0, 100, wxDefaultPosition, wxSize(128, -1));
	slider_volume->SetValue(snd_volume);
	sizer_gb->Add(slider_volume, wxGBPosition(1, 5));

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

/* AudioEntryPanel::~AudioEntryPanel
 * AudioEntryPanel class constructor
 *******************************************************************/
AudioEntryPanel::~AudioEntryPanel() {
	// Stop the timer to avoid crashes
	timer_seek->Stop();
}

/* AudioEntryPanel::loadEntry
 * Loads an entry into the audio entry panel
 *******************************************************************/
bool AudioEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Stop anything currently playing
	stopStream();
	opened = false;

	// Enable all playback controls initially
	slider_seek->Enable();
	btn_play->Enable();
	btn_pause->Enable();
	btn_stop->Enable();

	// Reset seek slider
	slider_seek->SetValue(0);
	slider_seek->SetRange(0, 0);
	slider_seek->Show();

	// Delete previous temp file
	if (wxFileExists(prevfile))
		wxRemoveFile(prevfile);

	// Autoplay if option is on
	if (snd_autoplay) {
		this->entry = entry;
		startStream();
		timer_seek->Start(10);
	}

	return true;
}

/* AudioEntryPanel::saveEntry
 * Saves any changes to the entry (does nothing here)
 *******************************************************************/
bool AudioEntryPanel::saveEntry() {
	return true;
}

/* AudioEntryPanel::open
 * Opens the current entry and performs the appropriate conversions
 *******************************************************************/
bool AudioEntryPanel::open() {
	// Check if already opened
	if (opened)
		return true;

	// Get entry data
	MemChunk& mcdata = entry->getMCData();

	// Setup temp filename
	wxFileName path(appPath(entry->getName(), DIR_TEMP));
	// Add extension if missing
	if (path.GetExt().IsEmpty())
		path.SetExt(entry->getType()->getExtension());

	// Convert if necessary, then write to file
	MemChunk convdata;
	if (entry->getType()->getFormat() == "snd_doom")			// Doom Sound -> WAV
		Conversions::doomSndToWav(mcdata, convdata);
	else if (entry->getType()->getFormat() == "snd_voc")		// Creative Voice File -> WAV
		Conversions::vocToWav(mcdata, convdata);
	else if (entry->getType()->getFormat() == "snd_bloodsfx")	// Creative Voice File -> WAV
		Conversions::bloodToWav(entry, convdata);
	else if (entry->getType()->getFormat() == "mus") {			// MUS -> MIDI
		Conversions::musToMidi(mcdata, convdata);
		path.SetExt("mid");
	}
	else if (entry->getType()->getFormat() == "gmid") {			// GMID -> MIDI
		Conversions::gmidToMidi(mcdata, convdata);
		path.SetExt("mid");
	}
	else
		convdata.importMem(mcdata.getData(), mcdata.getSize());

	if (entry->getType()->getFormat() == "midi" || entry->getType()->getFormat() == "mus" ||
		entry->getType()->getFormat() == "gmid") {
		convdata.exportFile(path.GetFullPath());
		//openAudio(convdata, path.GetFullPath());
		openMidi(path.GetFullPath());
		audio_type = AUTYPE_MIDI;
	}
	else
		openAudio(convdata, path.GetFullPath());

	// Keep filename so we can delete it later
	prevfile = path.GetFullPath();

	opened = true;
	return true;
}

/* AudioEntryPanel::openAudio
 * Opens an audio file for playback
 *******************************************************************/
bool AudioEntryPanel::openAudio(MemChunk& audio, string filename) {
	// Stop if sound currently playing
	sound.Stop();
	music.Stop();

	// (Re)create sound buffer
	if (sound_buffer)
		delete sound_buffer;
	sound_buffer = new sf::SoundBuffer();
	audio_type = AUTYPE_INVALID;

	// Load into buffer
	if (sound_buffer->LoadFromMemory((const char*)audio.getData(), audio.getSize())) {
		// Bind to sound
		sound.SetBuffer(*sound_buffer);
		audio_type = AUTYPE_SOUND;

		// Enable play controls
		slider_seek->Enable();
		slider_seek->SetRange(0, sound_buffer->GetDuration()*100);
		btn_play->Enable();
		btn_pause->Enable();
		btn_stop->Enable();

		return true;
	}
	else if (music.OpenFromMemory((const char*)audio.getData(), audio.getSize())) {
		// Couldn't open the audio as a sf::SoundBuffer, try sf::Music instead
		audio_type = AUTYPE_MUSIC;

		// Enable play controls
		btn_play->Enable();
		btn_stop->Enable();

		// sf::Music can't seek or pause
		slider_seek->Enable(false);
		btn_pause->Enable(false);

		return true;
	}
	else {
		// Couldn't open as sound or music, try the wxMediaCtrl

		// Dump audio to temp file
		audio.exportFile(filename);

		// Attempt to open with wxMediaCtrl
		if (media_ctrl->Load(filename)) {
			// Loaded successfully
			audio_type = AUTYPE_MEDIA;

			// Enable play controls
			slider_seek->Enable(true);
			slider_seek->SetRange(0, media_ctrl->Length());
			btn_play->Enable(true);
			btn_pause->Enable(true);
			btn_stop->Enable(true);

			return true;
		}
	}
	
	// Unable to open audio, disable play controls
	slider_seek->Enable(false);
	btn_play->Enable(false);
	btn_pause->Enable(false);
	btn_stop->Enable(false);

	return false;
}

/* AudioEntryPanel::openMidi
 * Opens a MIDI file for playback
 *******************************************************************/
bool nosf_warned = false;	// One-time 'no soundfont loaded' warning
bool AudioEntryPanel::openMidi(string filename) {
	// Enable volume control
	slider_volume->Enable(true);

	// Disable controls if we cannot play the midi
	if (!theMIDIPlayer->isInitialised()) {
		btn_play->Enable(false);
		btn_pause->Enable(false);
		btn_stop->Enable(false);
		slider_seek->Enable(false);

		return false;
	}

	// Warn if no soundfont is loaded
	if (!theMIDIPlayer->isSoundfontLoaded()) {
		// Disable play controls
		btn_play->Enable(false);
		btn_pause->Enable(false);
		btn_stop->Enable(false);
		slider_seek->Enable(false);

		// Popup message
		if (!nosf_warned) {
			wxMessageBox("No soundfont is currently set up for playing MIDIs. See the audio settings in Editor->Preferences", "Can't play MIDI", wxICON_ERROR);
			nosf_warned = true;
		}

		return false;
	}

	// Attempt to open midi
	if (theMIDIPlayer->openFile(filename)) {
		// Enable play controls
		btn_play->Enable();
		btn_pause->Enable();
		btn_stop->Enable();

		// Setup seekbar
		int length = theMIDIPlayer->getLength();
		if (length > 0) {
			slider_seek->Enable();
			slider_seek->SetRange(0, length);
		}

		return true;
	}
	else {
		// Disable play controls
		btn_play->Enable(false);
		btn_pause->Enable(false);
		btn_stop->Enable(false);
		slider_seek->Enable(false);

		return false;
	}
}

/* AudioEntryPanel::startStream
 * Begins playback of the current audio or MIDI stream
 *******************************************************************/
void AudioEntryPanel::startStream() {
	if (!opened)
		open();

	/*
	if (midi)
		theMIDIPlayer->play();
	else {
		if (audio_music)
			music.Play();
		else
			sound.Play();
	}
	*/

	switch (audio_type) {
	case AUTYPE_SOUND:
		sound.Play(); break;
	case AUTYPE_MUSIC:
		music.Play(); break;
	case AUTYPE_MIDI:
		theMIDIPlayer->play(); break;
	case AUTYPE_MEDIA:
		media_ctrl->Play(); break;
	}
}

/* AudioEntryPanel::stopStream
 * Stops playback of the current audio or MIDI stream
 *******************************************************************/
void AudioEntryPanel::stopStream() {
	/*
	if (midi)
		theMIDIPlayer->pause();
	else if (!audio_music)
		sound.Pause();
	*/

	switch (audio_type) {
	case AUTYPE_SOUND:
		sound.Pause(); break;
	case AUTYPE_MUSIC:
		music.Pause(); break;
	case AUTYPE_MIDI:
		theMIDIPlayer->pause(); break;
	case AUTYPE_MEDIA:
		media_ctrl->Pause(); break;
	}
}

/* AudioEntryPanel::resetStream
 * Resets the current audio or MIDI stream to the beginning
 *******************************************************************/
void AudioEntryPanel::resetStream() {
	/*
	if (midi)
		theMIDIPlayer->stop();
	else {
		if (audio_music)
			music.Stop();
		else
			sound.Stop();
	}
	*/

	switch (audio_type) {
	case AUTYPE_SOUND:
		sound.Stop(); break;
	case AUTYPE_MUSIC:
		music.Stop(); break;
	case AUTYPE_MIDI:
		theMIDIPlayer->stop(); break;
	case AUTYPE_MEDIA:
		media_ctrl->Stop(); break;
	}
}


/*******************************************************************
 * AUDIOENTRYPANEL CLASS EVENTS
 *******************************************************************/

/* AudioEntryPanel::onBtnPlay
 * Called when the play button is pressed
 *******************************************************************/
void AudioEntryPanel::onBtnPlay(wxCommandEvent& e) {
	startStream();
	timer_seek->Start(10);
}

/* AudioEntryPanel::onBtnPause
 * Called when the pause button is pressed
 *******************************************************************/
void AudioEntryPanel::onBtnPause(wxCommandEvent& e) {
	// Stop playing (no reset)
	stopStream();
	timer_seek->Stop();
}

/* AudioEntryPanel::onBtnStop
 * Called when the stop button is pressed
 *******************************************************************/
void AudioEntryPanel::onBtnStop(wxCommandEvent& e) {
	// Stop playing
	stopStream();
	timer_seek->Stop();

	// Reset
	resetStream();
	slider_seek->SetValue(0);
}

/* AudioEntryPanel::onTimer
 * Called when the playback timer ticks
 *******************************************************************/
void AudioEntryPanel::onTimer(wxTimerEvent& e) {
	// Get current playback position
	int pos = 0;
	/*
	if (midi)
		pos = theMIDIPlayer->getPosition();
	else
		pos = sound.GetPlayingOffset() * 100;
	*/
	switch (audio_type) {
	case AUTYPE_SOUND:
		pos = sound.GetPlayingOffset() * 100; break;
	case AUTYPE_MIDI:
		pos = theMIDIPlayer->getPosition(); break;
	case AUTYPE_MEDIA:
		pos = media_ctrl->Tell(); break;
	}

	// Set slider
	slider_seek->SetValue(pos);

	// Stop the timer if playback has reached the end
	if (pos >= slider_seek->GetMax() || 
		(audio_type == AUTYPE_SOUND && sound.GetStatus() == sf::Sound::Stopped) ||
		(audio_type == AUTYPE_MUSIC && music.GetStatus() == sf::Sound::Stopped) ||
		(audio_type == AUTYPE_MEDIA && media_ctrl->GetState() == wxMEDIASTATE_STOPPED))
		timer_seek->Stop();
}

/* AudioEntryPanel::onSliderSeekChanged
 * Called when the seek slider position is changed
 *******************************************************************/
void AudioEntryPanel::onSliderSeekChanged(wxCommandEvent& e) {
	/*
	if (midi)
		theMIDIPlayer->setPosition(slider_seek->GetValue());
	else
		sound.SetPlayingOffset(slider_seek->GetValue() * 0.01);
	*/

	switch (audio_type) {
	case AUTYPE_SOUND:
		sound.SetPlayingOffset(slider_seek->GetValue() * 0.01); break;
	case AUTYPE_MIDI:
		theMIDIPlayer->setPosition(slider_seek->GetValue()); break;
	case AUTYPE_MEDIA:
		media_ctrl->Seek(slider_seek->GetValue()); break;
	}
}

/* AudioEntryPanel::onSliderVolumeChanged
 * Called when the volume slider position is changed
 *******************************************************************/
void AudioEntryPanel::onSliderVolumeChanged(wxCommandEvent& e) {
	snd_volume = slider_volume->GetValue();
	/*
	if (midi)
		theMIDIPlayer->setVolume(snd_volume);
	else
		sound.SetVolume(snd_volume);
	*/

	switch (audio_type) {
	case AUTYPE_SOUND:
		sound.SetVolume(snd_volume); break;
	case AUTYPE_MUSIC:
		music.SetVolume(snd_volume); break;
	case AUTYPE_MIDI:
		theMIDIPlayer->setVolume(snd_volume); break;
	case AUTYPE_MEDIA:
		media_ctrl->SetVolume(snd_volume*0.01); break;
	}
}
