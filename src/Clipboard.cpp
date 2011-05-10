
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Clipboard.cpp
 * Description: The SLADE Clipboard implementation
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
#include "Clipboard.h"
#include "CTexture.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
Clipboard* Clipboard::instance = NULL;


/*******************************************************************
 * CLIPBOARDITEM CLASS FUNCTIONS
 *******************************************************************/

/* ClipboardItem::ClipboardItem
 * ClipboardItem class constructor
 *******************************************************************/
ClipboardItem::ClipboardItem(int type) {
	this->type = type;
}

/* ClipboardItem::~ClipboardItem
 * ClipboardItem class destructor
 *******************************************************************/
ClipboardItem::~ClipboardItem() {
}


/*******************************************************************
 * ENTRYTREECLIPBOARDITEM CLASS FUNCTIONS
 *******************************************************************/

/* EntryTreeClipboardItem::EntryTreeClipboardItem
 * EntryTreeClipboardItem class constructor
 *******************************************************************/
EntryTreeClipboardItem::EntryTreeClipboardItem(vector<ArchiveEntry*>& entries, vector<ArchiveTreeNode*>& dirs)
: ClipboardItem(CLIPBOARD_ENTRY_TREE) {
	// Create tree
	tree = new ArchiveTreeNode();

	for (unsigned a = 0; a < entries.size(); a++)
		tree->addEntry(new ArchiveEntry(*(entries[a])));

	for (unsigned a = 0; a < dirs.size(); a++)
		tree->addChild(dirs[a]->clone());
}

/* EntryTreeClipboardItem::~EntryTreeClipboardItem
 * EntryTreeClipboardItem class destructor
 *******************************************************************/
EntryTreeClipboardItem::~EntryTreeClipboardItem() {
	if (tree)
		delete tree;
}


/*******************************************************************
 * TEXTURECLIPBOARDITEM CLASS FUNCTIONS
 *******************************************************************/

/* TextureClipboardItem::TextureClipboardItem
 * TextureClipboardItem class constructor
 *******************************************************************/
TextureClipboardItem::TextureClipboardItem(CTexture* texture, Archive* parent) : ClipboardItem(CLIPBOARD_COMPOSITE_TEXTURE) {
	// Create/copy texture
	this->texture = new CTexture();
	this->texture->copyTexture(texture);

	// Copy patch entries if possible
	for (unsigned a = 0; a < texture->nPatches(); a++) {
		ArchiveEntry* entry = texture->getPatch(a)->getPatchEntry(parent);

		// Don't copy patch if it has been already
		bool there = false;
		for (unsigned b = 0; b < patch_entries.size(); b++) {
			if (patch_entries[b]->getName() == entry->getName()) {
				there = true;
				break;
			}
		}
		if (there) continue;

		// Copy patch entry
		if (entry)
			patch_entries.push_back(new ArchiveEntry(*entry));
	}
}

/* TextureClipboardItem::~TextureClipboardItem
 * TextureClipboardItem class destructor
 *******************************************************************/
TextureClipboardItem::~TextureClipboardItem() {
	// Clean up
	delete texture;
	for (unsigned a = 0; a < patch_entries.size(); a++) {
		if (patch_entries[a])
			delete patch_entries[a];
	}
}

/* TextureClipboardItem::getPatchEntry
 * Returns the entry copy for the patch at [index] in the texture
 *******************************************************************/
ArchiveEntry* TextureClipboardItem::getPatchEntry(string patch) {
	// Find copied patch entry with matching name
	for (unsigned a = 0; a < patch_entries.size(); a++) {
		if (S_CMPNOCASE(patch_entries[a]->getName(true).Truncate(8), patch))
			return patch_entries[a];
	}

	// Not found
	return NULL;
}


/*******************************************************************
 * CLIPBOARD CLASS FUNCTIONS
 *******************************************************************/

/* Clipboard::Clipboard
 * Clipboard class constructor
 *******************************************************************/
Clipboard::Clipboard() {
}

/* Clipboard::~Clipboard
 * Clipboard class destructor
 *******************************************************************/
Clipboard::~Clipboard() {
	for (uint32_t a = 0; a < items.size(); a++)
		delete items[a];
}

/* Clipboard::getItem
 * Returns the item at index or NULL if index is out of bounds
 *******************************************************************/
ClipboardItem* Clipboard::getItem(uint32_t index) {
	if (index >= items.size())
		return NULL;
	else
		return items[index];
}

/* Clipboard::clear
 * Clears all clipboard items
 *******************************************************************/
void Clipboard::clear() {
	for (uint32_t a = 0; a < items.size(); a++)
		delete items[a];

	items.clear();
}

/* Clipboard::addItem
 * Adds an item to the clipboard. Returns false if item is invalid
 *******************************************************************/
bool Clipboard::addItem(ClipboardItem* item) {
	// Clear current clipboard contents
	clear();

	if (!item)
		return false;

	items.push_back(item);
	return true;
}

/* Clipboard::addItems
 * Adds multiple items to the clipboard
 *******************************************************************/
bool Clipboard::addItems(vector<ClipboardItem*>& items) {
	// Clear current clipboard contents
	clear();

	for (unsigned a = 0; a < items.size(); a++) {
		if (items[a])
			this->items.push_back(items[a]);
	}

	return true;
}
