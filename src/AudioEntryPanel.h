
#ifndef __AUDIO_ENTRY_PANEL_H__
#define __AUDIO_ENTRY_PANEL_H__

#include "EntryPanel.h"

// Need to undef some stuff for audiere.h to work
#undef vector
#include <audiere.h>
using namespace audiere;
#define vector std::vector

class AudioEntryPanel : public EntryPanel {
private:
	string			prevfile;
	bool			midi;

	OutputStreamPtr	stream;
	MIDIStreamPtr	stream_midi;

	wxBitmapButton*	btn_play;
	wxBitmapButton*	btn_pause;
	wxBitmapButton*	btn_stop;
	wxSlider*		slider_seek;
	wxSlider*		slider_volume;
	wxTimer*		timer_seek;

	// Global audio device
	static AudioDevicePtr	device;
	static MIDIDevicePtr	device_midi;

public:
	AudioEntryPanel(wxWindow* parent);
	~AudioEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	bool	openAudio(string filename);
	bool	openMidi(string filename);
	void	startStream();
	void	stopStream();
	void	resetStream();

	// Events
	void	onBtnPlay(wxCommandEvent& e);
	void	onBtnPause(wxCommandEvent& e);
	void	onBtnStop(wxCommandEvent& e);
	void	onTimer(wxTimerEvent& e);
	void	onSliderSeekChanged(wxCommandEvent& e);
	void	onSliderVolumeChanged(wxCommandEvent& e);
};

#endif//__AUDIO_ENTRY_PANEL_H__
