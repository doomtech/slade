
#ifndef __WAD_H__
#define __WAD_H__

class Wad;

/*
class Lump
{
private:
	DWORD	index;
	DWORD	offset;
	DWORD	size;
	string	name;
	BYTE	*data;
	Wad		*parent;

	vector<string>	dir;

public:
	// Constructors
	Lump() { data = (BYTE *)NULL; size = 0; offset = 0; parent = NULL; }
	Lump(DWORD offset, DWORD size, string name, Wad* parent = NULL)
	{
		this->offset = offset;
		this->size = size;
		this->name = name;
		this->parent = parent;

		data = new BYTE[size];
	}

	// Destructor
	~Lump() { if (data) delete[] data; }

	DWORD	Offset()	{ return offset;	}
	DWORD	Size()		{ return size;		}
	string	Name(bool path = true, bool ext = true);
	BYTE*	Data()		{ return data;		}
	DWORD	Index()		{ return index;		}
	Wad*	Parent()	{ return parent;	}

	void	SetOffset(DWORD offset)	{ this->offset = offset;	}
	void	Rename(string name)		{ this->name = name;		}

	void	AddDir(string name)		{ dir.push_back(name); }
	void	ClearDir()				{ dir.clear(); }

	void	Resize(DWORD newsize)
	{
		size = newsize;
		data = (BYTE *)realloc(data, size);
	}

	void DumpToFile(string filename)
	{
		FILE *fp = fopen(chr(filename), "wb");
		fwrite(data, 1, size, fp);
		fclose(fp);
	}

	void LoadData(BYTE *data, DWORD size)
	{
		Resize(size);
		memcpy(this->data, data, size);
	}
};

class Wad
{
public:
	// Wad path
	string		path;

	// Header
	char	type[4];
	DWORD	num_lumps;
	DWORD	dir_offset;

	// Dierctory
	//Lump*	*directory;
	vector<Lump*> directory;

	// Stuff
	int		patches[2];
	int		flats[2];
	int		sprites[2];
	int		tx[2];

	vector<string>	available_maps;

	// Flags
	bool	locked;	// True if wad cannot be written to (for IWADs)
	bool	zip;

	// Constructor/Destructor
	Wad()
	{
		num_lumps = 0;
		dir_offset = 0;
		locked = false;
		type[0] = 'P';
		type[1] = 'W';
		type[2] = 'A';
		type[3] = 'D';

		patches[0] = patches[1] = -1;
		flats[0] = flats[1] = -1;
		sprites[0] = sprites[1] = -1;
		tx[0] = tx[1] = -1;
	}

	~Wad()
	{
		for (DWORD l = 0; l < directory.size(); l++)
			delete directory[l];
	}

	// Member Functions
	bool	open(string filename);
	bool	open_zip(string filename);
	void	save(bool nodes, string mapname = "");
	void	close();

	long	get_lump_index(string name);
	long	get_lump_index(string name, DWORD offset);
	Lump*	get_lump(string name, DWORD offset);

	void	add_lump(string name, long index);
	void	replace_lump(string name, DWORD new_size, BYTE *data, DWORD offset);
	void	delete_lump(string name, DWORD offset);

	//void*	lump_mem(DWORD index);
	void	dump_directory();

	void	calculate_offsets();
};
*/

class Lump
{
private:
	long		offset;
	long		size;
	string		name;
	BYTE		*data;
	int			type;
	bool		loaded;
	Wad*		parent;

	vector<string>	directory;

public:
	Lump(long offset = 0, long size = 0, string name = _T(""), Wad* parent = NULL);
	~Lump();

	long	getOffset() { return offset;	}
	long	getSize()	{ return size;		}
	string	getName(bool full = true, bool ext = true);
	int		getType()	{ return type;		}
	bool	isLoaded()	{ return loaded;	}
	Wad*	getParent() { return parent;	}
	BYTE*	getData(bool load = true);

	string	getDir(int index);
	string	getFullDir();
	int		dirLevel()	{ return (int)directory.size(); }
	void	addDir(string name, int index = -1);
	void	renameDir(string nname, int index);

	void	setOffset(DWORD o);
	void	reSize(DWORD s);
	void	setName(string n);
	void	setParent(Wad* p)	{ parent = p;	}

	void	dumpToFile(string filename);
	bool	loadFile(string filename);
	void	loadStream(FILE *fp);
	void	loadData(BYTE* buf, DWORD size);

	bool	isFolder();
};

class Wad
{
private:
	vector<Lump*>	directory;

public:
	Lump	*parent;

	// Wad path
	string		path;

	// Stuff
	int		patches[2];
	int		flats[2];
	int		sprites[2];
	int		tx[2];

	vector<string>	available_maps;

	// Flags
	bool	locked;	// True if wad cannot be written to (for IWADs)
	bool	zip;

	// Constructor/Destructor
	Wad();
	~Wad()
	{
		for (DWORD l = 0; l < directory.size(); l++)
			delete directory[l];
	}

