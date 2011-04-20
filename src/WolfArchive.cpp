
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    WolfArchive.cpp
 * Description: WolfArchive, archive class to handle Wolf 3D data.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WolfArchive.h"
#include "SplashWindow.h"
#include <wx/log.h>
#include <wx/filename.h>

/*******************************************************************
 * ADDITIONAL FUNCTIONS
 *******************************************************************/

/* WolfConstant
 * Returns a Wolf constant depending on the size of the archive.
 * Anyone who finds that the Doom source code is hacky should take
 * a look at how Wolf3D was coded. It's a wonder it works at all.
 *******************************************************************/
enum WolfConstants {
	NUMTILE8,
	STARTPICS,
	STARTPICM,
	STARTSPRITES,
	STARTTILE8,
	LATCHPICS_LUMP_START,
	LATCHPICS_LUMP_END,
	LATCH_START = LATCHPICS_LUMP_START,
	LATCH_END = LATCHPICS_LUMP_END,
	STARTPAL,
	ENDPAL,
	TITLE1PIC,
	TITLE2PIC,
	ENDSCREEN1PIC,
	ENDSCREEN9PIC,
	IDGUYS1PIC,
	IDGUYS2PIC,
};
#define return4(a, b, c, d, e) switch(a) { case 0: return b; case 1: return c; case 2: return d; case 3: return e; }
size_t WolfConstant(int name, size_t numlumps) {
	int game = 0; // 0: wolf shareware, 1: wolf full, 2: spear demo, 3: spear full
	switch (numlumps) {
		case 556: break;
		case 149: game = 1; break;
		case 133: game = 2; break;
		case 169: game = 3; break;
		default: return 0;
	}
	switch (name) {
		case LATCH_START:	return4(game,  95,  91,  79, 101) break;
		case LATCH_END:		return4(game, 138, 134, 127, 149) break;
		case STARTPICS:		return4(game,   3,   3,   3,   3) break;
		case STARTPICM:		return4(game, 139, 135, 128, 150) break;
		case STARTSPRITES:	return4(game, 139, 135, 128, 150) break;
		case STARTTILE8:	return4(game, 139, 135, 128, 150) break;
		case NUMTILE8:		return4(game,  72,  72,  72,  72) break;
		case STARTPAL:		return4(game,   0,   0, 131, 153) break;
		case ENDPAL:		return4(game,   0,   0, 131, 163) break;
		case TITLE1PIC:		return4(game,   0,   0,  74,  79) break;
		case TITLE2PIC:		return4(game,   0,   0,  75,  80) break;
		case ENDSCREEN1PIC:	return4(game,   0,   0,   0,  81) break;
		case ENDSCREEN9PIC:	return4(game,   0,   0,   0,  89) break;
		case IDGUYS1PIC:	return4(game,   0,   0,   0,  93) break;
		case IDGUYS2PIC:	return4(game,   0,   0,   0,  94) break;
	}
	return 0;
}

/* searchIMFName
 * Looks for the string naming the song, at no fixed place towards
 * the end of the file. Returns an empty string if nothing is found.
 *******************************************************************/
string searchIMFName(MemChunk& mc) {
	string ret = "";
	if (mc.getSize() > 90) {
		size_t counter = mc.getSize() - 1;
		size_t stop = counter - 88;
		while (counter > stop) {
			if (mc[counter] == 'F' && mc[counter - 1] == 'M'
				&& mc[counter - 2] == 'I' && mc[counter - 3] == '.')
			{
				size_t start = counter - 4;
				while ((mc[start] >= 'A' && mc[start] <= 'Z') || mc[start] == '-'
					|| (mc[start] >= '0' && mc[start] <= '9') || mc[start] == '!') {
					start--;
				}
				if (counter - start < 13 && mc[start] == '\\') {
					for (size_t i = 1; start + i <= counter; ++i)
						ret += mc[start + i];
				}
				break;
			}
			counter--;
		}
	}
	return ret;
}

/* addWolfPicHeader
 * Adds height and width information to a picture. Needed because
 * Wolfenstein 3-D is just that much of a horrible hacky mess.
 *******************************************************************/
