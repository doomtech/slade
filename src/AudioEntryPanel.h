
#ifndef __AUDIO_ENTRY_PANEL_H__
#define __AUDIO_ENTRY_PANEL_H__

#include "EntryPanel.h"
#include <wx/mediactrl.h>

#undef Status
#include <SFML/Audio.hpp>

class AudioEntryPanel : public EntryPanel {
private:
	string	prevfile;
	int		audio_type;
	//bool	midi;
	bool	opened;

	wxBitmapButton*	btn_play;
	wxBitmapButton*	btn_pause;
	wxBitmapButton*	btn_stop;
	wxSlider*		slider_seek;
	wxSlider*		slider_volume;
	wxTimer*		timer_seek;
	wxMediaCtrl*	media_ctrl;

	//bool				audio_music;
	sf::SoundBuffer*	sound_buffer;
	sf::Sound			sound;
	sf::Music			music;

	enum {
		AUTYPE_INVALID,
		AUTYPE_SOUND,
		AUTYPE_MUSIC,
		AUTYPE_MIDI,
		AUTYPE_MEDIA,
	};

public:
	AudioEntryPanel(wxWindow* parent);
	~AudioEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	bool	open();
	bool	openAudio(MemChunk& audio, string filename);
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
