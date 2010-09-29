
#ifndef __PALETTE_H__
#define	__PALETTE_H__

class Palette8bit {
private:
	rgba_t	colours[256];
	short	index_trans;

public:
	Palette8bit();
	~Palette8bit();

	rgba_t	colour(uint8_t index) { return colours[index]; }
	short	transIndex() { return index_trans; }

	bool	loadMem(MemChunk& mc);
	bool	loadMem(const uint8_t* data, uint32_t size);
	bool	saveMem(MemChunk& mc);
	bool	saveFile(string filename);

	void	setColour (uint8_t index, rgba_t  col)	{ colours[index]   = col; }
	void	setColourR(uint8_t index, uint8_t val)	{ colours[index].r = val; }
	void	setColourG(uint8_t index, uint8_t val)	{ colours[index].g = val; }
	void	setColourB(uint8_t index, uint8_t val)	{ colours[index].b = val; }
	void	setColourA(uint8_t index, uint8_t val)	{ colours[index].a = val; }
	void	setTransIndex(short index)				{ index_trans = index; }

	void	copyPalette(Palette8bit* copy);
	short	findColour(rgba_t colour);
	size_t	countColours();
};

#endif //__PALETTE_H__