void addWolfPicHeader(ArchiveEntry * entry, uint16_t width, uint16_t height) {
	if (!entry)
		return;

	MemChunk& mc = entry->getMCData();
	if (mc.getSize() == 0)
		return;

	uint32_t newsize = mc.getSize() + 4;
	uint8_t * newdata = new uint8_t[newsize];

	newdata[0] = (uint8_t) (width & 0xFF);
	newdata[1] = (uint8_t) (width>>8);
	newdata[2] = (uint8_t) (height & 0xFF);
	newdata[3] = (uint8_t) (height>>8);

	for (size_t i = 0; 4 + i < newsize; ++i) {
		newdata[4 + i] = mc[i];
	}
	//mc.clear();
	entry->importMem(newdata, newsize);
	delete[] newdata;
}

/* addIMFHeader
 * Automatizes this: http://zdoom.org/wiki/Using_OPL_music_in_ZDoom
 *******************************************************************/
void addIMFHeader(ArchiveEntry * entry) {
	if (!entry)
		return;

	MemChunk& mc = entry->getMCData();
	if (mc.getSize() == 0)
		return;

	uint32_t newsize = mc.getSize() + 9;
	uint8_t start = 0;
	if (mc[0] | mc[1]) {
		// non-zero start
		newsize += 2;
		start = 2;
	} else newsize += 4;

	uint8_t * newdata = new uint8_t[newsize];
	newdata[0] = 'A'; newdata[1] = 'D'; newdata[2] = 'L'; newdata[3] = 'I'; newdata[4] = 'B';
	newdata[5] = 1; newdata[6] = 0; newdata[7] = 0; newdata[8] = 1;
	if (mc[0] | mc[1]) {
		newdata[9] = mc[0]; newdata[10] = mc[1]; newdata[11] = 0; newdata[12] = 0;
	} else {
		newdata[9] = 0; newdata[10] = 0; newdata[11] = 0; newdata[12] = 0;
	}
	for (size_t i = 0; ((i + start < mc.getSize()) && (13 + i < newsize)); ++i) {
		newdata[13 + i] = mc[i+start];
	}
	//mc.clear();
	entry->importMem(newdata, newsize);
	delete[] newdata;
}

/* ExpandWolfGraphLump
 * Needed to read VGAGRAPH content. Adapted from Wolf3D code, but
 * with dead code removed from it.
 *******************************************************************/
struct huffnode {
	uint16_t bit0, bit1;	// 0-255 is a character, > is a pointer to a node
};
void ExpandWolfGraphLump (ArchiveEntry * entry, size_t lumpnum, size_t numlumps, huffnode *hufftable)
{
	if (!entry || entry->getSize() == 0)
		return;

	size_t expanded; // expanded size
	const uint8_t* source = entry->getData();


	if (lumpnum == WolfConstant(STARTTILE8, numlumps))
		expanded = 64 * WolfConstant(NUMTILE8, numlumps);
	else {
		expanded = *(uint32_t *)source;
		source += 4;			// skip over length
	}

	if (expanded == 0 || expanded > 65000) {
		wxLogMessage("ExpandWolfGraphLump: invalid expanded size in entry %d", lumpnum);
		return;
	}

	uint8_t *dest = new uint8_t[expanded];
    uint8_t *end, *start;
    huffnode *headptr, *huffptr;

    headptr = hufftable+254;        // head node is always node 254

    size_t written = 0;

    end = dest+expanded;
	start = dest;

    uint8_t val = *source++;
    uint8_t mask = 1;
    uint16_t nodeval = 0;
    huffptr = headptr;
    while(1) {
        if(!(val & mask))		nodeval = huffptr->bit0;
        else					nodeval = huffptr->bit1;
        if(mask==0x80)			val = *source++, mask = 1;
        else					mask <<= 1;

		if(nodeval<256) {
			*dest++ = (uint8_t) nodeval;
			written++;
			huffptr = headptr;
			if(dest>=end) break;
		} else if (nodeval < 512) {
			huffptr = hufftable + (nodeval - 256);
		} else wxLogMessage("ExpandWolfGraphLump: nodeval is out of control (%d) in entry %d", nodeval, lumpnum);
    }

	entry->importMem(start, expanded);
	delete[] start;
}

