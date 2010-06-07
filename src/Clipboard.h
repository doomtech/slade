
#ifndef __CLIPBOARD_H__
#define	__CLIPBOARD_H__

#include "Archive.h"

enum ClipboardType {
	CLIPBOARD_ENTRY_TREE,
	//CLIPBOARD_ENTRY,
	//CLIPBOARD_ZIPDIR,
	CLIPBOARD_COMPOSITE_TEXTURE,
	CLIPBOARD_PATCH,

	CLIPBOARD_UNKNOWN,
};

class ClipboardItem {
private:
	int			type;

public:
	ClipboardItem(int type = CLIPBOARD_UNKNOWN);
	~ClipboardItem();

	int	getType() { return type; }
};

class EntryTreeClipboardItem : public ClipboardItem {
private:
	ArchiveTreeNode*	tree;

public:
	EntryTreeClipboardItem(vector<ArchiveEntry*>& entries, vector<ArchiveTreeNode*>& dirs);
	~EntryTreeClipboardItem();

	ArchiveTreeNode*	getTree() { return tree; }
};

/*
class EntryClipboardItem : public ClipboardItem {
private:
	ArchiveEntry*	entry;

public:
	EntryClipboardItem(ArchiveEntry* entry);
	~EntryClipboardItem();

	ArchiveEntry*	getEntry() { return entry; }
};

class ZipDirClipboardItem : public ClipboardItem {
private:
	vector<ArchiveEntry*>	entries;

public:
	ZipDirClipboardItem();
	~ZipDirClipboardItem();

	uint32_t		nEntries() { return entries.size(); }
	bool			addEntry(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(uint32_t index);
};
*/

class Clipboard {
private:
	vector<ClipboardItem*>	items;
	static Clipboard*		instance;

public:
	Clipboard();
	~Clipboard();

	// Singleton implementation
	static Clipboard*	getInstance() {
		if (!instance)
			instance = new Clipboard();

		return instance;
	}

	uint32_t		nItems() { return items.size(); }
	ClipboardItem*	getItem(uint32_t index);
	bool			addItem(ClipboardItem* item);

	void	clear();
};

#endif//__CLIPBOARD_H__

// Define for less cumbersome ClipBoard::getInstance()
#define theClipboard Clipboard::getInstance()
