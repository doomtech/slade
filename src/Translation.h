
#ifndef __TRANSLATION_H__
#define __TRANSLATION_H__

#define	TRANS_PALETTE	1
#define TRANS_COLOUR	2
#define TRANS_DESAT		3

class Translation;

class TransRange {
friend class Translation;
protected:
	uint8_t	type;
	uint8_t	o_start;
	uint8_t	o_end;

public:
	TransRange(uint8_t type) {
		this->type = type;
		this->o_start = 0;
		this->o_end = 0;
	}

	uint8_t getType() { return type; }
	uint8_t	oStart() { return o_start; }
	uint8_t oEnd() { return o_end; }
};

class TransRangePalette : public TransRange {
friend class Translation;
private:
	uint8_t	d_start;
	uint8_t	d_end;

public:
	TransRangePalette() : TransRange(TRANS_PALETTE) {}
	TransRangePalette(TransRangePalette* copy) : TransRange(TRANS_PALETTE) {
		o_start = copy->o_start;
		o_end = copy->o_end;
		d_start = copy->d_start;
		d_end = copy->d_end;
	}

	uint8_t dStart() { return d_start; }
	uint8_t dEnd() { return d_end; }
};

class TransRangeColour : public TransRange {
friend class Translation;
private:
	rgba_t d_start, d_end;

public:
	TransRangeColour() : TransRange(TRANS_COLOUR) {}
	TransRangeColour(TransRangeColour* copy) : TransRange(TRANS_COLOUR) {
		o_start = copy->o_start;
		o_end = copy->o_end;
		d_start.set(copy->d_start);
		d_end.set(copy->d_end);
	}

	rgba_t dStart() { return d_start; }
	rgba_t dEnd() { return d_end; }
};

class TransRangeDesat : public TransRange {
friend class Translation;
private:
	float d_sr, d_sg, d_sb;
	float d_er, d_eg, d_eb;

public:
	TransRangeDesat() : TransRange(TRANS_DESAT) {}
	TransRangeDesat(TransRangeDesat* copy) : TransRange(TRANS_DESAT) {
		o_start = copy->o_start;
		o_end = copy->o_end;
		d_sr = copy->d_sr;
		d_sg = copy->d_sg;
		d_sb = copy->d_sb;
		d_er = copy->d_er;
		d_eg = copy->d_eg;
		d_eb = copy->d_eb;
	}

	float dSr() { return d_sr; }
	float dSg() { return d_sg; }
	float dSb() { return d_sb; }
	float dEr() { return d_er; }
	float dEg() { return d_eg; }
	float dEb() { return d_eb; }
};

class Translation {
private:
	vector<TransRange*>	translations;

public:
	Translation();
	~Translation();

	void	parse(string def);
	string	asText();
	void	clear();
	void	copy(Translation& copy);
	bool	isEmpty() { return translations.size() == 0; }

	unsigned	nRanges() { return translations.size(); }
	TransRange*	getRange(unsigned index);
};

#endif//__TRANSLATION_H__
