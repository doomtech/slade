
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

	wxFont	getFont();
	rgba_t	getForeground();
	rgba_t	getBackground();

	bool	parse(ParseTreeNode* node);
	void	applyTo(wxStyledTextCtrl* stc, int style);
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

	bool	parseSet(ParseTreeNode* root);
	void	applyTo(wxStyledTextCtrl* stc);

	static void			initDefaultStyleSet();
	static bool			readStyleSets(Tokenizer& tz);
	static bool			writeStyleSets(wxFile& file);
	static StyleSet*	getStyleSet(string name);
	static StyleSet*	getStyleSet(unsigned index);
};

#endif//__TEXT_STYLE_H__
