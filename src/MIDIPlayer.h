
#ifndef __MIDIPLAYER_H__
#define __MIDIPLAYER_H__

// Determine playback library (windows=audiere, other=fluidsynth)
#ifndef WIN32
#define MIDI_LIB_FS
#include <fluidsynth.h>
#else
#include <audiere.h>
#endif

class MIDIPlayer {
private:
	static MIDIPlayer*	instance;

#ifdef MIDI_LIB_FS
	// FluidSynth
	fluid_settings_t*		fs_settings;
	fluid_synth_t*			fs_synth;
	fluid_player_t*			fs_player;
	fluid_audio_driver_t*	fs_adriver;
#else
	// Audiere
	MIDIDevicePtr	device_midi;
	MIDIStreamPtr	stream_midi;
#endif

public:
	MIDIPlayer();
	~MIDIPlayer();

	// Singleton implementation
	static MIDIPlayer*	getInstance() {
		if (!instance)
			instance = new MIDIPlayer();

		return instance;
	}

	bool	openFile(string filename);
	bool	play();
	bool	pause();
	bool	stop();
	bool	isPlaying();
	int		getPosition();
	bool	setPosition(int pos);
	int		getLength();
};

// Define for less cumbersome MIDIPlayer::getInstance()
#define theMIDIPlayer MIDIPlayer::getInstance()

#endif//__MIDIPLAYER_H__
