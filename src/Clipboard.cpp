
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
#include "ZipArchive.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
Clipboard* Clipboard::instance = NULL;


/*******************************************************************
 * CLIPBOARDITEM CLASS FUNCTIONS
 *******************************************************************/

ClipboardItem::ClipboardItem(int type) {
	this->type = type;
}

ClipboardItem::~ClipboardItem() {
}


/*******************************************************************
 * ENTRYCLIPBOARDITEM CLASS FUNCTIONS
 *******************************************************************/

EntryClipboardItem::EntryClipboardItem(ArchiveEntry* entry)
: ClipboardItem(CLIPBOARD_ENTRY) {
	if (entry)
		this->entry = new ArchiveEntry(*entry);
	else
		this->entry = NULL;
}

EntryClipboardItem::~EntryClipboardItem() {
	if (entry)
		delete entry;
}


/*******************************************************************
 * ZIPDIRCLIPBOARDITEM CLASS FUNCTIONS
 *******************************************************************/

ZipDirClipboardItem::ZipDirClipboardItem()
: ClipboardItem(CLIPBOARD_ZIPDIR) {
}

ZipDirClipboardItem::~ZipDirClipboardItem() {
	for (size_t a = 0; a < entries.size(); a++)
		delete entries[a];
}

bool ZipDirClipboardItem::addEntry(ArchiveEntry* entry) {
	if (!entry)
		return false;

	entries.push_back(new ArchiveEntry(*entry));
	return true;
}

ArchiveEntry* ZipDirClipboardItem::getEntry(uint32_t index) {
	if (index >= entries.size())
		return NULL;
	else
		return entries[index];
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
	if (!item)
		return false;

	items.push_back(item);
	return true;
}