/*******************************************************************
 * VARIABLES
 *******************************************************************/

/*******************************************************************
 * WOLFARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* WolfArchive::WolfArchive
 * WolfArchive class constructor
 *******************************************************************/
WolfArchive::WolfArchive()
: TreelessArchive(ARCHIVE_WOLF) {
}

/* WolfArchive::~WolfArchive
 * WolfArchive class destructor
 *******************************************************************/
WolfArchive::~WolfArchive() {
}

/* WolfArchive::getEntryOffset
 * Gets a lump entry's offset
 * Returns the lump entry's offset, or zero if it doesn't exist
 *******************************************************************/
uint32_t WolfArchive::getEntryOffset(ArchiveEntry* entry) {
	return uint32_t((int)entry->exProp("Offset"));
}

/* WolfArchive::setEntryOffset
 * Sets a lump entry's offset
 *******************************************************************/
void WolfArchive::setEntryOffset(ArchiveEntry* entry, uint32_t offset) {
	entry->exProp("Offset") = (int)offset;
}


/* WolfArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string WolfArchive::getFileExtensionString() {
	return "Wolfenstein 3D Files (*.wl1; *.wl6; *.sod; *.sd?)|*.wl1;*.wl6;*.sod;*.sd1;*.sd2;*.sd3";
}

/* WolfArchive::getFormat
 * Gives the "archive_dat" string
 *******************************************************************/
string WolfArchive::getFormat() {
	return "archive_wolf";
}

