
#include "Main.h"
#include "WxStuff.h"
#include "TextStyle.h"

vector<StyleSet*>	style_sets;
StyleSet*			ss_default = NULL;

TextStyle::TextStyle() {
	// Default (undefined) values
	font = "";
	size = -1;
	fg_defined = false;
	bg_defined = false;
	bold = -1;
	italic = -1;
	underlined = -1;
}

TextStyle::~TextStyle() {
}

bool TextStyle::parse(ParseTreeNode* node) {
	// Check any info was given
	if (!node)
		return false;

	// Go through info nodes
	for (unsigned a = 0; a < node->nChildren(); a++) {
		ParseTreeNode* child = (ParseTreeNode*)node->getChild(a);
		string name = child->getName();

		// Font name
		if (s_cmpnocase(name, "font"))
			font = child->getStringValue();

		// Font size
		if (s_cmpnocase(name, "size"))
			size = child->getIntValue();

		// Foreground colour
		if (s_cmpnocase(name, "foreground")) {
			foreground.set(child->getIntValue(0), child->getIntValue(1), child->getIntValue(2), 255);
			fg_defined = true;
		}

		// Background colour
		if (s_cmpnocase(name, "background")) {
			background.set(child->getIntValue(0), child->getIntValue(1), child->getIntValue(2), 255);
			bg_defined = true;
		}

		// Bold
		if (s_cmpnocase(name, "bold"))
			bold = (int)child->getBoolValue();

		// Italic
		if (s_cmpnocase(name, "italic"))
			italic = (int)child->getBoolValue();

		// Underlined
		if (s_cmpnocase(name, "underlined"))
			underlined = (int)child->getBoolValue();
	}

	return true;
}

void TextStyle::applyTo(wxStyledTextCtrl* stc, int style) {
	// Set font face
	if (!font.IsEmpty())
		stc->StyleSetFaceName(style, font);

	// Set font size
	if (size > 0)
		stc->StyleSetSize(style, size);

	// Set foreground
	if (fg_defined)
		stc->StyleSetForeground(style, WXCOL(foreground));

	// Set background
	if (bg_defined)
		stc->StyleSetBackground(style, WXCOL(background));

	// Set bold
	if (bold > 0)
		stc->StyleSetBold(style, true);
	else if (bold == 0)
		stc->StyleSetBold(style, false);

	// Set italic
	if (italic > 0)
		stc->StyleSetItalic(style, true);
	else if (italic == 0)
		stc->StyleSetItalic(style, false);

	// Set underlined
	if (underlined > 0)
		stc->StyleSetUnderline(style, true);
	else if (underlined == 0)
		stc->StyleSetUnderline(style, false);
}



StyleSet::StyleSet() {
}

StyleSet::~StyleSet() {
}

