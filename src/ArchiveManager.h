
#ifndef __ARCHIVEMANAGER_H__
#define __ARCHIVEMANAGER_H__

#include "ListenerAnnouncer.h"

class Archive;

class ArchiveManager : public Announcer, Listener {
private:
	vector<Archive*>	open_archives;
	Archive*			resource_archive;

public:
	ArchiveManager();
	~ArchiveManager();

	static ArchiveManager&	getInstance() {
		static ArchiveManager instance;
		return instance;
	}

	bool		addArchive(Archive* archive);
	Archive*	getArchive(int index);
	Archive*	getArchive(string filename);
	Archive*	openArchive(string filename);
	Archive*	newArchive(BYTE type);
	bool		closeArchive(int index);
	bool		closeArchive(string filename);
	bool		closeArchive(Archive* archive);
	int			numArchives() { return (int)open_archives.size(); }
	int			archiveIndex(Archive* archive);
	Archive*	resourceArchive() { return resource_archive; }

	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

#endif //__ARCHIVEMANAGER_H__