/* WolfArchive::open
 * Reads a Wolf format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WolfArchive::open(string filename) {
	// Find wolf archive type
	wxFileName fn1(filename);
	bool opened = false;
	if (fn1.GetName().MakeUpper() == "MAPHEAD" || fn1.GetName().MakeUpper() == "GAMEMAPS") {
		wxFileName fn2(fn1);
		fn1.SetName("MAPHEAD");
		fn2.SetName("GAMEMAPS");
		MemChunk data, head;
		head.importFile(fn1.GetFullPath());
		data.importFile(fn2.GetFullPath());
		opened = openMaps(head, data);
	} else if (fn1.GetName().MakeUpper() == "AUDIOHED" || fn1.GetName().MakeUpper() == "AUDIOT") {
		wxFileName fn2(fn1);
		fn1.SetName("AUDIOHED");
		fn2.SetName("AUDIOT");
		MemChunk data, head;
		head.importFile(fn1.GetFullPath());
		data.importFile(fn2.GetFullPath());
		opened = openAudio(head, data);
	} else if (fn1.GetName().MakeUpper() == "VGAHEAD" || fn1.GetName().MakeUpper() == "VGAGRAPH" || fn1.GetName().MakeUpper() == "VGADICT") {
		wxFileName fn2(fn1);
		wxFileName fn3(fn1);
		fn1.SetName("VGAHEAD");
		fn2.SetName("VGAGRAPH");
		fn3.SetName("VGADICT");
		MemChunk data, head, dict;
		head.importFile(fn1.GetFullPath());
		data.importFile(fn2.GetFullPath());
		dict.importFile(fn3.GetFullPath());
		opened = openGraph(head, data, dict);
	} else {
		// Read the file into a MemChunk
		MemChunk mc;
		if (!mc.importFile(filename)) {
			Global::error = "Unable to open file. Make sure it isn't in use by another program.";
			return false;
		}
		// Load from MemChunk
		opened = open(mc);
	}

	if (opened) {
		// Update variables
		this->filename = filename;
		this->on_disk = true;

		return true;
	}
	else
		return false;
}

/* WolfArchive::open
 * Reads VSWAP Wolf format data from a MemChunk.
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WolfArchive::open(MemChunk& mc) {
	// Check data was given
	if (!mc.hasData())
		return false;

	// Read Wolf header
	mc.seek(0, SEEK_SET);
	uint16_t num_lumps;
	mc.read(&num_lumps, 2);		// Number of chunks
	mc.read(&spritestart, 2);	// First sprite
	mc.read(&soundstart, 2);	// First sound
	num_lumps	= wxINT16_SWAP_ON_BE(num_lumps);
	spritestart	= wxINT16_SWAP_ON_BE(spritestart);
	soundstart	= wxINT16_SWAP_ON_BE(soundstart);

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the offsets
	theSplashWindow->setProgressMessage("Reading Wolf archive data");
	WolfHandle * pages = new WolfHandle[num_lumps];
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_lumps*2.0f));

		// Read offset info
		uint32_t offset = 0;
		mc.read(&offset,	4);		// Offset
		pages[d].offset = wxINT32_SWAP_ON_BE(offset);

		// If the lump data goes before the end of the directory,
		// the data file is invalid
		if (pages[d].offset < (unsigned)((num_lumps + 1) * 6)) {
			delete[] pages;
			wxLogMessage("WolfArchive::open: Wolf archive is invalid or corrupt");
			Global::error = "Archive is invalid and/or corrupt ";
			setMuted(false);
			return false;
		}
	}

	// Then read the sizes
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)(d + num_lumps) / (float)num_lumps*2.0f));

		// Read size info
		uint16_t size = 0;
		mc.read(&size,	2);		// Offset
		size	= wxINT16_SWAP_ON_BE(size);

		// Wolf chunks have no names, so just give them a number
		string name;
		if (d < spritestart)		name = "WAL";
		else if (d < soundstart)	name = "SPR";
		else						name = "SND";
		name += S_FMT("%05d", d);

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(name, size);
		nlump->setLoaded(false);
		nlump->exProp("Offset") = (int)pages[d].offset;
		nlump->setState(0);

		// Add to entry list
		getRoot()->addEntry(nlump);

		// If the lump data goes past the end of file,
		// the data file is invalid
		if (getEntryOffset(nlump) + size > mc.getSize()) {
			delete[] pages;
			wxLogMessage("WolfArchive::open: Wolf archive is invalid or corrupt");
			Global::error = "Archive is invalid and/or corrupt";
			setMuted(false);
			return false;
		}

	}
	// Cleanup
	delete[] pages;

	// Detect all entry types
	MemChunk edata;
	theSplashWindow->setProgressMessage("Detecting entry types");
	for (size_t a = 0; a < numEntries(); a++) {
		// Update splash window progress
		theSplashWindow->setProgress((((float)a / (float)num_lumps)));

		// Get entry
		ArchiveEntry* entry = getEntry(a);

		// Read entry data if it isn't zero-sized
		if (entry->getSize() > 0) {
			// Read the entry data
			mc.exportMemChunk(edata, getEntryOffset(entry), entry->getSize());
			entry->importMemChunk(edata);
		}

		// Detect entry type
		EntryType::detectEntryType(entry);

		// Set entry to unchanged
		entry->setState(0);
	}

	// Setup variables
	setMuted(false);
	setModified(false);
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

/* WolfArchive::openAudio
 * Reads Wolf AUDIOT/AUDIOHEAD format data from a MemChunk.
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WolfArchive::openAudio(MemChunk& head, MemChunk& data) {
	// Check data was given
	if (!head.hasData() || !data.hasData())
		return false;

	// Read Wolf header file
	uint32_t num_lumps = (head.getSize()>>2) - 1;
	spritestart	= soundstart = num_lumps;

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the offsets
	theSplashWindow->setProgressMessage("Reading Wolf archive data");
	const uint32_t * offsets = (const uint32_t *) head.getData();
	MemChunk edata;
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_lumps));

		// Read offset info
		uint32_t offset = wxINT32_SWAP_ON_BE(offsets[d]);
		uint32_t size = wxINT32_SWAP_ON_BE(offsets[d+1]) - offset;

		// If the lump data goes before the end of the directory,
		// the data file is invalid
		if (offset + size > data.getSize()) {
			wxLogMessage("WolfArchive::openAudio: Wolf archive is invalid or corrupt");
			Global::error = S_FMT("Archive is invalid and/or corrupt in entry %d", d);
			setMuted(false);
			return false;
		}

		// Read entry data if it isn't zero-sized
		if (size > 0) {
			// Read the entry data
			data.exportMemChunk(edata, offset, size);
		}

		// Wolf chunks have no names, so just give them a number
		string name = searchIMFName(edata);
		if (name == "")
			name = S_FMT("LMP%05d", d);

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(name, size);
		nlump->setLoaded(false);
		nlump->exProp("Offset") = (int)offset;

		// Detect entry type
		if (size > 0) nlump->importMemChunk(edata);
		EntryType::detectEntryType(nlump);

		// Add to entry list
		nlump->setState(0);
		getRoot()->addEntry(nlump);
	}

	// Setup variables
	setMuted(false);
	setModified(false);
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

/* WolfArchive::openMaps
 * Reads Wolf GAMEMAPS/MAPHEAD format data from a MemChunk.
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WolfArchive::openMaps(MemChunk& head, MemChunk& data) {
	// Check data was given
	if (!head.hasData() || !data.hasData())
		return false;

	// Read Wolf header file
	uint32_t num_lumps = (head.getSize()- 2) >> 2;
	spritestart	= soundstart = num_lumps;

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the offsets
	theSplashWindow->setProgressMessage("Reading Wolf archive data");
	const uint32_t * offsets = (const uint32_t *) (2 + head.getData());
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_lumps));

		// Read offset info
		uint32_t offset = wxINT32_SWAP_ON_BE(offsets[d]);
		uint32_t size = 38;

		// If the lump data goes before the end of the directory,
		// the data file is invalid
		if (offset + size > data.getSize()) {
			wxLogMessage("WolfArchive::openMaps: Wolf archive is invalid or corrupt");
			Global::error = S_FMT("Archive is invalid and/or corrupt in entry %d", d);
			setMuted(false);
			return false;
		}

		if (offset == 0 && d > 0) continue;

		string name = "";
		for (size_t i = 0; i < 16; ++i) {
			name += data[offset + 22 + i];
			if (data[offset + 22 + i] == 0)
				break;
		}

		ArchiveEntry* nlump = new ArchiveEntry(name, size);
		nlump->setLoaded(false);
		nlump->exProp("Offset") = (int)offset;
		nlump->setState(0);

		// Add to entry list
		getRoot()->addEntry(nlump);

		// Add map planes to entry list
		uint32_t planeofs[3];
		uint16_t planelen[3];
		planeofs[0] = READ_L32(data, offset);
		planeofs[1] = READ_L32(data, offset + 4);
		planeofs[2] = READ_L32(data, offset + 8);
		planelen[0] = READ_L16(data, offset +12);
		planelen[1] = READ_L16(data, offset +14);
		planelen[2] = READ_L16(data, offset +16);
		for (int i = 0; i < 3; ++i) {
			name = S_FMT("PLANE%d", i);
			nlump = new ArchiveEntry(name, planelen[i]);
			nlump->setLoaded(false);
			nlump->exProp("Offset") = (int)planeofs[i];
			nlump->setState(0);
			getRoot()->addEntry(nlump);
		}
	}

	// Detect all entry types
	MemChunk edata;
	theSplashWindow->setProgressMessage("Detecting entry types");
	for (size_t a = 0; a < numEntries(); a++) {
		// Update splash window progress
		theSplashWindow->setProgress((((float)a / (float)num_lumps)));

		// Get entry
		ArchiveEntry* entry = getEntry(a);

		// Read entry data if it isn't zero-sized
		if (entry->getSize() > 0) {
			// Read the entry data
			data.exportMemChunk(edata, getEntryOffset(entry), entry->getSize());
			entry->importMemChunk(edata);
		}

		// Detect entry type
		EntryType::detectEntryType(entry);

		// Set entry to unchanged
		entry->setState(0);
	}

	// Setup variables
	setMuted(false);
	setModified(false);
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

/* WolfArchive::openGraph
 * Reads Wolf VGAGRAPH/VGAHEAD/VGADICT format data from a MemChunk.
 * Returns true if successful, false otherwise
 *******************************************************************/
