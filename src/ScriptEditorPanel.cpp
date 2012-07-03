
#include "Main.h"
#include "WxStuff.h"
#include "ScriptEditorPanel.h"
#include "Archive.h"
#include "SToolBar.h"
#include "EntryOperations.h"
#include "GameConfiguration.h"
#include "MapEditorWindow.h"

ScriptEditorPanel::ScriptEditorPanel(wxWindow* parent)
: wxPanel(parent, -1) {
	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Toolbar
	SToolBar* toolbar = new SToolBar(this);
	sizer->Add(toolbar, 0, wxEXPAND);

	wxArrayString actions;
	actions.Add("mapw_script_save");
	actions.Add("mapw_script_compile");
	toolbar->addActionGroup("Scripts", actions);

	// Add text editor
	text_editor = new TextEditor(this, -1);
	sizer->Add(text_editor, 1, wxEXPAND|wxALL, 4);

	// Set language
	string lang = theGameConfiguration->scriptLanguage();
	if (S_CMPNOCASE(lang, "acs_hexen"))
		text_editor->setLanguage(TextLanguage::getLanguage("acs"));
	else if (S_CMPNOCASE(lang, "acs_zdoom"))
		text_editor->setLanguage(TextLanguage::getLanguage("acs_z"));
}

ScriptEditorPanel::~ScriptEditorPanel() {
}

bool ScriptEditorPanel::openScripts(ArchiveEntry* entry) {
	if (!entry) {
		entry_script = NULL;
		return false;
	}

	if (text_editor->loadEntry(entry)) {
		entry_script = entry;
		return true;
	}
	else
		return false;
}

bool ScriptEditorPanel::handleAction(string name) {
	if (name == "mapw_script_compile") {
		// Compile
		EntryOperations::compileACS(entry_script, false, entry_compiled, theMapEditor);

		return true;
	}

	else if (name == "mapw_script_save") {
		// Write text to entry
		wxCharBuffer buf = text_editor->GetText().mb_str();
		entry_script->importMem(buf, buf.length());
	}

	return false;
}
