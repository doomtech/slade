
#ifndef __SCRIPT_EDITOR_PANEL_H__
#define __SCRIPT_EDITOR_PANEL_H__

#include "TextEditor.h"
#include "MainApp.h"

class ArchiveEntry;
class ScriptEditorPanel : public wxPanel, SActionHandler {
private:
	string			script_text;
	ArchiveEntry*	entry_script;
	ArchiveEntry*	entry_compiled;

	TextEditor*	text_editor;
	wxButton*	btn_save;
	wxButton*	btn_compile;

public:
	ScriptEditorPanel(wxWindow* parent);
	~ScriptEditorPanel();

	void	setScriptEntry(ArchiveEntry* entry) { entry_script = entry; }
	void	setCompiledEntry(ArchiveEntry* entry) { entry_compiled = entry; }

	bool	openScripts(ArchiveEntry* entry);
	bool	handleAction(string name);
};

#endif//__SCRIPT_EDITOR_PANEL_H__