#define WC(a) WolfConstant(a, num_lumps)
bool WolfArchive::openGraph(MemChunk& head, MemChunk& data, MemChunk& dict) {
	// Check data was given
	if (!head.hasData() || !data.hasData() || !dict.hasData())
		return false;

	if (dict.getSize() != 1024) {
		Global::error = S_FMT("WolfArchive::openGraph: VGADICT is improperly sized (%d bytes instead of 1024)", dict.getSize());
		return false;
	}
	huffnode nodes[256];
	memcpy(nodes, dict.getData(), 1024);

	// Read Wolf header file
	uint32_t num_lumps = (head.getSize() / 3) - 1;
	spritestart	= soundstart = num_lumps;

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the offsets
	theSplashWindow->setProgressMessage("Reading Wolf archive data");
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_lumps));

		// Read offset info
		uint32_t offset = READ_L24(head, (d * 3));

		// Compute size from next offset
		uint32_t size = /*((d == num_lumps - 1) ? data.getSize() - offset :*/(
			READ_L24(head, ((d+1) * 3)) - offset);

		// If the lump data goes before the end of the directory,
		// the data file is invalid
		if (offset + size > data.getSize()) {
			wxLogMessage("WolfArchive::openGraph: Wolf archive is invalid or corrupt");
			Global::error = S_FMT("Archive is invalid and/or corrupt in entry %d", d);
			setMuted(false);
			return false;
		}

		// Wolf chunks have no names, so just give them a number
		string name;
		if (d == 0)														name = "INF";
		else if (d == 1 || d == 2)										name = "FNT";
		else if (d >= WC(STARTPICS)) {
			if (d >= WC(STARTPAL) && d < WC(ENDPAL))					name = "PAL";
			else if (d == WC(TITLE1PIC) || d == WC(TITLE2PIC))			name = "TIT";
			else if (d == WC(IDGUYS1PIC)|| d == WC(IDGUYS1PIC))			name = "IDG";
			else if (d >= WC(ENDSCREEN1PIC) && d <= WC(ENDSCREEN9PIC))	name = "END";
			else if (d < WC(STARTPICM)) 								name = "PIC";
			else														name = "LMP";
		}
		else name = "LMP";
		name += S_FMT("%05d", d);


		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(name, size);
		nlump->setLoaded(false);
		nlump->exProp("Offset") = (int)offset;
		nlump->setState(0);

		// Add to entry list
		getRoot()->addEntry(nlump);
	}

	// Detect all entry types
	MemChunk edata;
	const uint16_t * pictable;
	theSplashWindow->setProgressMessage("Detecting entry types");
	for (size_t a = 0; a < numEntries(); a++) {
		//wxLogMessage(s_fmt("Entry %d/%d", a, numEntries()));
		// Update splash window progress
		theSplashWindow->setProgress((((float)a / (float)num_lumps)));

		// Get entry
		ArchiveEntry* entry = getEntry(a);

		// Read entry data if it isn't zero-sized
		if (entry->getSize() > 0) {
			// Read the entry data
			data.exportMemChunk(edata, getEntryOffset(entry), entry->getSize());
			entry->importMemChunk(edata);
		}
		ExpandWolfGraphLump(entry, a, num_lumps, nodes);

		// Store pictable information
		if (a == 0)
			pictable = (uint16_t *) entry->getData();
		else if (a >= WC(STARTPICS) && a < WC(STARTPICM)) {
			size_t i = (a - WC(STARTPICS))<<1;
			addWolfPicHeader(entry, pictable[i], pictable[i+1]);
		}

		// Detect entry type
		EntryType::detectEntryType(entry);

		// Set entry to unchanged
		entry->setState(0);
	}

	// Setup variables
	setMuted(false);
	setModified(false);
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

