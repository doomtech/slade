
#ifndef __WAD_H__
#define __WAD_H__

class Lump;

class Wad
{
private:

protected:
	string			filename;
	vector<Lump*>	lumps;
	bool			iwad;

public:
	Wad();
	~Wad();

	virtual bool	openFile(string filename_);
	virtual bool	loadLump(Lump* lump);
};


class ZipWad : public Wad
{
private:

public:
	ZipWad();
	~ZipWad();

	bool	openFile(string filename_);
};

#endif //__WAD_H__
