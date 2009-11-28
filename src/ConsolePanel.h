
#ifndef __CONSOLEPANEL_H__
#define __CONSOLEPANEL_H__

#include "ListenerAnnouncer.h"
#include <wx/textctrl.h>

class ConsolePanel : public wxPanel, Listener {
private:
	wxTextCtrl	*text_log;
	wxTextCtrl	*text_command;

public:
	ConsolePanel(wxWindow *parent, int id);
	~ConsolePanel();

	void initLayout();

	// Listener stuff
	void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Events
	void onCommandEnter(wxCommandEvent& e);
};

#endif//__CONSOLEPANEL_H__