/* WolfArchive::addEntry
 * Override of Archive::addEntry to force entry addition to the root
 * directory, and update namespaces if needed
 *******************************************************************/
ArchiveEntry* WolfArchive::addEntry(ArchiveEntry* entry, unsigned position, ArchiveTreeNode* dir, bool copy) {
	// Check entry
	if (!entry)
		return NULL;

	// Check if read-only
	if (isReadOnly())
		return NULL;

	// Copy if necessary
	if (copy)
		entry = new ArchiveEntry(*entry);

	// Do default entry addition (to root directory)
	Archive::addEntry(entry, position);

	return entry;
}

/* WolfArchive::addEntry
 * Since there are no namespaces, just give the hot potato to the
 * other function and call it a day.
 *******************************************************************/
ArchiveEntry* WolfArchive::addEntry(ArchiveEntry* entry, string add_namespace, bool copy) {
	return addEntry(entry, 0xFFFFFFFF, NULL, copy);
}

/* WolfArchive::renameEntry
 * Wolf chunks have no names, so renaming is pointless.
 *******************************************************************/
bool WolfArchive::renameEntry(ArchiveEntry* entry, string name) {
	return false;
}

/* WolfArchive::write
 * Writes the dat archive to a file
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WolfArchive::write(string filename, bool update) {
	// Write to a MemChunk, then export it to a file
	MemChunk mc;
	if (write(mc, true))
		return mc.exportFile(filename);
	else
		return false;
}

/* WolfArchive::write
 * Writes the dat archive to a MemChunk
 * Returns true if successful, false otherwise [Not implemented]
 *******************************************************************/
