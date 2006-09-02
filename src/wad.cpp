// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << wad.cpp - Wad class functions        >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "byteswap.h"
#include "main.h"
#include "wad.h"
#include "splash.h"

#include "zdbsp/wad.h"
#include "zdbsp/processor.h"

#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#ifndef WIN32
#define stricmp strcasecmp
#endif

// Variables ----------------------------- >>
WadList	wads;

CVAR(String, nodebuilder_path, "", CVAR_SAVE)
CVAR(String, nodebuilder_params, "", CVAR_SAVE)

// External Variables -------------------- >>
extern string map_lumps[12];


Lump::Lump(long offset, long size, string name, Wad* parent)
{
	this->offset = offset;
	this->size = size;
	this->name = name;
	this->parent = parent;

	data = NULL;
	loaded = false;

	if (size > 0)
	{
		data = new BYTE[size];
		memset(data, 0, size);
	}
}

Lump::~Lump()
{
	if (data)
		delete[] data;
}

BYTE* Lump::getData(bool load)
{
	if (loaded || !load)
		return data;

	if (size > 0 && parent)
	{
		FILE* fp = fopen(parent->path.c_str(), "rb");
		fseek(fp, offset, SEEK_SET);
		fread(data, size, 1, fp);
		fclose(fp);

		loaded = true;
	}

	return data;
}

string Lump::getName(bool full, bool ext)
{
	if (parent->zip)
	{
		string ret;

		if (full)
			ret += getFullDir();

		if (ext)
			ret += name;
		else
		{
			wxFileName fn(name);
			ret += fn.GetName();
		}

		return ret;
	}
	else
		return name;
}

void Lump::setOffset(DWORD o)
{
	offset = o;
}

void Lump::reSize(DWORD s)
{
	if (data)
	{
		delete[] data;
		data = NULL;
		loaded = false;
	}

	size = s;

	if (size > 0)
	{
		data = new BYTE[size];
		memset(data, 0, size);
	}
}

void Lump::setName(string n)
{
	name = n;
}

void Lump::dumpToFile(string filename)
{
	if (!data)
		return;

	FILE *fp = fopen((char*)filename.c_str(), "wb");
	fwrite(data, size, 1, fp);
	fclose(fp);
}

bool Lump::loadFile(string filename)
{
	FILE *fp = fopen((char*)filename.c_str(), "rb");

	if (!fp)
		return false;

	// Get size
	fseek(fp, 0, SEEK_END);
	long filesize = ftell(fp);
	size = filesize;
	fseek(fp, 0, SEEK_SET);

	reSize(size);
	fread(data, size, 1, fp);
	fclose(fp);

	loaded = true;

	return true;
}

void Lump::loadStream(FILE *fp)
{
	loaded = true;

	if (size == 0)
		return;

	fseek(fp, offset, SEEK_SET);
	fread(data, size, 1, fp);
}

void Lump::loadData(BYTE* buf, DWORD size)
{
	reSize(size);
	memcpy(data, buf, size);
	loaded = true;
}

void Lump::addDir(string name, int index)
{
	if (index == -1)
		directory.push_back(name);
	else
		directory.insert(directory.begin() + index, name);
}

string Lump::getDir(int index)
{
	if (index < 0 || index >= directory.size())
		return _T("");
	else
		return directory[index];
}

void Lump::renameDir(string nname, int index)
{
	if (index >= 0 && index < directory.size())
		directory[index] = nname;
}

string Lump::getFullDir()
{
	if (directory.size() == 0)
		return _T("");

	string ret;
	for (int a = 0; a < directory.size(); a++)
	{
		ret += directory[a];
		ret += _T("/");
	}

	return ret;
}

bool Lump::isFolder()
{
	if (size == 0 && getName(false, false).Right(1) == _T("/"))
		return true;
	else
		return false;
}

Wad::Wad()
{
	locked = false;
	zip = false;
	path = _T("");
	parent = NULL;

	patches[0] = patches[1] = -1;
	flats[0] = flats[1] = -1;
	sprites[0] = sprites[1] = -1;
	tx[0] = tx[1] = -1;
}

