
#ifndef __WAD_H__
#define __WAD_H__

class Lump;

#define FORMAT_WAD	0
#define FORMAT_ZIP	1

class Wad
{
private:

protected:
	string				filename;
	vector<Lump*>		lumps;
	bool				iwad;
	//vector<mapdesc_t>	maps;

public:
	struct mapdesc_t
	{
		string	name;
		Lump*	head;
		Lump*	end;
		BYTE	format;	// 0=doom 1=hexen 2=udmf
	};

	Wad();
	~Wad();

	// General info
	string			getFileName(bool fullpath = true);
	bool			isIWAD();
	int				numLumps();
	virtual BYTE	format() { return FORMAT_WAD; }

	// Lump accessors
	Lump*	lumpAt(int index);
	int		lumpIndex(Lump* lump);

	// File operations
	virtual bool	openFile(string filename, string &error);
	virtual bool	loadLump(Lump* lump);

	// Misc
	virtual vector<mapdesc_t>	detectMaps();
};

class ZipWad : public Wad
{
private:

public:
	ZipWad();
	~ZipWad();

	BYTE	format() { return FORMAT_ZIP; }
	bool	openFile(string filename, string &error);
	bool	loadLump(Lump* lump);

	vector<mapdesc_t>	detectMaps();
};

#endif //__WAD_H__
