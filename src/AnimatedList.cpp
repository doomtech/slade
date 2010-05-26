/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    BinaryControlLump.cpp
 * Description: Various classes used to represent data objects from
 *				Boom's ANIMATED lumps.
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
#include "Misc.h"
#include "AnimatedList.h"

/*******************************************************************
 * ANIMATEDENTRY CLASS FUNCTIONS
 *******************************************************************/

/* AnimatedEntry::AnimatedEntry
 * AnimatedEntry class constructor
 *******************************************************************/
AnimatedEntry::AnimatedEntry(animated_t entry) {
	// Init variables
	this->first = wxString::From8BitData(entry.first, 8);
	this->last = wxString::From8BitData(entry.last, 8);
	this->type = entry.type & ANIM_MASK;
	this->speed = wxINT32_SWAP_ON_BE(entry.speed);
	this->decals = !!(entry.type & ANIM_DECALS);
}

/* AnimatedEntry::~AnimatedEntry
 * AnimatedEntry class destructor
 *******************************************************************/
AnimatedEntry::~AnimatedEntry() {
}

/*******************************************************************
 * ANIMATEDLIST CLASS FUNCTIONS
 *******************************************************************/

/* AnimatedList::AnimatedList
 * AnimatedList class constructor
 *******************************************************************/
AnimatedList::AnimatedList() {
}

/* AnimatedList::~AnimatedList
 * AnimatedList class destructor
 *******************************************************************/
AnimatedList::~AnimatedList() {
	clear();
}

/* AnimatedList::getEntry
 * Returns the AnimatedEntry at [index], or NULL if [index] is out of
 * range
 *******************************************************************/
AnimatedEntry* AnimatedList::getEntry(size_t index) {
	// Check index range
	if (index > nEntries())
		return NULL;

	return entries[index];
}

/* AnimatedList::getEntry
 * Returns an AnimatedEntry matching [name], or NULL if no match
 * found; looks for name at both the first and the last frames
 *******************************************************************/
AnimatedEntry* AnimatedList::getEntry(string name) {
	for (size_t a = 0; a < nEntries(); a++) {
		if (entries[a]->getFirst().CmpNoCase(name) == 0 ||
			entries[a]->getLast().CmpNoCase(name) == 0)
			return entries[a];
	}

	// No match found
	return NULL;
}

/* AnimatedList::clear
 * Clears all entries
 *******************************************************************/
void AnimatedList::clear() {
	for (size_t a = 0; a < entries.size(); a++)
		delete entries[a];
	entries.clear();

}

/* AnimatedList::readANIMATEDData
 * Reads in a Boom-format ANIMATED entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool AnimatedList::readANIMATEDData(ArchiveEntry* animated) {
	// Check entries were actually given
	if (!animated || animated->getSize() == 0)
		return false;

	uint8_t * data = new uint8_t[animated->getSize()];
	memcpy(data, animated->getData(), animated->getSize());
	uint8_t * cursor = data;
	uint8_t * eodata = cursor + animated->getSize();
	animated_t * type;

	while (cursor < eodata && *cursor != ANIM_STOP) {
		// reads an entry
		if (cursor + sizeof(animated_t) > eodata) {
			wxLogMessage("Error: ANIMATED entry is corrupt");
			delete[] data;
			return false;
		}
		type = (animated_t *) cursor;
		AnimatedEntry * entry = new AnimatedEntry(*type);
		cursor += sizeof(animated_t);

		// Add texture to list
		entries.push_back(entry);
	}
	delete[] data;
	return true;
}

/* AnimatedList::writeANIMATEDData
 * Write in a Boom-format ANIMATED entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool AnimatedList::writeANIMATEDData(ArchiveEntry* animated) {
	// Check entries were actually given
	if (!animated)
		return false;

	uint8_t * data = new uint8_t[animated->getSize()];
	memcpy(data, animated->getData(), animated->getSize());
	uint8_t * cursor = data;
	uint8_t * eodata = cursor + animated->getSize();
	animated_t * type;

	while (cursor < eodata && *cursor != ANIM_STOP) {
		// reads an entry
		if (cursor + sizeof(animated_t) > eodata) {
			wxLogMessage("Error: ANIMATED entry is corrupt");
			delete[] data;
			return false;
		}
		type = (animated_t *) cursor;
		AnimatedEntry * entry = new AnimatedEntry(*type);
		cursor += sizeof(animated_t);

		// Add texture to list
		entries.push_back(entry);
	}
	delete[] data;
	return true;
}

bool AnimatedList::convertAnimated(ArchiveEntry* entry, MemChunk * animdata) {
	const uint8_t * cursor = entry->getData(true);
	const uint8_t * eodata = cursor + entry->getSize();
	const animated_t * animation;
	string conversion;

	while (cursor < eodata && *cursor != ANIM_STOP) {
		// reads an entry
		if (cursor + sizeof(animated_t) > eodata) {
			wxLogMessage("Error: ANIMATED entry is corrupt");
			return false;
		}
		animation = (animated_t *) cursor;
		cursor += sizeof(animated_t);

		// Create animation string
		conversion = s_fmt("%s\tOptional\t%-8s\tRange\t%-8s\tTics %i%s",
			(animation->type ? "Texture" : "Flat"),
			animation->first, animation->last, animation->speed,
			(animation->type == ANIM_DECALS ? " AllowDecals\n" : "\n"));

		// Write string to animdata
		animdata->reSize(animdata->getSize() + conversion.length(), true);
		animdata->write(conversion.To8BitData().data(), conversion.length());
	}
	return true;
}