void Wad::findMaps()
{
	// Find all maps
	if (zip)
	{
		for (int a = 0; a < numLumps(); a++)
		{
			if (lumpAt(a)->dirLevel() == 1 && lumpAt(a)->getDir(0).CmpNoCase(_T("maps")) == 0)
			{
				BYTE* data = lumpAt(a)->getData();

				if (data[0] == 'P' && data[1] == 'W' && data[2] == 'A' && data[3] == 'D')
					available_maps.push_back(lumpAt(a)->getName(false, false));
			}
		}
	}
	else
	{
		//log_message(s_fmt(_T("Finding maps in %s..."), chr(path)));
		for (DWORD l = 0; l < numLumps(); l++)
		{
			bool done = false;
			bool existing_map_lumps[12] = { false, false, false, false,
				false, false, false, false,
				false, false, false, false };

			int index = l;
			while (!done)
			{
				l++;

				if (l == numLumps())
					break;

				string name = directory[l]->getName(false, false);
				done = true;

				for (int s = 0; s < 12; s++)
				{
					if (name == map_lumps[s])
					{
						existing_map_lumps[s] = true;
						done = false;
					}
				}
			}

			l--;

			if (!memchr(existing_map_lumps, 0, 5))
			{
				available_maps.push_back(directory[index]->getName(false, false));
				//log_message(available_maps.back());
			}
		}
	}
}

// Wad::open: Opens a wad file
// ------------------------ >>
bool Wad::open(string filename, bool load_data)
{
	if (filename == _T(""))
		return false;

	close();
	path = filename;
	zip = false;

	// Open the file
	FILE *fp = fopen(chr(filename), "rb");

	// Check if it opened correctly
	if (!fp)
	{
		log_message(s_fmt(_T("Wad file \"%s\" cannot be found.\n"), chr(filename)));
		close();
		return false;
	}

	long num_lumps = 0;
	long filesize = 0;
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Read the header
	char type[4] = "";
	DWORD dir_offset = 0;
	fread(&type, 1, 4, fp);			// Wad type
	fread(&num_lumps, 4, 1, fp);	// No. of lumps in wad
	fread(&dir_offset, 4, 1, fp);	// Offset to directory

	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Lock the wad if IWAD
	if (type[0] == 'I')
		locked = true;
	else
		locked = false;

	if (type[1] != 'W' || type[2] != 'A' || type[3] != 'D')
	{
		message_box(_T("Not a valid Doom wad file!"), _T("Error"));
		return false;
	}

	// Read the directory
	fseek(fp, dir_offset, SEEK_SET);
	for (DWORD d = 0; d < num_lumps; d++)
	{
		char name[9] = "";
		DWORD offset = 0;
		DWORD size = 0;

		fread(&offset, 4, 1, fp);	// Offset
		fread(&size, 4, 1, fp);		// Size
		fread(name, 1, 8, fp);		// Name

		offset = wxINT32_SWAP_ON_BE(offset);
		size = wxINT32_SWAP_ON_BE(size);

		if (offset + size > filesize)
		{
			message_box(_T("Not a valid Doom wad file!"), _T("Error"));
			return false;
		}

		name[8] = '\0';
		Lump* nlump = new Lump(offset, size, wxString::FromAscii(name).Upper(), this);

		if (nlump->getName() == _T("P_START") || nlump->getName() == _T("PP_START"))
			patches[START] = d;

		if (nlump->getName() == _T("P_END") || nlump->getName() == _T("PP_END"))
			patches[END] = d;

		if (nlump->getName() == _T("F_START") || nlump->getName() == _T("FF_START"))
			flats[START] = d;

		if (nlump->getName() == _T("F_END") || nlump->getName() == _T("FF_END"))
			flats[END] = d;

		if (nlump->getName() == _T("S_START") || nlump->getName() == _T("SS_START"))
			sprites[START] = d;

		if (nlump->getName() == _T("S_END") || nlump->getName() == _T("SS_END"))
			sprites[END] = d;

		if (nlump->getName() == _T("TX_START"))
			tx[START] = d;

		if (nlump->getName() == _T("TX_END"))
			tx[END] = d;

		directory.push_back(nlump);
	}

	// Read wad data into memory
	if (load_data)
	{
		for (DWORD d = 0; d < numLumps(); d++)
			directory[d]->loadStream(fp);
	}

	fclose(fp);

	findMaps();

	return true;
}

