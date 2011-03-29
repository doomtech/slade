
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ResourceManager.cpp
 * Description: The ResourceManager class. Manages all editing
 *              resources (patches, gfx, etc) in all open archives
 *              and the base resource.
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
#include "ResourceManager.h"
#include "ArchiveManager.h"
#include "Console.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
ResourceManager* ResourceManager::instance = NULL;


/*******************************************************************
 * RESOURCE CLASS FUNCTIONS
 *******************************************************************/

/* Resource::Resource
 * Resource class constructor
 *******************************************************************/
Resource::Resource(ArchiveEntry* entry) {
}

/* Resource::~Resource
 * Resource class destructor
 *******************************************************************/
Resource::~Resource() {
}

/* Resource::add
 * Adds matching [entry] to the resource
 *******************************************************************/
void Resource::add(ArchiveEntry* entry) {
	entries.push_back(entry);
}

/* Resource::remove
 * Removes matching [entry] from the resource
 *******************************************************************/
void Resource::remove(ArchiveEntry* entry) {
	unsigned a = 0;
	while (a < entries.size()) {
		if (entries[a] == entry)
			entries.erase(entries.begin() + a);
		else
			a++;
	}
}

/* Resource::length
 * Returns the number of entries matching this resource
 *******************************************************************/
int Resource::length() {
	return entries.size();
}


/*******************************************************************
 * RESOURCEMANAGER CLASS FUNCTIONS
 *******************************************************************/

/* ResourceManager::ResourceManager
 * ResourceManager class constructor
 *******************************************************************/
ResourceManager::ResourceManager() {
}

/* ResourceManager::~ResourceManager
 * ResourceManager class destructor
 *******************************************************************/
ResourceManager::~ResourceManager() {
}

/* ResourceManager::addArchive
 * Adds an archive to be managed
 *******************************************************************/
void ResourceManager::addArchive(Archive* archive) {
	// Check archive was given
	if (!archive)
		return;

	// Go through entries
	vector<ArchiveEntry*> entries;
	archive->getEntryTreeAsList(entries);
	for (unsigned a = 0; a < entries.size(); a++)
		addEntry(entries[a]);

	// Listen to the archive
	listenTo(archive);
}

/* ResourceManager::removeArchive
 * Removes a managed archive
 *******************************************************************/
void ResourceManager::removeArchive(Archive* archive) {
	// Check archive was given
	if (!archive)
		return;

	// Go through entries
	vector<ArchiveEntry*> entries;
	archive->getEntryTreeAsList(entries);
	for (unsigned a = 0; a < entries.size(); a++)
		removeEntry(entries[a]);
}

/* ResourceManager::addEntry
 * Adds an entry to be managed
 *******************************************************************/
void ResourceManager::addEntry(ArchiveEntry* entry) {
	// Detect type if unknown
	if (entry->getType() == EntryType::unknownType())
		EntryType::detectEntryType(entry);

	// Get entry type
	EntryType* type = entry->getType();

	// Get resource name (extension cut, uppercase)
	string name = entry->getName(true).Upper();

	// Check for patch entry
	if (type->extraProps().propertyExists("patch"))
		patches[name].add(entry);
}

/* ResourceManager::removeEntry
 * Removes a managed entry
 *******************************************************************/
void ResourceManager::removeEntry(ArchiveEntry* entry) {
	// Get resource name (extension cut, uppercase)
	string name = entry->getName(true).Upper();

	// Remove from patches
	patches[name].remove(entry);
}

/* ResourceManager::listAllPatches
 * Dumps all patch names and the number of matching entries for each
 *******************************************************************/
void ResourceManager::listAllPatches() {
	ResourceMap::iterator i = patches.begin();

	// Add all properties to given list
	while (i != patches.end()) {
		wxLogMessage("%s (%d)", CHR(i->first), i->second.length());
		i++;
	}
}

/* ResourceManager::getPatchEntry
 * Returns the most appropriate managed resource entry for [patch],
 * or NULL if no match found
 *******************************************************************/
ArchiveEntry* ResourceManager::getPatchEntry(string patch, string nspace, Archive* priority) {
	// Check resource with matching name exists
	Resource& res = patches[patch];
	if (res.entries.size() == 0)
		return NULL;

	// Go through resource entries
	ArchiveEntry* entry = res.entries[0];
	for (unsigned a = 0; a < res.entries.size(); a++) {
		// If the entry is in the correct namespace (if namespace is important)
		if (nspace.IsEmpty() || res.entries[a]->isInNamespace(nspace)) {
			// If it's in the 'priority' archive, return it
			if (priority && res.entries[a]->getParent() == priority)
				return entry;

			// Otherwise, if it's in a 'later' archive than the current resource entry, set it
			if (theArchiveManager->archiveIndex(entry->getParent()) <=
				theArchiveManager->archiveIndex(res.entries[a]->getParent()))
				entry = res.entries[a];
		}
	}

	// Return most relevant entry
	return entry;
}

/* ResourceManager::onAnnouncement
 * Called when an announcement is recieved from any managed archive
 *******************************************************************/
void ResourceManager::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	event_data.seek(0, SEEK_SET);

	// An entry is modified
	if (event_name == "entry_state_changed") {
		uint32_t index;
		wxUIntPtr ptr;
		event_data.read(&index, sizeof(uint32_t));
		event_data.read(&ptr, sizeof(wxUIntPtr));
		ArchiveEntry* entry = (ArchiveEntry*)wxUIntToPtr(ptr);
		removeEntry(entry);
		addEntry(entry);
	}

	// An entry is removed or renamed
	if (event_name == "entry_removing" || event_name == "entry_renaming") {
		int index;
		wxUIntPtr ptr;
		event_data.read(&index, sizeof(int));
		event_data.read(&ptr, sizeof(wxUIntPtr));
		ArchiveEntry* entry = (ArchiveEntry*)wxUIntToPtr(ptr);
		removeEntry(entry);
	}
}





CONSOLE_COMMAND(list_res_patches, 0) {
	theResourceManager->listAllPatches();
}
