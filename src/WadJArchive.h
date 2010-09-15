
#ifndef __WADJARCHIVE_H__
#define	__WADJARCHIVE_H__

#include "Archive.h"
#include "WadArchive.h"

class WadJArchive : public WadArchive {
private:
	vector<ArchiveEntry*>	entries;
	char					wad_type[4];
	ArchiveEntry*			patches[2];
	ArchiveEntry*			sprites[2];
	ArchiveEntry*			flats[2];
	ArchiveEntry*			tx[2];

public:
	WadJArchive();
	~WadJArchive();

	// Wad specific
	bool	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	open(MemChunk& mc);

	bool	write(MemChunk& mc, bool update = true);
	bool	write(string filename, bool update = true);

	static bool isWadJArchive(MemChunk& mc);
	static bool isWadJArchive(string filename);
};

#endif	/* __WADJARCHIVE_H__ */