bool Wad::openZip(string filename, bool load_data)
{
	path = filename;
	this->zip = true;

	wxZipEntry* entry;

	wxFFileInputStream in(filename);
	if (!in.IsOk())
		return false;

	wxZipInputStream zip(in);
	if (!zip.IsOk())
	{
		message_box(_T("Invalid zip file!"), _T("Error"));
		return false;
	}

	entry = zip.GetNextEntry();
	vector<string> alldirs;
	while (entry)
	{
		if (!entry->IsDir())
		{
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);

			// Create lump
			Lump *nlump = new Lump(0, entry->GetSize(), fn.GetFullName(), this);

			// Load data
			BYTE* buffer = new BYTE[entry->GetSize()];
			zip.Read(buffer, entry->GetSize());
			nlump->loadData(buffer, entry->GetSize());
			delete[] buffer;

			// Get directory info
			wxArrayString dirs = fn.GetDirs();
			for (int a = 0; a < dirs.size(); a++)
				nlump->addDir(dirs[a]);

			if (dirs.size() > 0)
			{
				string dir = dirs[0] + _T("/");
				vector_add_nodup(alldirs, dir);
				for (int a = 1; a < dirs.size(); a++)
				{
					dir += dirs[a] + _T("/");
					vector_add_nodup(alldirs, dir);
				}
			}

			// Add to directory
			directory.push_back(nlump);
		}
		else
			addDir(entry->GetName(wxPATH_UNIX));

		entry = zip.GetNextEntry();
	}

	for (int a = 0; a < alldirs.size(); a++)
	{
		wxFileName fn(alldirs[a], wxPATH_UNIX);
		wxArrayString dirs2 = fn.GetDirs();

		Lump* lump = addLump(dirs2[fn.GetDirCount()-1] + _T("/"), 0);

		for (int a = 0; a < dirs2.size()-1; a++)
			lump->addDir(dirs2[a]);
	}

	findMaps();

	return true;
}

// Wad::dump_directory: Writes all directory entries to the logfile
// ------------------------------------------------------------- >>
void Wad::dumpDirectory()
{
	for (DWORD l = 0; l < numLumps(); l++)
		log_message(s_fmt(_T("%d: %s (%db at %d)\n"), l, chr(directory[l]->getName()), directory[l]->getSize(), directory[l]->getOffset()));
}

// Wad::get_lump_index: Returns the index of the first lump with the specified name
//                      Returns -1 if no matching lump is found
// ----------------------------------------------------------------------------- >>
long Wad::getLumpIndex(string name, DWORD offset, bool dir, bool ext)
{
	if (numLumps() == 0 || offset > numLumps())
		return -1;

	for (DWORD l = offset; l < numLumps(); l++)
	{
		if (directory[l]->getName(dir, ext).CmpNoCase(name) == 0)
			return l;
	}

	return -1;
}

// Wad::get_lump: Returns a pointer to the first matching lump from offset
// -------------------------------------------------------------------- >>
Lump* Wad::getLump(string name, DWORD offset, bool dir, bool ext)
{
	for (DWORD l = offset; l < numLumps(); l++)
	{
		if (directory[l]->getName(dir, ext).CmpNoCase(name) == 0)
			return directory[l];
	}

	return NULL;
}

// Wad::add_lump: Adds a new 0-sized lump before <index>
// -------------------------------------------------- >>
Lump* Wad::addLump(string name, long index)
{
	Lump* nlump = new Lump(0, 0, name, this);

	if (index < 0 || index >= numLumps())
		directory.push_back(nlump);
	else
		directory.insert(directory.begin() + index, nlump);

	return nlump;
}

// Wad::replace_lump: Replaces lump data with new data
// ------------------------------------------------ >>
void Wad::replaceLump(long index, DWORD new_size, BYTE *data)
{
	if (index > -1 && index < numLumps())
	{
		directory[index]->reSize(new_size);
		memcpy(directory[index]->getData(false), data, new_size);
	}
}

// Wad::delete_lump: Deletes a lump
// ----------------------------- >>
void Wad::deleteLump(string name, DWORD offset)
{
	long index = getLumpIndex(name, offset);
	deleteLump(index);
}

// Wad::delete_lump: Deletes a lump
// ----------------------------- >>
void Wad::deleteLump(long index, bool delfolder)
{
	if (index < 0 || index > numLumps())
		return;

	Lump *lump = lumpAt(index);
	if (lump->isFolder() && delfolder)
	{
		vector<string> dirs;
		for (int a = 0; a < lump->dirLevel(); a++)
			dirs.push_back(lump->getDir(a));

		string ldir = lump->getName(false);
		ldir.RemoveLast();
		dirs.push_back(ldir);

		for (int a = 0; a < numLumps(); a++)
		{
			if (lumpAt(a)->dirLevel() < dirs.size())
				continue;

			bool in = true;
			for (int d = 0; d < dirs.size(); d++)
			{
				if (dirs[d] != lumpAt(a)->getDir(d))
				{
					in = false;
					break;
				}
			}

			if (in)
			{
				deleteLump(a, false);
				a--;
			}
		}

		index = lumpIndex(lump);
	}

	directory.erase(directory.begin() + index);
}

