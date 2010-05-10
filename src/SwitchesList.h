#ifndef SWITCHESLIST_H
#define	SWITCHESLIST_H

#include "ArchiveEntry.h"
#include "BinaryControlLump.h"

class SwitchesEntry
{
private:
	uint8_t				type;
	string				off;
	string				on;

public:
	SwitchesEntry(switches_t entry);
	~SwitchesEntry();

	string		getOff()	{ return off; }
	string		getOn()		{ return on; }
	uint8_t		getType()	{ return type; }
};

class SwitchesList {
private:
	vector<SwitchesEntry*>		entries;

public:
	SwitchesList();
	~SwitchesList();

	uint32_t		nEntries() { return entries.size(); }
	SwitchesEntry*	getEntry(size_t index);
	SwitchesEntry*	getEntry(string name);
	void			clear();
	bool			readSWITCHESData(ArchiveEntry* switches);
	bool			writeSWITCHESData(ArchiveEntry* switches);

	// Static functions
	static bool convertSwitches(ArchiveEntry* entry, MemChunk* animdata);
};

#endif //SWITCHESLIST_H
