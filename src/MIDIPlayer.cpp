
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MIDIPlayer.cpp
 * Description: MIDIPlayer class, a singleton class that handles
 *              playback of MIDI files. Can only play one MIDI at a
 *              time, and currently contains two separate
 *              implementations: one using audiere for Windows, and
 *              another using fluidsynth for any other OS
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
#include "MIDIPlayer.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
MIDIPlayer*	MIDIPlayer::instance = NULL;
string soundfont = "/usr/share/sounds/sf2/FluidR3_GM.sf2";	// Hard-coded for now


#ifdef MIDI_LIB_FS
/*******************************************************************
 * MIDIPLAYER FLUIDSYNTH IMPLEMENTATION
 *******************************************************************/

/* MIDIPlayer::MIDIPlayer
 * MIDIPlayer class constructor
 *******************************************************************/
MIDIPlayer::MIDIPlayer() {
	// Setup fluidsynth
	fs_settings = new_fluid_settings();
	fluid_settings_setstr(fs_settings, "audio.driver", "alsa");
    fs_synth = new_fluid_synth(fs_settings);
	fluid_synth_sfload(fs_synth, CHR(soundfont), 1);
    fs_player = new_fluid_player(fs_synth);
    fs_adriver = new_fluid_audio_driver(fs_settings, fs_synth);

	if (!fs_player || !fs_adriver)
		wxLogMessage("Warning: Failed to initialise FluidSynth, MIDI playback disabled");
}

/* MIDIPlayer::~MIDIPlayer
 * MIDIPlayer class destructor
 *******************************************************************/
MIDIPlayer::~MIDIPlayer() {
	delete_fluid_audio_driver(fs_adriver);
	delete_fluid_player(fs_player);
	delete_fluid_synth(fs_synth);
	delete_fluid_settings(fs_settings);
}

/* MIDIPlayer::openFile
 * Opens the MIDI file at [filename] for playback. Returns true if
 * successful, false otherwise
 *******************************************************************/
bool MIDIPlayer::openFile(string filename) {
	// Delete+Recreate player
	delete_fluid_player(fs_player);
	fs_player = NULL;
	fs_player = new_fluid_player(fs_synth);

	// Open midi
	if (fs_player) {
		fluid_player_add(fs_player, CHR(filename));
		return true;
	}
	else
		return false;
}

/* MIDIPlayer::play
 * Begins playback of the currently loaded MIDI stream. Returns true
 * if successful, false otherwise
 *******************************************************************/
bool MIDIPlayer::play() {
	return (fluid_player_play(fs_player) == FLUID_OK);
}

/* MIDIPlayer::pause
 * Pauses playback of the currently loaded MIDI stream
 *******************************************************************/
bool MIDIPlayer::pause() {
	// Cannot currently pause in fluidsynth (no seek), so just stop
	fluid_player_stop(fs_player);
	return true;
}

/* MIDIPlayer::stop
 * Stops playback of the currently loaded MIDI stream
 *******************************************************************/
bool MIDIPlayer::stop() {
	fluid_player_stop(fs_player);
	return true;
}

/* MIDIPlayer::isPlaying
 * Returns true if the MIDI stream is currently playing, false if not
 *******************************************************************/
bool MIDIPlayer::isPlaying() {
	return (fluid_player_get_status(fs_player) == FLUID_PLAYER_PLAYING);
}

/* MIDIPlayer::getPosition
 * Returns the current position of the playing MIDI stream
 *******************************************************************/
int MIDIPlayer::getPosition() {
	// Cannot currently seek in fluidsynth
	return 0;
}

/* MIDIPlayer::setPosition
 * Seeks to [pos] in the currently loaded MIDI stream
 *******************************************************************/
bool MIDIPlayer::setPosition(int pos) {
	// Cannot currently seek in fluidsynth
	return false;
}

/* MIDIPlayer::getLength
 * Returns the length (or maximum position) of the currently loaded
 * MIDI stream
 *******************************************************************/
int MIDIPlayer::getLength() {
	// Cannot currently get length in fluidsynth
	return 0;
}


/*******************************************************************
 * MIDIPLAYER AUDIERE IMPLEMENTATION
 *******************************************************************/
#else

/* MIDIPlayer::MIDIPlayer
 * MIDIPlayer class constructor
 *******************************************************************/
MIDIPlayer::MIDIPlayer() {
	// Setup audiere midi
	device_midi = OpenMIDIDevice("");
	if (!device_midi)
		wxLogMessage("Error: Unable to open Audiere MIDI device, MIDI playback disabled");
}

/* MIDIPlayer::~MIDIPlayer
 * MIDIPlayer class destructor
 *******************************************************************/
MIDIPlayer::~MIDIPlayer() {
}

/* MIDIPlayer::openFile
 * Opens the MIDI file at [filename] for playback. Returns true if
 * successful, false otherwise
 *******************************************************************/
bool MIDIPlayer::openFile(string filename) {
	// Check midi device is ok
	if (!device_midi)
		return false;

	// Load the midi
	stream_midi = device_midi->openStream(CHR(filename));

	return !!stream_midi;
}

/* MIDIPlayer::play
 * Begins playback of the currently loaded MIDI stream. Returns true
 * if successful, false otherwise
 *******************************************************************/
bool MIDIPlayer::play() {
	if (stream_midi)
		stream_midi->play();
	return true;
}

/* MIDIPlayer::pause
 * Pauses playback of the currently loaded MIDI stream
 *******************************************************************/
bool MIDIPlayer::pause() {
	if (stream_midi)
		stream_midi->pause();
	return true;
}

/* MIDIPlayer::stop
 * Stops playback of the currently loaded MIDI stream
 *******************************************************************/
bool MIDIPlayer::stop() {
	if (stream_midi)
		stream_midi->stop();
	return true;
}

/* MIDIPlayer::isPlaying
 * Returns true if the MIDI stream is currently playing, false if not
 *******************************************************************/
bool MIDIPlayer::isPlaying() {
	if (stream_midi && stream_midi->isPlaying())
		return true;
	else
		return false;
}

/* MIDIPlayer::getPosition
 * Returns the current position of the playing MIDI stream
 *******************************************************************/
int MIDIPlayer::getPosition() {
	if (stream_midi)
		return stream_midi->getPosition();
	else
		return 0;
}

/* MIDIPlayer::setPosition
 * Seeks to [pos] in the currently loaded MIDI stream
 *******************************************************************/
bool MIDIPlayer::setPosition(int pos) {
	if (stream_midi)
		stream_midi->setPosition(pos);
	return true;
}

/* MIDIPlayer::getLength
 * Returns the length (or maximum position) of the currently loaded
 * MIDI stream
 *******************************************************************/
int MIDIPlayer::getLength() {
	if (stream_midi)
		return stream_midi->getLength();
	else
		return 0;
}

#endif