// copy_file: Creates a copy of a file
// -------------------------------- >>
void copy_file(string input, string output)
{
	FILE *in = fopen(chr(input), "rb");
	if (in)
	{
		FILE *out = fopen(chr(output), "wb");
		while (!feof(in))
		{
			BYTE b = 0;
			fread(&b, 1, 1, in);
			fwrite(&b, 1, 1, out);
		}
		fclose(out);
		fclose(in);
	}
}

// Wad::save: Saves a wad file to disk (backs up the file, then overwrites it)
// ------------------------------------------------------------------------ >>
void Wad::save(bool nodes, string mapname)
{
	if (locked)
	{
		message_box(_T("Saving to an IWAD is disallowed. Use 'Save As' instead."), _T("Not Allowed"));
		return;
	}

	if (zip)
	{
		saveZip();
		return;
	}

	if (!parent)
	{
		// Remove old backup file
		string bakfile = path + _T(".bak");
		remove(chr(bakfile));

		// Copy current file contents to new backup file
		copy_file(path, bakfile);
	}

	// Determine directory offset & individual lump offsets
	DWORD dir_offset = 12;

	for (DWORD l = 0; l < numLumps(); l++)
	{
		directory[l]->setOffset(dir_offset);
		dir_offset += directory[l]->getSize();
	}

	// Open wadfile for writing
	FILE *fp = NULL;

	if (!parent)
		fp = fopen(chr(path), "wb");
	else
		fp = fopen(chr(c_path(_T("sladetemp.wad"), DIR_TMP)), "wb");

	// Write the header
	long num_lumps = numLumps();
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);
	fwrite("PWAD", 1, 4, fp);
	fwrite(&num_lumps, 4, 1, fp);
	fwrite(&dir_offset, 4, 1, fp);

	// Write the lumps
	for (DWORD l = 0; l < num_lumps; l++)
		lefwrite(directory[l]->getData(false), directory[l]->getSize(), 1, fp);

	// Write the directory
	for (DWORD l = 0; l < numLumps(); l++)
	{
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		long offset = directory[l]->getOffset();
		long size = directory[l]->getSize();

		for (int c = 0; c < directory[l]->getName().length(); c++)
			name[c] = directory[l]->getName()[c];

		offset = wxINT32_SWAP_ON_BE(offset);
		size = wxINT32_SWAP_ON_BE(size);
		fwrite(&offset, 4, 1, fp);
		fwrite(&size, 4, 1, fp);
		fwrite(name, 1, 8, fp);
	}

	fclose(fp);

	// Build nodes
	if (nodes)
	{
		string wadpath = path;

		if (parent)
			wadpath = _T("sladetemp.wad");

		splash(_T("Building Nodes"), true);

		GLOnly = false;
		BuildGLNodes = false;

		FWadReader inwad(chr(wadpath));
		FWadWriter outwad(chr(s_fmt(_T("%s.temp"), chr(wadpath))), inwad.IsIWAD());

		int lump = 0;
		int max = inwad.NumLumps();

		while (lump < max)
		{
			if (inwad.IsMap (lump) &&
				(mapname == _T("") || mapname.CmpNoCase(inwad.LumpName(lump)) == 0)) //stricmp (inwad.LumpName (lump), str(mapname).c_str()) == 0))
			{
				splash(s_fmt(_T("Building nodes on %s"), inwad.LumpName(lump)), true);
				FProcessor builder(inwad, lump);
				builder.Write (outwad);

				lump = inwad.LumpAfterMap (lump);
			}
			else if (inwad.IsGLNodes (lump))
			{
				// Ignore GL nodes from the input for any maps we process.
				if (BuildNodes && (Map == NULL || stricmp(inwad.LumpName (lump)+3, Map) == 0))
					lump = inwad.SkipGLNodes (lump);
				else
				{
					outwad.CopyLump (inwad, lump);
					++lump;
				}
			}
			else
			{
				outwad.CopyLump (inwad, lump);
				++lump;
			}
		}

		outwad.Close();

		remove(path.c_str());
		copy_file(s_fmt(_T("%s.temp"), chr(wadpath)), wadpath);
		remove(chr(s_fmt("%s.temp", chr(wadpath))));

		splash_hide();
	}

	if (parent)
	{
		parent->loadFile(c_path(_T("sladetemp.wad"), DIR_TMP));
		remove(chr(c_path(_T("sladetemp.wad"), DIR_TMP)));
		parent->getParent()->save(false);
	}
}

