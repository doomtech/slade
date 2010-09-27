
#include "Main.h"
#include "MIDIPlayer.h"

MIDIPlayer*	MIDIPlayer::instance = NULL;
string soundfont = "/usr/share/sounds/sf2/FluidR3_GM.sf2";	// Hard-coded for now

// FluidSynth implementation
#ifdef MIDI_LIB_FS

MIDIPlayer::MIDIPlayer() {
	// Setup fluidsynth
	fs_settings = new_fluid_settings();
	fluid_settings_setstr(fs_settings, "audio.driver", "alsa");
    fs_synth = new_fluid_synth(fs_settings);
	fluid_synth_sfload(fs_synth, chr(soundfont), 1);
    fs_player = new_fluid_player(fs_synth);
    fs_adriver = new_fluid_audio_driver(fs_settings, fs_synth);

	if (!fs_player || !fs_adriver)
		wxLogMessage("Warning: Failed to initialise FluidSynth, MIDI playback disabled");
}

MIDIPlayer::~MIDIPlayer() {
	delete_fluid_audio_driver(fs_adriver);
	delete_fluid_player(fs_player);
	delete_fluid_synth(fs_synth);
	delete_fluid_settings(fs_settings);
}

bool MIDIPlayer::openFile(string filename) {
	// Delete+Recreate player
	delete_fluid_player(fs_player);
	fs_player = NULL;
	fs_player = new_fluid_player(fs_synth);

	// Open midi
	if (fs_player) {
		fluid_player_add(fs_player, chr(filename));
		return true;
	}
	else
		return false;
}

bool MIDIPlayer::play() {
	return (fluid_player_play(fs_player) == FLUID_OK);
}

bool MIDIPlayer::pause() {
	fluid_player_stop(fs_player);
	return true;
}

bool MIDIPlayer::stop() {
	fluid_player_stop(fs_player);
	return true;
}

bool MIDIPlayer::isPlaying() {
	return (fluid_player_get_status(fs_player) == FLUID_PLAYER_PLAYING);
}

int MIDIPlayer::getPosition() {
	return 0;
}

bool MIDIPlayer::setPosition(int pos) {
	return false;
}

int MIDIPlayer::getLength() {
	return 0;
}


// Audiere implementation
#else

MIDIPlayer::MIDIPlayer() {
	// Setup audiere midi
	device_midi = OpenMIDIDevice("");
	if (!device_midi)
		wxLogMessage("Error: Unable to open Audiere MIDI device, MIDI playback disabled");
}

MIDIPlayer::~MIDIPlayer() {
}

bool MIDIPlayer::openFile(string filename) {
	// Check midi device is ok
	if (!device_midi)
		return false;

	// Load the midi
	stream_midi = device_midi->openStream(chr(filename));

	return !!stream;
}

bool MIDIPlayer::play() {
	if (stream_midi)
		stream_midi->play();
	return true;
}

bool MIDIPlayer::pause() {
	if (stream_midi)
		stream_midi->pause();
	return true;
}

bool MIDIPlayer::stop() {
	if (stream_midi)
		stream_midi->stop();
	return true;
}

bool MIDIPlayer::isPlaying() {
	if (stream_midi && stream_midi->isPlaying())
		return true;
	else
		return false;
}

int MIDIPlayer::getPosition() {
	if (stream_midi)
		return stream_midi->getPosition();
	else
		return 0;
}

bool MIDIPlayer::setPosition(int pos) {
	if (stream_midi)
		stream_midi->setPosition(pos);
	return true;
}

int MIDIPlayer::getLength() {
	if (stream_midi)
		return stream_midi->getLength();
}

#endif
