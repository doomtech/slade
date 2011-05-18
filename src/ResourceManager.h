
#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "ListenerAnnouncer.h"
#include "Archive.h"
#include <wx/hashmap.h>

class ResourceManager;
class CTexture;

// This base class is probably not really needed
class Resource {
friend class ResourceManager;
private:
	string	type;

public:
	Resource(string type) { this->type = type; }
	~Resource() {}

	virtual int	length() { return 0; }
};

class EntryResource : public Resource {
friend class ResourceManager;
private:
	vector<ArchiveEntry*>	entries;

public:
	EntryResource(ArchiveEntry* entry = NULL);
	~EntryResource();

	void	add(ArchiveEntry* entry);
	void	remove(ArchiveEntry* entry);

	int		length();
};

class TextureResource : public Resource {
friend class ResourceManager;
public:
	struct tex_res_t {
		CTexture*	tex;
		Archive*	parent;
	};

	TextureResource();
	~TextureResource();

	void	add(CTexture* tex, Archive* parent);
	void	remove(Archive* parent);

	int		length();

private:
	vector<tex_res_t>	textures;
};

// Declare hash map class to hold resources
WX_DECLARE_STRING_HASH_MAP(EntryResource, EntryResourceMap);
WX_DECLARE_STRING_HASH_MAP(TextureResource, TextureResourceMap);

class ResourceManager : public Listener, public Announcer {
private:
	EntryResourceMap	patches;
	EntryResourceMap	graphics;
	TextureResourceMap	textures;

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

	void	getAllTextures(vector<TextureResource::tex_res_t>& list, Archive* priority, Archive* ignore = NULL);

	ArchiveEntry*	getPatchEntry(string patch, string nspace = "patches", Archive* priority = NULL);
	CTexture*		getTexture(string texture, Archive* priority = NULL, Archive* ignore = NULL);

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

// Define for less cumbersome ResourceManager::getInstance()
#define theResourceManager ResourceManager::getInstance()

#endif//__RESOURCE_MANAGER_H__
