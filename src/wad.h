
#ifndef __WAD_H__
#define __WAD_H__

class Lump
{
private:
	DWORD	index;
	DWORD	offset;
	DWORD	size;
	string	name;
	BYTE	*data;

public:
	// Constructors
	Lump() { data = (BYTE *)NULL; size = 0; offset = 0; }
	Lump(DWORD offset, DWORD size, string name)
	{
		this->offset = offset;
		this->size = size;
		this->name = name;

		data = (BYTE *)malloc(size);
	}

	// Destructor
	~Lump() { free(data); }

	DWORD	Offset()	{ return offset;	}
	DWORD	Size()		{ return size;		}
	string	Name()		{ return name;		}
	BYTE*	Data()		{ return data;		}
	DWORD	Index()		{ return index;		}

	void	SetOffset(DWORD offset)	{ this->offset = offset;	}
	void	Rename(string name)		{ this->name = name;		}

	void	Resize(DWORD newsize)
	{
		size = newsize;
		data = (BYTE *)realloc(data, size);
	}

	void DumpToFile(string filename)
	{
		FILE *fp = fopen(filename.c_str(), "wb");
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
	Lump*	*directory;
	//vector<Lump*> directory;

	// Stuff
	int		patches[2];
	int		flats[2];
	int		sprites[2];
	int		tx[2];

	vector<string>	available_maps;

	// Flags
	bool	locked;	// True if wad cannot be written to (for IWADs)

	// Constructor/Destructor
	Wad()
	{
		directory = (Lump **)NULL;
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
		if (directory)
		{
			for (DWORD l = 0; l < num_lumps; l++)
				delete directory[l];

			free(directory);
		}
	}

	// Member Functions
	bool	open(string filename);
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
		printf("%s\n", iwad->path.c_str());

		for (DWORD w = 0; w < n_wads; w++)
			printf("%s\n", wads[w]->path.c_str());
	}
};

#define START	0
#define	END		1

#endif
