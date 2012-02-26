
#ifndef __AUDIO_PREFS_PANEL_H__
#define __AUDIO_PREFS_PANEL_H__

class AudioPrefsPanel : public wxPanel {
private:
	wxCheckBox*	cb_snd_autoplay;
	wxTextCtrl*	text_soundfont_path;
	wxButton*	btn_browse_soundfont;
	wxButton*	btn_reset_player;

public:
	AudioPrefsPanel(wxWindow* parent);
	~AudioPrefsPanel();

	void	applyPreferences();

	// Events
	void	onBtnBrowseSoundfont(wxCommandEvent& e);
	void	onBtnResetPlayer(wxCommandEvent& e);
};

#endif//__AUDIO_PREFS_PANEL_H__
