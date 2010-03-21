#ifndef ANIMATEDLIST_H
#define	ANIMATEDLIST_H

#include "ArchiveEntry.h"
#include "BinaryControlLump.h"

class AnimatedEntry
{
private:
	uint8_t				type;
	string				first;
	string				last;
	uint32_t			speed;
	bool				decals;

public:
	AnimatedEntry(animated_t entry);
	~AnimatedEntry();

	string		getFirst()	{ return first; }
	string		getLast()	{ return last; }
	uint8_t		getType()	{ return type; }
	uint32_t	getSpeed()	{ return speed; }
	bool		getDecals()	{ return decals; }
};

class AnimatedList {
private:
	vector<AnimatedEntry*>		entries;

public:
	AnimatedList();
	~AnimatedList();

	uint32_t		nEntries() { return entries.size(); }
	AnimatedEntry*	getEntry(size_t index);
	AnimatedEntry*	getEntry(string name);
	void			clear();
	bool			readANIMATEDData(ArchiveEntry* animated);
	bool			writeANIMATEDData(ArchiveEntry* animated);
};

#endif //ANIMATEDLIST_H