void Wad::saveZip()
{
	string bakfile = path + _T(".bak");

	// Remove old backup file
	remove(chr(bakfile));

	// Copy current file contents to new backup file
	copy_file(path, bakfile);

	wxFFileOutputStream out(path);
	if (!out.IsOk())
		return;

	wxZipOutputStream zip(out, 9);

	for (int a = 0; a < directory.size(); a++)
	{
		if (directory[a]->isFolder())
			zip.PutNextDirEntry(directory[a]->getName());
		else
		{
			wxZipEntry* entry = new wxZipEntry(directory[a]->getName());

			zip.PutNextEntry(entry);
			zip.Write(directory[a]->getData(), directory[a]->getSize());
		}
	}

	zip.Close();
}

// Wad::close: Closes the wad file
// ---------------------------- >>
void Wad::close()
{
	if (path == _T(""))
		return;

	available_maps.clear();

	for (int a = 0; a < directory.size(); a++)
		delete directory[a];

	directory.clear();

	path = _T("");
	locked = true;
}

Lump* Wad::lumpAt(int index)
{
	if (index < 0 || index >= numLumps())
		 return NULL;

	return directory[index];
}

long Wad::lumpIndex(Lump* lump)
{
	for (int a = 0; a < directory.size(); a++)
	{
		if (directory[a] == lump)
			return a;
	}

	return -1;
}

Lump* Wad::lastLump()
{
	if (directory.size() == 0)
		return NULL;
	else
		return directory.back();
}

void Wad::swapLumps(long index1, long index2)
{
	if (!lumpAt(index1) || !lumpAt(index2))
		return;

	Lump* temp = lumpAt(index2);
	directory[index2] = lumpAt(index1);
	directory[index1] = temp;
}

void Wad::swapLumps(Lump* lump1, Lump* lump2)
{
	swapLumps(lumpIndex(lump1), lumpIndex(lump2));
}

void Wad::renameDir(Lump* dirlump, string newname)
{
	for (int a = 0; a < numLumps(); a++)
	{
		Lump* lump = directory[a];

		if (lump == dirlump || lump->dirLevel() <= dirlump->dirLevel())
			continue;

		bool in = true;
		for (int d = 0; d < dirlump->dirLevel(); d++)
		{
			if (lump->getDir(d) != dirlump->getDir(d))
			{
				in = false;
				break;
			}
		}

		string dname = dirlump->getName(false);
		dname.RemoveLast();
		if (in && lump->getDir(dirlump->dirLevel()) == dname)
			lump->renameDir(newname, dirlump->dirLevel());
	}

	dirlump->setName(newname + _T("/"));
}

void Wad::deleteAllLumps()
{
	while (numLumps() > 0)
		deleteLump(0);
}

void Wad::addDir(string path)
{
}


WadList::~WadList()
{
	delete iwad;
	for (int a = 0; a < wads.size(); a++)
		delete wads[a];
}

bool WadList::open(string filename, BYTE flags)
{
	Wad *nwad = new Wad();
	bool retval = false;

	if (flags & WL_ZIP)
		retval = nwad->openZip(filename, true);
	else
		retval = nwad->open(filename, true);

	if (retval)
	{
		if (flags & WL_IWAD)
			iwad = nwad;
		else
			wads.push_back(nwad);
	}
	else
		delete nwad;

	return retval;
}

bool WadList::close(string filename)
{
	for (int a = 0; a < wads.size(); a++)
	{
		if (wads[a]->path == filename)
		{
			wads[a]->close();
			delete wads[a];
			wads.erase(wads.begin() + a);
			return true;
		}
	}

	return false;
}

void WadList::closeAll()
{
	for (int a = 0; a < wads.size(); a++)
		delete wads[a];

	wads.clear();
}

void WadList::reloadAll()
{
	for (int a = 0; a < wads.size(); a++)
	{
		string path = wads[a]->path;
		wads[a]->close();
		wads[a]->open(path);
	}
}

Wad* WadList::getWad(int index)
{
	if (index == -1)
		return iwad;
	else if (index > -1 && index < wads.size())
		return wads[index];
	else
		return NULL;
}

Wad* WadList::getLast()
{
	if (wads.size() > 0)
		return wads.back();
	else
		return NULL;
}

Wad* WadList::getLastWithMaps()
{
	for (int a = wads.size() - 1; a >= 0; a--)
	{
		if (wads[a]->available_maps.size() > 0)
			return wads[a];
	}

	return NULL;
}

Wad* WadList::getWadWithLump(string name)
{
	for (int a = wads.size() - 1; a >= 0; a--)
	{
		if (wads[a]->getLump(name))
			return wads[a];
	}

	if (iwad->getLump(name))
		return iwad;

	return NULL;
}

