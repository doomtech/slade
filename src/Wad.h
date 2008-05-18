
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

	string		getFileName();
	bool		isIWAD();
	int			numLumps();
	Lump*		lumpAt(int index);

	virtual bool	openFile(string filename, string &error);
	virtual bool	loadLump(Lump* lump);
};


class ZipWad : public Wad
{
private:

public:
	ZipWad();
	~ZipWad();

	bool	openFile(string filename, string &error);
};

#endif //__WAD_H__
