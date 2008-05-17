
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

	virtual bool	openFile(string filename);
	virtual bool	loadLump(Lump* lump);
};


class ZipWad : public Wad
{
private:

public:
	ZipWad();
	~ZipWad();

	bool	openFile(string filename);
};

#endif //__WAD_H__