bool WolfArchive::write(MemChunk& mc, bool update) {
	return false;
}

/* WolfArchive::loadEntryData
 * Loads an entry's data from the datafile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WolfArchive::loadEntryData(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return false;

	// Do nothing if the lump's size is zero,
	// or if it has already been loaded
	if (entry->getSize() == 0 || entry->isLoaded()) {
		entry->setLoaded();
		return true;
	}

	// Open wadfile
	wxFile file(filename);

	// Check if opening the file failed
	if (!file.IsOpened()) {
		wxLogMessage("WolfArchive::loadEntryData: Failed to open datfile %s", filename.c_str());
		return false;
	}

	// Seek to lump offset in file and read it in
	file.Seek(getEntryOffset(entry), wxFromStart);
	entry->importFileStream(file, entry->getSize());

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* WolfArchive::isWolfArchive
 * Checks if the given data is a valid Wolfenstein VSWAP archive
 *******************************************************************/
bool WolfArchive::isWolfArchive(MemChunk& mc) {
	// Read Wolf header
	mc.seek(0, SEEK_SET);
	uint16_t num_lumps, sprites, sounds;
	mc.read(&num_lumps, 2);		// Size
	num_lumps	= wxINT16_SWAP_ON_BE(num_lumps);
	if (num_lumps == 0)
		return false;

	mc.read(&sprites, 2);		// Sprites start
	mc.read(&sounds, 2);		// Sounds start
	sprites		= wxINT16_SWAP_ON_BE(sprites);
	sounds		= wxINT16_SWAP_ON_BE(sounds);
	if (sprites > sounds)
		return false;

	// Read lump info
	uint32_t offset = 0;
	uint16_t size = 0;
	size_t totalsize = 6 * (num_lumps + 1);
	size_t pagesize = (totalsize / 512) + ((totalsize % 512) ? 1 : 0);
	size_t filesize = mc.getSize();
	if (filesize < totalsize)
		return false;

	WolfHandle * pages = new WolfHandle[num_lumps];
	uint32_t lastoffset = 0;
	for (size_t a = 0; a < num_lumps; ++a) {
		mc.read(&offset, 4);
		offset = wxINT32_SWAP_ON_BE(offset);
		if (offset < lastoffset || offset % 512) {
			delete[] pages;
			return false;
		}
		lastoffset = offset;
		pages[a].offset = offset;
	}
	uint16_t lastsize = 0;
	for (size_t b = 0; b < num_lumps; ++b) {
		mc.read(&size, 2);
		size = wxINT16_SWAP_ON_BE(size);
		pagesize += (size / 512) + ((size % 512) ? 1 : 0);
		pages[b].size = size;
		if (b > 0 && (pages[b - 1].offset + pages[b - 1].size) > pages[b].offset) {
			delete[] pages;
			return false;
		}
		lastsize = size;
	}
	delete[] pages;
	return ((pagesize * 512) <= filesize || filesize >= lastoffset + lastsize);
}

/* WolfArchive::isWolfArchive
 * Checks if the file at [filename] is a valid Wolfenstein VSWAP archive
 *******************************************************************/
