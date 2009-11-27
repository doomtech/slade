
#ifndef __CLIPBOARD_H__
#define	__CLIPBOARD_H__

enum {
	CLIPBOARD_ENTRY,
	CLIPBOARD_ZIPDIR,
	CLIPBOARD_COMPOSITE_TEXTURE,
	CLIPBOARD_PATCH,
};

struct cb_item_t {
	int			type;
	MemChunk	data;
};

class Clipboard {
private:
	vector<cb_item_t*>	items;
	
	static Clipboard*	instance;

public:
	Clipboard();
	~Clipboard();

	// Singleton implementation
	static Clipboard*	getInstance() {
		if (!instance)
			instance = new Clipboard();

		return instance;
	}

	uint32_t	nItems() { return items.size(); }
	cb_item_t*	getItem(uint32_t index);
	bool		addItem(int type, MemChunk& data);

	void	clear();
};

#endif//__CLIPBOARD_H__

// Define for less cumbersome ClipBoard::getInstance()
#define theClipboard Clipboard::getInstance()
