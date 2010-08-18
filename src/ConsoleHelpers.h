#ifndef __CONSOLEHELPERS_H__
#define	__CONSOLEHELPERS_H__

// No #includes, they're a headache
class Archive;
class ArchiveEntry;
class ArchivePanel;
class GfxEntryPanel;

// Console helpers
namespace CH {
	Archive*				getCurrentArchive();
	ArchiveEntry*			getCurrentArchiveEntry();
	vector<ArchiveEntry*>	getCurrentArchiveEntries();
	ArchivePanel*			getCurrentArchivePanel();
	GfxEntryPanel*			getCurrentGfxPanel();
}

#endif // __CONSOLEHELPERS_H__