bool StyleSet::parseSet(ParseTreeNode* root) {
	if (!root)
		return false;

	// Get name
	ParseTreeNode* node = (ParseTreeNode*)root->getChild("name");
	if (node)
		name = node->getName();
	else
		name = "Unnamed Style";

	// Init default style
	wxFont f(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	ts_default.font = f.GetFaceName();
	ts_default.size = 10;
	ts_default.foreground.set(0, 0, 0, 255);
	ts_default.fg_defined = true;
	ts_default.background.set(255, 255, 255, 255);
	ts_default.bg_defined = true;
	ts_default.bold = 0;
	ts_default.italic = 0;
	ts_default.underlined = 0;

	// Parse styles
	ts_default.parse((ParseTreeNode*)root->getChild("default"));			// Default style
	ts_preprocessor.parse((ParseTreeNode*)root->getChild("preprocessor"));	// Default style
	ts_comment.parse((ParseTreeNode*)root->getChild("comment"));			// Comment style
	ts_string.parse((ParseTreeNode*)root->getChild("string"));				// String style
	ts_character.parse((ParseTreeNode*)root->getChild("character"));		// Character style
	ts_keyword.parse((ParseTreeNode*)root->getChild("keyword"));			// Keyword style
	ts_constant.parse((ParseTreeNode*)root->getChild("constant"));			// Constant style
	ts_function.parse((ParseTreeNode*)root->getChild("function"));			// Function style
	ts_bracematch.parse((ParseTreeNode*)root->getChild("bracematch"));		// Brace-match style
	ts_bracebad.parse((ParseTreeNode*)root->getChild("bracebad"));			// Brace-mismatch style

	return true;
}

void StyleSet::applyTo(wxStyledTextCtrl* stc) {
	// Set default style
	ts_default.applyTo(stc, wxSTC_STYLE_DEFAULT);

	// Apply default style to all
	stc->StyleClearAll();

	// Apply other styles
	ts_preprocessor.applyTo(stc, wxSTC_C_PREPROCESSOR);
	ts_comment.applyTo(stc, wxSTC_C_COMMENT);
	ts_comment.applyTo(stc, wxSTC_C_COMMENTLINE);
	ts_string.applyTo(stc, wxSTC_C_STRING);
	ts_character.applyTo(stc, wxSTC_C_CHARACTER);
	ts_keyword.applyTo(stc, wxSTC_C_WORD);
	ts_constant.applyTo(stc, wxSTC_C_GLOBALCLASS);
	ts_function.applyTo(stc, wxSTC_C_WORD2);
	ts_bracematch.applyTo(stc, wxSTC_STYLE_BRACELIGHT);
	ts_bracebad.applyTo(stc, wxSTC_STYLE_BRACEBAD);
}

void StyleSet::initDefaultStyleSet() {
	// If it exists no need to recreate it
	if (ss_default)
		return;

	// Create default style set
	ss_default = new StyleSet();

	// Setup default values
	ss_default->name = "SLADE Default";

	// Default style
	ss_default->ts_default.font = "";
	ss_default->ts_default.size = 10;
	ss_default->ts_default.foreground.set(0, 0, 0, 255);
	ss_default->ts_default.fg_defined = true;
	ss_default->ts_default.background.set(255, 255, 255, 255);
	ss_default->ts_default.bg_defined = true;
	ss_default->ts_default.bold = false;
	ss_default->ts_default.italic = false;
	ss_default->ts_default.underlined = false;

	// Preprocessor
	ss_default->ts_preprocessor.foreground.set(0, 100, 200, 255);
	ss_default->ts_preprocessor.fg_defined = true;

	// Comment
	ss_default->ts_comment.foreground.set(0, 150, 0, 255);
	ss_default->ts_comment.fg_defined = true;

	// String
	ss_default->ts_string.foreground.set(0, 120, 130, 255);
	ss_default->ts_string.fg_defined = true;

	// Character
	ss_default->ts_character.foreground.set(0, 120, 130, 255);
	ss_default->ts_character.fg_defined = true;

	// Keyword
	ss_default->ts_keyword.foreground.set(0, 30, 200, 255);
	ss_default->ts_keyword.fg_defined = true;

	// Constant
	ss_default->ts_constant.foreground.set(180, 30, 200, 255);
	ss_default->ts_constant.fg_defined = true;

	// Function
	ss_default->ts_function.foreground.set(200, 100, 30, 255);
	ss_default->ts_function.fg_defined = true;

	// Brace match
	ss_default->ts_bracematch.background.set(170, 255, 170, 255);
	ss_default->ts_bracematch.bg_defined = true;

	// Brace mismatch
	ss_default->ts_bracebad.background.set(255, 170, 170, 255);
	ss_default->ts_bracebad.bg_defined = true;
}

bool StyleSet::readStyleSets(Tokenizer& tz) {
	return false;
}

bool StyleSet::writeStyleSets(wxFile& file) {
	return false;
}

StyleSet* StyleSet::getStyleSet(string name) {
	for (unsigned a = 0; a < style_sets.size(); a++) {
		if (s_cmpnocase(style_sets[a]->name, name))
			return style_sets[a];
	}

	return ss_default;
}

StyleSet* StyleSet::getStyleSet(unsigned index) {
	return ss_default;
}
