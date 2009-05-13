
#ifndef __CONSOLEPANEL_H__
#define __CONSOLEPANEL_H__

#include "ListenerAnnouncer.h"
#include <wx/textctrl.h>

class ConsolePanel : public wxPanel, Listener {
private:
	wxTextCtrl	*text_log;
	wxTextCtrl	*text_command;

public:
	// wxWidgets ID table
	enum {
		CP_TEXT_COMMAND,
	};

	ConsolePanel(wxWindow *parent, int id);
	~ConsolePanel();

	void initLayout();

	// wxWidgets event handlers
	void onCommandEnter(wxCommandEvent &e);

	// Listener stuff
	void onAnnouncement(string event_name, MemChunk& event_data);

	DECLARE_EVENT_TABLE()
};

#endif//__CONSOLEPANEL_H__
