
#ifndef __TEXT_STYLE_H__
#define __TEXT_STYLE_H__

#include "Parser.h"
#include <wx/font.h>
#include <wx/stc/stc.h>

class TextStyle {
	friend class StyleSet;
private:
	string	font;
	int		size;
	rgba_t	foreground;
	bool	fg_defined;
	rgba_t	background;
	bool	bg_defined;
	int		bold;
	int		italic;
	int		underlined;

public:
	TextStyle();
	~TextStyle();

	string	getFontFace() { return font; }
	int		getFontSize() { return size; }
	bool	hasForeground() { return fg_defined; }
	bool	hasBackground() { return bg_defined; }
	int		getBold() { return bold; }
	int		getItalic() { return italic; }
	int		getUnderlined() { return underlined; }

	wxFont	getFont();
	rgba_t	getForeground() { return foreground; }
	rgba_t	getBackground() { return background; }

	bool	parse(ParseTreeNode* node);
	void	applyTo(wxStyledTextCtrl* stc, int style);
	bool	copyStyle(TextStyle* copy);
};

class StyleSet {
private:
	string		name;
	TextStyle	ts_default;
	TextStyle	ts_preprocessor;
	TextStyle	ts_comment;
	TextStyle	ts_string;
	TextStyle	ts_character;
	TextStyle	ts_keyword;
	TextStyle	ts_constant;
	TextStyle	ts_function;
	TextStyle	ts_bracematch;
	TextStyle	ts_bracebad;

public:
	StyleSet();
	~StyleSet();

	string	getName() { return name; }

	bool		parseSet(ParseTreeNode* root);
	void		applyTo(wxStyledTextCtrl* stc);
	bool		copySet(StyleSet* copy);
	TextStyle*	getStyle(string name);

	// Static functions for styleset management
	static void			initCurrent();
	static StyleSet*	currentSet();
	static bool			loadSet(string name);
	static bool			loadSet(unsigned index);
	static void			applyCurrent(wxStyledTextCtrl* stc);
	static string		getName(unsigned index);
	static unsigned		numSets();

	static bool			loadResourceStyles();
	static bool			loadCustomStyles();
};

#endif//__TEXT_STYLE_H__