	DWORD	numLumps() { return (DWORD)directory.size(); }

	// Member Functions
	bool	open(string filename, bool load_data = false);
	bool	openZip(string filename, bool load_data = false);
	void	save(bool nodes, string mapname = _T(""));
	void	saveZip();
	void	close();

	long	getLumpIndex(string name, DWORD offset = 0, bool dir = false, bool ext = false);
	long	lumpIndex(Lump* lump);
	Lump*	getLump(string name, DWORD offset = 0, bool dir = false, bool ext = false);
	Lump*	lumpAt(int index);
	Lump*	lastLump();

	void	addDir(string path);
	void	renameDir(Lump* dirlump, string newname);
	Lump*	addLump(string name, long index);
	void	replaceLump(long index, DWORD new_size, BYTE *data);
	void	deleteLump(string name, DWORD offset = 0);
	void	deleteLump(long index, bool delfolder = true);
	void	swapLumps(long index1, long index2);
	void	swapLumps(Lump* lump1, Lump* lump2);

	void	deleteAllLumps();

	void	dumpDirectory();
	bool	needsSave();
	void	findMaps();
};

class WadList
{
private:
	vector<Wad*>	wads;
	Wad*			iwad;

public:
	WadList() { iwad = new Wad(); }
	~WadList();

	int		nWads() { return (int)wads.size(); }

	bool	open(string filename, BYTE flags = 0);
	bool	close(string filename);
	void	closeAll();
	void	reloadAll();
	Wad*	getWad(int index = -1);
	Wad*	getLast();
	Wad*	getLastWithMaps();
	Wad*	getWadWithLump(string name, bool full = false, bool ext = false);
	Lump*	getLump(string name, bool full = false, bool ext = false);
};

#define WL_IWAD	0x01
#define WL_ZIP	0x02

/*
class WadList
{
private:
	Wad*	*wads;	// Wads list
	Wad*	iwad;	// Game IWAD

public:
	BYTE	n_wads;	// Number of opened wads

	WadList() { wads = (Wad **)NULL; n_wads = 0; iwad = new Wad(); }
	~WadList() { free(wads); }

	bool open_wad(string filename)
	{
		Wad *new_wad = new Wad();

		if (!new_wad->open(filename))
			return false;

		if (new_wad->type[0] == 'I')
			return false;

		n_wads++;
		wads = (Wad **)realloc(wads, n_wads * sizeof(Wad *));
		wads[n_wads - 1] = new_wad;

		return true;
	}

	bool open_iwad(string filename)
	{
		return iwad->open(filename);
	}

	bool close_wad(string filename)
	{
		int index = -1;

		BYTE w = 0;
		for (w = 0; w < n_wads; w++)
		{
			if (wads[w]->path == filename)
				index = w;
		}

		if (index == -1)
			return false;

		delete wads[index];

		for (w = index; w < n_wads - 1; w++)
			wads[w] = wads[w + 1];

		n_wads--;
		wads = (Wad **)realloc(wads, sizeof(Wad*));

		return true;
	}

	void close_all()
	{
		if (n_wads = 0)
			return;

		for (BYTE w = 0; w < n_wads; w++)
			delete wads[w];
		
		n_wads = 0;
		wads = (Wad **)NULL;
	}

	void reload_wads()
	{
		for (int a = 0; a < n_wads; a++)
		{
			string path = wads[a]->path;
			wads[a]->close();
			wads[a]->open(path);
		}
	}


	Wad* get_iwad()
	{
		return iwad;
	}

	Wad* get_wad(DWORD index)
	{
		return wads[index];
	}

	Wad* get_last()
	{
		if (n_wads > 0)
			return wads[n_wads - 1];
		else
			return NULL;
	}

	Wad* get_last_withmaps()
	{
		if (n_wads > 0)
		{
			for (int w = n_wads - 1; w >= 0; w--)
			{
				if (wads[w]->available_maps.size() != 0)
					return wads[w];
			}

			return NULL;
		}
		else
			return NULL;
	}

	Wad* get_wad_with_lump(string name)
	{
		if (n_wads > 0)
		{
			for (int w = n_wads - 1; w >= 0; w--)
			{
				if (wads[w]->get_lump(name, 0))
					return wads[w];
			}
		}

		if (iwad->get_lump(name, 0))
			return iwad;
		else
			return NULL;
	}

	Lump* get_lump(string name)
	{
		for (DWORD w = n_wads - 1; w == 0; w--)
		{
			if (wads[w]->get_lump(name, 0))
				return wads[w]->get_lump(name, 0);
		}

		if (iwad->get_lump(name, 0))
			return iwad->get_lump(name, 0);
		else
			return NULL;
	}

	void list_wads()
	{
		printf("%s\n", chr(iwad->path));

		for (DWORD w = 0; w < n_wads; w++)
			printf("%s\n", chr(wads[w]->path));
	}
};
*/

#define START	0
#define	END		1

#endif
