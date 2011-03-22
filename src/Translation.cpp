
#include "Main.h"
#include "Translation.h"
#include "Tokenizer.h"
#include "Math.h"

Translation::Translation() {
}

Translation::~Translation() {
	for (unsigned a = 0; a < translations.size(); a++)
		delete translations[a];
}

void Translation::parse(string def) {
	// Open definition string for processing w/tokenizer
	Tokenizer tz;
	tz.setSpecialCharacters("[]:%,=");
	tz.openString(def);

	wxLogMessage("Parse translation \"%s\"", CHR(def));

	// Read original range
	uint8_t o_start = tz.getInteger();
	if (!tz.checkToken(":")) return;
	uint8_t o_end = tz.getInteger();
	if (!tz.checkToken("=")) return;

	// Type of translation depends on next token
	if (tz.peekToken() == "[") {
		// Colour translation
		rgba_t start, end;

		tz.getToken();	// Skip [

		// Read start colour
		start.r = tz.getInteger();
		if (!tz.checkToken(",")) return;
		start.g = tz.getInteger();
		if (!tz.checkToken(",")) return;
		start.b = tz.getInteger();

		if (!tz.checkToken("]")) return;
		if (!tz.checkToken(":")) return;
		if (!tz.checkToken("[")) return;

		// Read end colour
		end.r = tz.getInteger();
		if (!tz.checkToken(",")) return;
		end.g = tz.getInteger();
		if (!tz.checkToken(",")) return;
		end.b = tz.getInteger();

		if (!tz.checkToken("]")) return;

		// Add translation
		TransRangeColour* tr = new TransRangeColour();
		tr->o_start = o_start;
		tr->o_end = o_end;
		tr->d_start.set(start);
		tr->d_end.set(end);
		translations.push_back(tr);

		wxLogMessage("Added colour translation");
	}
	else if (tz.peekToken() == "%") {
		// Desat colour translation
		float sr, sg, sb, er, eg, eb;

		tz.getToken();	// Skip %
		if (!tz.checkToken("[")) return;

		// Read start colour
		sr = tz.getFloat();
		if (!tz.checkToken(",")) return;
		sg = tz.getFloat();
		if (!tz.checkToken(",")) return;
		sb = tz.getFloat();

		if (!tz.checkToken("]")) return;
		if (!tz.checkToken(":")) return;
		if (!tz.checkToken("[")) return;

		// Read end colour
		er = tz.getFloat();
		if (!tz.checkToken(",")) return;
		eg = tz.getFloat();
		if (!tz.checkToken(",")) return;
		eb = tz.getFloat();

		if (!tz.checkToken("]")) return;

		// Add translation
		TransRangeDesat* tr = new TransRangeDesat();
		tr->o_start = o_start;
		tr->o_end = o_end;
		tr->d_sr = sr;
		tr->d_sg = sg;
		tr->d_sb = sb;
		tr->d_er = er;
		tr->d_eg = eg;
		tr->d_eb = eb;
		translations.push_back(tr);

		wxLogMessage("Added desat translation");
	}
	else {
		// Palette range translation
		uint8_t d_start, d_end;

		// Read range values
		d_start = tz.getInteger();
		if (!tz.checkToken(":")) return;
		d_end = tz.getInteger();

		// Add translation
		TransRangePalette* tr = new TransRangePalette();
		tr->o_start = o_start;
		tr->o_end = o_end;
		tr->d_start = d_start;
		tr->d_end = d_end;
		translations.push_back(tr);

		wxLogMessage("Added range translation");
	}
}

string Translation::asText() {
	string ret;

	// Go through translation ranges
	for (unsigned a = 0; a < translations.size(); a++) {
		// Palette range translation
		if (translations[a]->type == TRANS_PALETTE) {
			TransRangePalette* tr = (TransRangePalette*)translations[a];
			ret += S_FMT("\"%d:%d=%d:%d\", ", tr->o_start, tr->o_end, tr->d_start, tr->d_end);
		}

		// Colour range translation
		else if (translations[a]->type == TRANS_COLOUR) {
			TransRangeColour* tr = (TransRangeColour*)translations[a];
			ret += S_FMT("\"%d:%d=[%d,%d,%d]:[%d,%d,%d]\", ",
						tr->o_start, tr->o_end,
						tr->d_start.r, tr->d_start.g, tr->d_start.b,
						tr->d_end.r, tr->d_end.g, tr->d_end.b);
		}

		// Desaturated colour range translation
		else if (translations[a]->type == TRANS_DESAT) {
			TransRangeDesat* tr = (TransRangeDesat*)translations[a];
			ret += S_FMT("\"%d:%d=%%[%1.2f,%1.2f,%1.2f]:[%1.2f,%1.2f,%1.2f]\", ",
						tr->o_start, tr->o_end,
						tr->d_sr, tr->d_sg, tr->d_sb,
						tr->d_er, tr->d_eg, tr->d_eb);
		}
	}

	// If any translations were defined, remove last ", "
	if (!ret.IsEmpty())
		ret.RemoveLast(2);

	return ret;
}

void Translation::clear() {
	for (unsigned a = 0; a < translations.size(); a++)
		delete translations[a];
	translations.clear();
}

void Translation::copy(Translation& copy) {
	// Clear current definitions
	clear();

	// Copy translations
	for (unsigned a = 0; a < copy.translations.size(); a++) {
		if (copy.translations[a]->type == TRANS_PALETTE)
			translations.push_back(new TransRangePalette((TransRangePalette*)copy.translations[a]));
		if (copy.translations[a]->type == TRANS_COLOUR)
			translations.push_back(new TransRangeColour((TransRangeColour*)copy.translations[a]));
		if (copy.translations[a]->type == TRANS_DESAT)
			translations.push_back(new TransRangeDesat((TransRangeDesat*)copy.translations[a]));
	}
}

TransRange* Translation::getRange(unsigned index) {
	if (index >= translations.size())
		return NULL;
	else
		return translations[index];
}
