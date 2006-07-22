
#include "wx_stuff.h"

class ConsoleEntry : public wxTextCtrl
{
private:

public:
	ConsoleEntry(wxWindow* parent, int id);
	~ConsoleEntry();

	void key_down(wxKeyEvent &event);

	DECLARE_EVENT_TABLE()
};


class ConsoleWindow : public wxFrame
{
private:

public:
	enum
	{
		CW_ENTRY_COMMAND,
	};

	ConsoleWindow();
	~ConsoleWindow();

	wxTextCtrl		*log;
	ConsoleEntry	*entry;

	void update_log();

	void close(wxCloseEvent& event);
	void cmd_activated(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