Lump* WadList::getLump(string name)
{
	for (int a = wads.size() - 1; a >= 0; a--)
	{
		int index = wads[a]->getLumpIndex(name);
		if (index != -1)
			return wads[a]->lumpAt(index);
	}

	int index = iwad->getLumpIndex(name);
	if (index != -1)
		return iwad->lumpAt(index);

	return NULL;
}














/*
string Lump::Name(bool path, bool ext)
{
	if (parent->zip)
	{
		string ret;

		if (path && dir.size() > 0)
		{
			for (int a = 0; a < dir.size(); a++)
			{
				ret += dir[a];
				ret += _T("/");
			}
		}

		if (ext)
			ret += name;
		else
		{
			wxFileName fn(name);
			ret += fn.GetName();
		}

		return ret;
	}
	else
		return name;
}

// Wad::open: Opens a wad file
// ------------------------ >>
bool Wad::open(string filename)
{
	close();
	path = filename;

	// Open the file
	FILE *fp = fopen(str(filename).c_str(), "rb");

	// Check if it opened correctly
	if (!fp)
	{
		//printf("Wad file \"%s\" cannot be found.\n", filename.c_str());
		return false;
	}

	// Read the header
	lefread(&type, 1, 4, fp);			// Wad type
	lefread(&num_lumps, 4, 1, fp);	// No. of lumps in wad
	lefread(&dir_offset, 4, 1, fp);	// Offset to directory

	// Lock the wad if IWAD
	if (type[0] == 'I')
		locked = true;
	else
		locked = false;

	// Setup the directory
	//directory = (dir_entry_t *)calloc(num_lumps, sizeof(dir_entry_t));
	directory = (Lump **)calloc(num_lumps, sizeof(Lump*));

	// Read the directory
	fseek(fp, dir_offset, SEEK_SET);
	for (DWORD d = 0; d < num_lumps; d++)
	{
		char name[9] = "";
		DWORD offset = 0;
		DWORD size = 0;

		lefread(&offset, 4, 1, fp);	// Offset
		lefread(&size, 4, 1, fp);		// Size
		lefread(name, 1, 8, fp);		// Name

		name[8] = '\0';
		directory[d] = new Lump(offset, size, wxString::FromAscii(name).Upper(), this);

		if (directory[d]->Name() == _T("P_START") || directory[d]->Name() == _T("PP_START"))
			patches[START] = d;

		if (directory[d]->Name() == _T("P_END") || directory[d]->Name() == _T("PP_END"))
			patches[END] = d;

		if (directory[d]->Name() == _T("F_START") || directory[d]->Name() == _T("FF_START"))
			flats[START] = d;

		if (directory[d]->Name() == _T("F_END") || directory[d]->Name() == _T("FF_END"))
			flats[END] = d;

		if (directory[d]->Name() == _T("S_START") || directory[d]->Name() == _T("SS_START"))
			sprites[START] = d;

		if (directory[d]->Name() == _T("S_END") || directory[d]->Name() == _T("SS_END"))
			sprites[END] = d;

		if (directory[d]->Name() == _T("TX_START"))
			tx[START] = d;

		if (directory[d]->Name() == _T("TX_END"))
			tx[END] = d;
	}

	// Read wad data into memory (only if wad isn't locked)
	if (!locked)
	{
		for (DWORD d = 0; d < num_lumps; d++)
		{
			//directory[d].data = (BYTE *)malloc(directory[d].size);

			fseek(fp, directory[d]->Offset(), SEEK_SET);
			lefread(directory[d]->Data(), 1, directory[d]->Size(), fp);
		}
	}

	// Find all available maps
	for (DWORD l = 0; l < num_lumps; l++)
	{
		//if (directory[l]->Size() == 0)
		//{
			string mapname = directory[l]->Name();
			bool done = false;
			bool existing_map_lumps[12] = { false, false, false, false,
											false, false, false, false,
											false, false, false, false };

			while (!done)
			{
				l++;

				if (l == num_lumps)
					break;

				string name = directory[l]->Name();
				done = true;

				for (int s = 0; s < 12; s++)
				{
					if (name.substr(0, map_lumps[s].length()) == map_lumps[s])
					{
						existing_map_lumps[s] = true;
						done = false;
					}
				}
			}
			
			l--;

			if (!memchr(existing_map_lumps, 0, 5))
				available_maps.push_back(mapname);
		//}
	}

	sort(available_maps.begin(), available_maps.end());
	//available_maps.sort();

	zip = false;
	fclose(fp);
	return true;
}

bool Wad::open_zip(string filename)
{
	path = filename;

	wxZipEntry* entry;

	wxFFileInputStream in(filename);
	if (!in.IsOk())
		return false;

	wxZipInputStream zip(in);
	if (!zip.IsOk())
	{
		message_box(_T("Invalid zip file!"));
		return false;
	}

	vector<string> alldirs;
	entry = zip.GetNextEntry();
	while (entry)
	{
		if (!entry->IsDir())
		{
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);

			// Create lump
			Lump *nlump = new Lump(0, entry->GetSize(), fn.GetFullName(), this);

			// Load data
			BYTE* buffer = new BYTE[entry->GetSize()];
			zip.Read(buffer, entry->GetSize());
			//nlump->loadData(buffer, entry->GetSize());
			nlump->LoadData(buffer, entry->GetSize());
			delete[] buffer;

			// Get directory info
			wxArrayString dirs = fn.GetDirs();
			for (int a = 0; a < dirs.size(); a++)
				nlump->AddDir(dirs[a]);

			if (dirs.size() > 0)
			{
				string dir = dirs[0] + _T("/");
				vector_add_nodup(alldirs, dir)
				for (int a = 1; a < dirs.size(); a++)
				{
					dir += dirs[a] + _T("/");
					vector_add_nodup(alldirs, dir)
				}
			}

			// Add to directory
			directory.push_back(nlump);
		}
		else
			vector_add_nodup(alldirs, entry->GetName(wxPATH_UNIX));

		entry = zip.GetNextEntry();
	}

	for (int a = 0; a < alldirs.size(); a++)
	{
		wxFileName fn(alldirs[a], wxPATH_UNIX);
		wxArrayString dirs2 = fn.GetDirs();

		Lump* lump = addLump(dirs2[fn.GetDirCount()-1] + _T("/"), 0);

		for (int a = 0; a < dirs2.size()-1; a++)
			lump->addDir(dirs2[a]);
	}

	zip = true;
	return true;
}

// Wad::dump_directory: Writes all directory entries to the logfile
// ------------------------------------------------------------- >>
void Wad::dump_directory()
{
	for (DWORD l = 0; l < num_lumps; l++)
		log_message(s_fmt(_T("%d: %s (%db at %d)\n"), l, directory[l]->Name(), directory[l]->Size(), directory[l]->Offset()));
}

// Wad::get_lump_index: Returns the index of the first lump with the specified name
//                      Returns -1 if no matching lump is found
// ----------------------------------------------------------------------------- >>
long Wad::get_lump_index(string name)
{
	for (DWORD l = 0; l < num_lumps; l++)
	{
		if (directory[l]->Name().CmpNoCase(name) == 0)
			return l;
	}

	return -1;
}

// Same as above but searches from an offset
// -------------------------------------- >>
long Wad::get_lump_index(string name, DWORD offset)
{
	if (num_lumps == 0)
		return -1;

	for (DWORD l = offset; l < num_lumps; l++)
	{
		if (directory[l]->Name().CmpNoCase(name) == 0)
			return l;
	}

	return -1;
}

// Wad::get_lump: Returns a pointer to the first matching lump from offset
// -------------------------------------------------------------------- >>
Lump* Wad::get_lump(string name, DWORD offset)
{
	for (DWORD l = offset; l < num_lumps; l++)
	{
		if (directory[l]->Name().CmpNoCase(name) == 0)
			return directory[l];
	}

	return NULL;
}

// Wad::add_lump: Adds a new 0-sized lump before <index>
// -------------------------------------------------- >>
Lump* Wad::add_lump(string name, long index)
{
	Lump* nlump = new Lump(0, 0, name, this);

	if (index < 0 || index >= directory.size())
		directory.push_back(nlump);
	else
		directory.insert(directory.begin() + index, nlump);

	return nlump;
}

// Wad::replace_lump: Replaces lump data with new data
// ------------------------------------------------ >>
void Wad::replace_lump(string name, DWORD new_size, BYTE *data, DWORD offset)
{
	int index = get_lump_index(name, offset);

	if (index > -1)
	{
		//free(directory[index]->Data());
		directory[index]->Resize(new_size);
		memcpy(directory[index]->Data(), data, new_size);
	}
}

// Wad::delete_lump: Deletes a lump
// ----------------------------- >>
void Wad::delete_lump(string name, DWORD offset)
{
	long index = -1;

	for (DWORD l = offset; l < num_lumps; l++)
	{
		if (directory[l]->Name().CmpNoCase(name) == 0)
		{
			index = l;
			break;
		}
	}

	if (index == -1)
		return;

	delete directory[index];

	for (DWORD l = index; l < num_lumps - 1; l++)
		directory[l] = directory[l + 1];

	num_lumps--;
	directory = (Lump **)realloc(directory, num_lumps * sizeof(Lump *));
}

void copy_file(string input, string output)
{
	FILE *in = fopen(str(input).c_str(), "rb");
	if (in)
	{
		FILE *out = fopen(str(output).c_str(), "wb");
		while (!feof(in))
		{
			BYTE b = 0;
			lefread(&b, 1, 1, in);
			lefwrite(&b, 1, 1, out);
		}
		fclose(out);
		fclose(in);
	}
}

// Wad::save: Saves a wad file to disk (backs up the file, then overwrites it)
// ------------------------------------------------------------------------ >>
void Wad::save(bool nodes, string mapname)
{
	if (locked)
	{
		message_box(_T("Saving to the IWAD is disallowed. Use 'Save As' instead."), _T("Save Disallowed"));
		return;
	}

	string bakfile = path + _T(".bak");
	//printf("%s\n%s\n", path.c_str(), bakfile.c_str());

	// Remove old backup file
	remove(str(bakfile).c_str());

	// Copy current file contents to new backup file
	copy_file(path, bakfile);

	// Determine directory offset & individual lump offsets
	dir_offset = 12;

	for (DWORD l = 0; l < num_lumps; l++)
	{
		directory[l]->SetOffset(dir_offset);
		dir_offset += directory[l]->Size();
	}

	// Open wadfile for writing
	FILE *fp = fopen(str(path).c_str(), "wb");

	// Write the header
	lefwrite(type, 1, 4, fp);
	lefwrite(&num_lumps, 4, 1, fp);
	lefwrite(&dir_offset, 4, 1, fp);

	// Write the lumps
	for (DWORD l = 0; l < num_lumps; l++)
		lefwrite(directory[l]->Data(), 1, directory[l]->Size(), fp);

	// Write the directory
	for (DWORD l = 0; l < num_lumps; l++)
	{
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		long offset = directory[l]->Offset();
		long size = directory[l]->Size();

		for (int c = 0; c < directory[l]->Name().length(); c++)
			name[c] = directory[l]->Name()[c];

		lefwrite(&offset, 4, 1, fp);
		lefwrite(&size, 4, 1, fp);
		lefwrite(name, 1, 8, fp);
	}

	fclose(fp);

	// Build nodes
	if (nodes)
	{
		splash(_T("Building Nodes"), true);

		FWadReader inwad(str(path).c_str());
		FWadWriter outwad(str(s_fmt(_T("%s.temp"), path)).c_str(), inwad.IsIWAD());

		int lump = 0;
		int max = inwad.NumLumps();

		while (lump < max)
		{
			if (inwad.IsMap (lump) &&
				(mapname == _T("") || mapname.CmpNoCase(inwad.LumpName(lump)) == 0)) //stricmp (inwad.LumpName (lump), str(mapname).c_str()) == 0))
			{
				splash(s_fmt(_T("Building nodes on %s"), inwad.LumpName(lump)), true);
				FProcessor builder(inwad, lump);
				builder.Write (outwad);

				lump = inwad.LumpAfterMap (lump);
			}
			else if (inwad.IsGLNodes (lump))
			{
				// Ignore GL nodes from the input for any maps we process.
				if (BuildNodes && (Map == NULL || stricmp (inwad.LumpName (lump)+3, Map) == 0))
					lump = inwad.SkipGLNodes (lump);
				else
				{
					outwad.CopyLump (inwad, lump);
					++lump;
				}
			}
			else
			{
				outwad.CopyLump (inwad, lump);
				++lump;
			}
		}

		outwad.Close();

		remove(path.c_str());
		copy_file(s_fmt(_T("%s.temp"), path), path);
		remove(str(s_fmt("%s.temp", path)).c_str());

		splash_hide();
	}
}

void Wad::close()
{
	if (path == _T(""))
		return;

	available_maps.clear();

	for (int a = 0; a < num_lumps; a++)
		delete directory[a];

	free(directory);
	directory = NULL;

	num_lumps = 0;
	path = "";
	locked = true;
}

void Wad::calculate_offsets()
{
	// Determine directory offset & individual lump offsets
	dir_offset = 12;

	for (DWORD l = 0; l < num_lumps; l++)
	{
		directory[l]->SetOffset(dir_offset);
		dir_offset += directory[l]->Size();
	}
}

*/