bool WolfArchive::isWolfArchive(string filename) {
	// Find wolf archive type
	wxFileName fn1(filename);
	if (fn1.GetName().MakeUpper() == "MAPHEAD" || fn1.GetName().MakeUpper() == "GAMEMAPS") {
		wxFileName fn2(fn1);
		fn1.SetName("MAPHEAD");
		fn2.SetName("GAMEMAPS");
		return (wxFile::Exists(fn1.GetFullPath()) && wxFile::Exists(fn2.GetFullPath()));
	} else if (fn1.GetName().MakeUpper() == "AUDIOHED" || fn1.GetName().MakeUpper() == "AUDIOT") {
		wxFileName fn2(fn1);
		fn1.SetName("AUDIOHED");
		fn2.SetName("AUDIOT");
		return (wxFile::Exists(fn1.GetFullPath()) && wxFile::Exists(fn2.GetFullPath()));
	} else if (fn1.GetName().MakeUpper() == "VGAHEAD" || fn1.GetName().MakeUpper() == "VGAGRAPH"
		|| fn1.GetName().MakeUpper() == "VGADICT") {
		wxFileName fn2(fn1);
		wxFileName fn3(fn1);
		fn1.SetName("VGAHEAD");
		fn2.SetName("VGAGRAPH");
		fn3.SetName("VGADICT");
		return (wxFile::Exists(fn1.GetFullPath()) && wxFile::Exists(fn2.GetFullPath())
			&& wxFile::Exists(fn3.GetFullPath()));
	}

	// else we have to deal with a VSWAP archive, which is the only self-contained type

	// Open file for reading
	wxFile file(filename);

	// Check it opened ok
	if (!file.IsOpened())
		return false;

	// Read Wolf header
	file.Seek(0, wxFromStart);
	uint16_t num_lumps, sprites, sounds;

	file.Read(&num_lumps, 2);	// Size
	num_lumps	= wxINT16_SWAP_ON_BE(num_lumps);
	if (num_lumps == 0)
		return false;

	file.Read(&sprites, 2);		// Sprites start
	file.Read(&sounds, 2);		// Sounds start
	sprites		= wxINT16_SWAP_ON_BE(sprites);
	sounds		= wxINT16_SWAP_ON_BE(sounds);
	if (sprites > sounds)
		return false;

	// Read lump info
	uint32_t offset = 0;
	uint16_t size = 0;
	size_t totalsize = 6 * (num_lumps + 1);
	size_t pagesize = (totalsize / 512) + ((totalsize % 512) ? 1 : 0);
	size_t filesize = file.Length();
	if (filesize < totalsize)
		return false;

	WolfHandle * pages = new WolfHandle[num_lumps];
	uint32_t lastoffset = 0;
	for (size_t a = 0; a < num_lumps; ++a) {
		file.Read(&offset, 4);
		offset = wxINT32_SWAP_ON_BE(offset);
		if (offset < lastoffset || offset % 512) {
			delete[] pages;
			return false;
		}
		lastoffset = offset;
		pages[a].offset = offset;
	}
	uint16_t lastsize = 0;
	for (size_t b = 0; b < num_lumps; ++b) {
		file.Read(&size, 2);
		size = wxINT16_SWAP_ON_BE(size);
		pagesize += (size / 512) + ((size % 512) ? 1 : 0);
		pages[b].size = size;
		if (b > 0 && (pages[b - 1].offset + pages[b - 1].size) > pages[b].offset) {
			delete[] pages;
			return false;
		}
		lastsize = size;
	}
	delete[] pages;
	return ((pagesize * 512) <= filesize || filesize >= lastoffset + lastsize);
}

/*******************************************************************
 * EXTRA CONSOLE COMMANDS
 *******************************************************************/
#include "Console.h"
#include "MainWindow.h"

CONSOLE_COMMAND(addimfheader, 0) {
	vector<ArchiveEntry *> entries = theMainWindow->getCurrentEntrySelection();

	for (size_t i = 0; i < entries.size(); ++i)
		addIMFHeader(entries[i]);
}

