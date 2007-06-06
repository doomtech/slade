
#ifdef USE_WXSCINTILLA

#include <wx/wxscintilla.h>

class ScriptView : public wxScintilla
{
private:

public:
	ScriptView(wxWindow *parent, int id);
	~ScriptView();

	void set_margin_widths();

	// Scintilla Events
	void onTextChanged(wxScintillaEvent &event);

	// Events
	void key_down(wxKeyEvent &event);

	DECLARE_EVENT_TABLE()
};

#else

class ScriptView : public wxTextCtrl
{
private:

public:
	ScriptView(wxWindow *parent, int id);
	~ScriptView();
};

#endif

class ScriptEditor : public wxFrame
{
private:
	ScriptView	*script_view;
	wxButton	*btn_save;
	wxButton	*btn_compile;

public:
	enum
	{
		SE_SCRIPTVIEW,
		SE_BTN_SAVE,
		SE_BTN_COMPILE,
	};

	ScriptEditor(wxWindow *parent);
	~ScriptEditor();

	void load_scripts();

	void close(wxCloseEvent &event);
	void btn_save_clicked(wxCommandEvent &event);
	void btn_compile_clicked(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
