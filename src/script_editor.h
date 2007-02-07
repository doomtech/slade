
#include <wx/wxscintilla.h>

class ScriptView : public wxScintilla
{
private:

public:
	ScriptView(wxWindow *parent);
	~ScriptView();
};

class ScriptEditor : public wxFrame
{
private:
	ScriptView	*script_view;

public:
	ScriptEditor(wxWindow *parent);
	~ScriptEditor();
};
