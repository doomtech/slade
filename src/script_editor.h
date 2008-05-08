
#include <wx/wxscintilla.h>

class ScriptText : public wxScintilla
{
private:

public:
	ScriptText(wxWindow *parent, int id);
	~ScriptText();

	void set_margin_widths();
	void setup_style(int style, string name);
	void char_added(wxScintillaEvent &event);

	void key_down(wxKeyEvent &event);

	DECLARE_EVENT_TABLE()
};

class ScriptEditor : public wxFrame
{
private:
	//wxTextCtrl	*text_script;
	ScriptText	*text_script;
	wxButton	*btn_savechanges;
	wxButton	*btn_compile;

public:
	enum
	{
		SE_BTN_SAVECHANGES,
		SE_BTN_COMPILE,
		SE_TEXT_SCRIPT,
	};

	ScriptEditor(wxWindow *parent);
	~ScriptEditor();

	void close(wxCloseEvent &event);
	void save_changes_clicked(wxCommandEvent &event);
	void compile_clicked(wxCommandEvent &event);
	void text_changed(wxScintillaEvent &event);

	DECLARE_EVENT_TABLE()
};

struct style_t
{
	string name;
	BYTE fred, fgreen, fblue;
	BYTE bred, bgreen, bblue;
	BYTE flags;
};

#define STYLE_BOLD		0x01
#define STYLE_ITALIC	0x02
#define STYLE_UNDERLINE	0x04
#define STYLE_UPPERCASE	0x08
#define STYLE_LOWERCASE	0x10
