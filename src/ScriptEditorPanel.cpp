
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
	actions.Add("mapw_script_jumpto");
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

	// Bind events
	list_words->Bind(wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, &ScriptEditorPanel::onWordListActivate, this);
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
	wxArrayString functions = tl->getFunctionsSorted();
	wxArrayString constants = tl->getConstantsSorted();

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

void ScriptEditorPanel::openJumpToDialog() {
	// --- Scan for functions/scripts ---
	Tokenizer tz;
	vector<jp_t> jump_points;
	tz.openString(text_editor->GetText());

	string token = tz.getToken();
	while (!token.IsEmpty()) {
		if (token == "{") {
			// Skip block
			while (!token.IsEmpty() && token != "}")
				token = tz.getToken();
		}

		else if (S_CMPNOCASE(token, "script")) {
			// Create jump point
			jp_t jp;
			jp.name = tz.getToken();
			jp.line = tz.lineNo() - 1;

			// Numbered script, add 'Script' to name
			if (jp.name.IsNumber())
				jp.name = S_FMT("Script %s", CHR(jp.name));

			jump_points.push_back(jp);
		}

		else if (S_CMPNOCASE(token, "function")) {
			// Create jump point
			jp_t jp;
			jp.name = tz.getToken();
			jp.line = tz.lineNo() - 1;
			jump_points.push_back(jp);
		}

		token = tz.getToken();
	}

	// Do nothing if no jump points
	if (jump_points.size() == 0)
		return;

	
	// --- Setup/show dialog ---
	wxDialog dlg(this, -1, "Jump To...");
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	dlg.SetSizer(sizer);

	// Add Jump to dropdown
	wxChoice* choice_jump_to = new wxChoice(&dlg, -1);
	sizer->Add(choice_jump_to, 0, wxEXPAND|wxALL, 4);
	for (unsigned a = 0; a < jump_points.size(); a++)
		choice_jump_to->Append(jump_points[a].name);
	choice_jump_to->SetSelection(0);

	// Add dialog buttons
	sizer->Add(dlg.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Show dialog
	dlg.SetInitialSize(wxSize(250, -1));
	dlg.CenterOnParent();
	if (dlg.ShowModal() == wxID_OK) {
		int selection = choice_jump_to->GetSelection();
		if (selection >= 0 && selection < (int)jump_points.size()) {
			// Get line number
			int line = jump_points[selection].line;

			// Move to line
			int pos = text_editor->GetLineEndPosition(line);
			text_editor->SetCurrentPos(pos);
			text_editor->SetSelection(pos, pos);
			text_editor->SetFirstVisibleLine(line);
			text_editor->SetFocus();
		}
	}
}

bool ScriptEditorPanel::handleAction(string name) {
	// Compile Script
	if (name == "mapw_script_compile") {
		EntryOperations::compileACS(entry_script, false, entry_compiled, theMapEditor);

		return true;
	}

	// Save Script
	else if (name == "mapw_script_save") {
		// Write text to entry
		wxCharBuffer buf = text_editor->GetText().mb_str();
		entry_script->importMem(buf, buf.length());
	}

	// Jump To
	else if (name == "mapw_script_jumpto")
		openJumpToDialog();

	return false;
}

void ScriptEditorPanel::onWordListActivate(wxCommandEvent& e) {
	// Get word
	wxTreeListItem item = list_words->GetSelection();
	string word = list_words->GetItemText(item);

	// Get language
	TextLanguage* language = text_editor->getLanguage();
	if (!language)
		return;

	// Check for selection
	if (text_editor->GetSelectionStart() < text_editor->GetSelectionEnd()) {
		// Replace selection with word
		text_editor->ReplaceSelection(word);
		text_editor->SetFocus();
		return;
	}

	// Check for function
	int pos = text_editor->GetCurrentPos();
	if (language->isFunction(word)) {
		TLFunction* func = language->getFunction(word);

		// Add function + ()
		word += "()";
		text_editor->InsertText(pos, word);

		// Move caret inside braces and show calltip
		pos += word.length() - 1;
		text_editor->SetCurrentPos(pos + word.length() - 1);
		text_editor->SetSelection(pos, pos);
		text_editor->updateCalltip();

		text_editor->SetFocus();
	}
	else {
		// Not a function, just add it & move caret position
		text_editor->InsertText(pos, word);
		pos += word.length();
		text_editor->SetCurrentPos(pos);
		text_editor->SetSelection(pos, pos);
		text_editor->SetFocus();
	}
}

/*
void ScriptEditorPanel::onJumpListSelected(wxCommandEvent& e) {
	// Check valid selection
	int selection = choice_jump_to->GetSelection();
	if (selection >= 0 && selection < jump_points.size()) {
		// Get line number
		int line = jump_points[selection].line;

		// Move to line
		int pos = text_editor->GetLineEndPosition(line);
		text_editor->SetCurrentPos(pos);
		text_editor->SetSelection(pos, pos);
		text_editor->SetFirstVisibleLine(line);
		text_editor->SetFocus();
	}
}
*/
