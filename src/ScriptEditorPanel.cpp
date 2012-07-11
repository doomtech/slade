
#include "Main.h"
#include "WxStuff.h"
#include "ScriptEditorPanel.h"
#include "Archive.h"
#include "SToolBar.h"
#include "EntryOperations.h"
#include "GameConfiguration.h"
#include "MapEditorWindow.h"
#include <wx/dataview.h>

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
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 1, wxEXPAND);

	text_editor = new TextEditor(this, -1);
	hbox->Add(text_editor, 1, wxEXPAND|wxALL, 4);

	// Set language
	string lang = theGameConfiguration->scriptLanguage();
	if (S_CMPNOCASE(lang, "acs_hexen"))
		text_editor->setLanguage(TextLanguage::getLanguage("acs"));
	else if (S_CMPNOCASE(lang, "acs_zdoom"))
		text_editor->setLanguage(TextLanguage::getLanguage("acs_z"));

	// Add function/constants list
	list_words = new wxTreeListCtrl(this, -1);
	list_words->SetInitialSize(wxSize(200, -10));
	hbox->Add(list_words, 0, wxEXPAND|wxALL, 4);
	populateWordList();
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

void ScriptEditorPanel::populateWordList() {
	// Clear/refresh list
	list_words->DeleteAllItems();
	list_words->ClearColumns();
	list_words->AppendColumn("Language");

	// Get functions and constants
	TextLanguage* tl = TextLanguage::getLanguage("acs_z");
	wxArrayString functions = wxSplit(tl->getFunctionsList(), ' ');
	wxArrayString constants = wxSplit(tl->getConstantsList(), ' ');

	// Add functions to list
	wxTreeListItem item = list_words->AppendItem(list_words->GetRootItem(), "Functions");
	for (unsigned a = 0; a < functions.size()-1; a++) {
		list_words->AppendItem(item, functions[a]);
	}

	// Add constants to list
	item = list_words->AppendItem(list_words->GetRootItem(), "Constants");
	for (unsigned a = 0; a < constants.size()-1; a++) {
		list_words->AppendItem(item, constants[a]);
	}
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
