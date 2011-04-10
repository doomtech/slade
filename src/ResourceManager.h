
#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "ListenerAnnouncer.h"
#include "Archive.h"
#include <wx/hashmap.h>

class ResourceManager;

class Resource {
friend class ResourceManager;
private:
	vector<ArchiveEntry*>	entries;

public:
	Resource(ArchiveEntry* entry = NULL);
	~Resource();

	void	add(ArchiveEntry* entry);
	void	remove(ArchiveEntry* entry);

	int		length();
};

// Declare hash map class to hold resources
WX_DECLARE_STRING_HASH_MAP(Resource, ResourceMap);

class ResourceManager : public Listener, public Announcer {
private:
	ResourceMap		patches;
	ResourceMap		graphics;

	static ResourceManager*	instance;

public:
	ResourceManager();
	~ResourceManager();

	static ResourceManager*	getInstance() {
		if (!instance)
			instance = new ResourceManager();

		return instance;
	}

	void	addArchive(Archive* archive);
	void	removeArchive(Archive* archive);

	void	addEntry(ArchiveEntry* entry);
	void	removeEntry(ArchiveEntry* entry);

	void	listAllPatches();
	void	getAllPatchEntries(vector<ArchiveEntry*>& list, Archive* priority);

	ArchiveEntry*	getPatchEntry(string patch, string nspace = "patches", Archive* priority = NULL);

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

// Define for less cumbersome ResourceManager::getInstance()
#define theResourceManager ResourceManager::getInstance()

#endif//__RESOURCE_MANAGER_H__
