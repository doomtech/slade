
#ifndef __WADARCHIVE_H__
#define	__WADARCHIVE_H__

#include "Archive.h"

class WadArchive : public Archive {
private:
	vector<ArchiveEntry*>	entries;
	char					wad_type[4];
	ArchiveEntry*			patches[2];
	ArchiveEntry*			sprites[2];
	ArchiveEntry*			flats[2];
	ArchiveEntry*			tx[2];

public:
	WadArchive();
	~WadArchive();

	// Wad specific
	bool			isIWAD();
	ArchiveEntry*	patchesBegin() { return patches[0]; }
	ArchiveEntry*	patchesEnd() { return patches[1]; }
	ArchiveEntry*	spritesBegin() { return sprites[0]; }
	ArchiveEntry*	spritesEnd() { return sprites[1]; }
	ArchiveEntry*	flatsBegin() { return flats[0]; }
	ArchiveEntry*	flatsEnd() { return flats[1]; }
	ArchiveEntry*	txBegin() { return tx[0]; }
	ArchiveEntry*	txEnd() { return tx[1]; }
	void			setPatchMarkers(ArchiveEntry* begin, ArchiveEntry* end) { patches[0] = begin; patches[1] = end; }
	void			setSpriteMarkers(ArchiveEntry* begin, ArchiveEntry* end) { sprites[0] = begin; sprites[1] = end; }
	void			setFlatMarkers(ArchiveEntry* begin, ArchiveEntry* end) { flats[0] = begin; flats[1] = end; }
	void			setTxMarkers(ArchiveEntry* begin, ArchiveEntry* end) { tx[0] = begin; tx[1] = end; }

	int				entryIndex(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(uint32_t index);
	ArchiveEntry*	getEntry(string name);
	string			getFileExtensionString();
	string			getFormat();

	bool	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	open(MemChunk& mc);

	bool	write(MemChunk& mc, bool update = true);
	bool	write(string filename, bool update = true);

	void	close();

	uint32_t	getEntryOffset(ArchiveEntry* entry);
	void		setEntryOffset(ArchiveEntry* entry, uint32_t offset);
	bool		loadEntryData(ArchiveEntry* entry);
	uint32_t	numEntries();

	bool			addEntry(ArchiveEntry* entry, uint32_t position = 0);
	ArchiveEntry*	addNewEntry(string name = "", uint32_t position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, uint32_t position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);

	bool	renameEntry(ArchiveEntry* entry, string new_name);

	vector<mapdesc_t>	detectMaps();
	string				detectEntrySection(ArchiveEntry* entry);

	ArchiveEntry*			findEntry(string search, bool includesubdirs = true);
	ArchiveEntry*			findEntry(int edftype, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(string search, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(int edftype, bool includesubdirs = true);

	static bool exportEntriesAsWad(string filename, vector<ArchiveEntry*> entries) {
		WadArchive wad;

		// Add entries to wad archive
		for (size_t a = 0; a < entries.size(); a++) {
			// Add each entry to the wad archive
			wad.addExistingEntry(entries[a], entries.size(), true);
		}

		return wad.save(filename);
	}

	static bool isWadArchive(MemChunk& mc);
	static bool isWadArchive(string filename);
};

#endif	/* _WADARCHIVE_H */
