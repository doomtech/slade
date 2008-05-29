
#ifndef __WAD_H__
#define __WAD_H__

class Lump;

#define FORMAT_WAD	0
#define FORMAT_ZIP	1

class Wad
{
private:

protected:
	struct mapdesc_t
	{
		string	name;
		Lump*	head;
		BYTE	format;	// 0=doom 1=hexen 2=udmf
	};

	BYTE				format;
	string				filename;
	vector<Lump*>		lumps;
	bool				iwad;
	vector<mapdesc_t>	maps;

public:
	Wad(BYTE format = FORMAT_WAD);
	~Wad();

	string		getFileName();
	bool		isIWAD();
	bool		isFormat(BYTE format) { return this->format == format; }
	int			numLumps();
	Lump*		lumpAt(int index);

	bool	openWadFile(string filename, string &error);
	bool	openZipFile(string filename, string &error);

	bool	loadLump(Lump* lump);
	void	detectMaps();
};

#endif //__WAD_H__
