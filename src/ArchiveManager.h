
#ifndef __ARCHIVEMANAGER_H__
#define __ARCHIVEMANAGER_H__

#include "ListenerAnnouncer.h"
#include "Archive.h"

class ArchiveManager : public Announcer, Listener {
private:
	struct archive_t {
		Archive*	archive;
		bool		resource;
	};

	vector<archive_t>	open_archives;
	Archive*			program_resource_archive;
	Archive*			base_resource_archive;
	bool				res_archive_open;

	static ArchiveManager*	instance;

public:
	ArchiveManager();
	~ArchiveManager();

	static ArchiveManager*	getInstance() {
		if (!instance)
			instance = new ArchiveManager();

		return instance;
	}

	bool		resArchiveOK() { return res_archive_open; }
	bool		addArchive(Archive* archive);
	Archive*	getArchive(int index);
	Archive*	getArchive(string filename);
	Archive*	openArchive(string filename);
	Archive*	newArchive(uint8_t type);
	bool		closeArchive(int index);
	bool		closeArchive(string filename);
	bool		closeArchive(Archive* archive);
	void		closeAll();
	int			numArchives() { return (int)open_archives.size(); }
	int			archiveIndex(Archive* archive);
	Archive*	programResourceArchive() { return program_resource_archive; }
	Archive*	baseResourceArchive() { return base_resource_archive; }
	bool		openBaseResource(string filename);

	ArchiveEntry*	getResourceEntry(string name);

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

// Define for less cumbersome ArchiveManager::getInstance()
#define theArchiveManager ArchiveManager::getInstance()

#endif //__ARCHIVEMANAGER_H__
