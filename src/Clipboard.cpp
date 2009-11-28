
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
cb_item_t* Clipboard::getItem(uint32_t index) {
	if (index >= items.size())
		return NULL;
	else
		return items[index];
}

/* Clipboard::addItem
 * Adds an item to the clipboard with the given type and data
 *******************************************************************/
bool Clipboard::addItem(int type, MemChunk& data) {
	cb_item_t* new_item = new cb_item_t;

	new_item->type = type;
	new_item->data.loadMem(data.getData(), data.getSize());

	items.push_back(new_item);
}

/* Clipboard::clear
 * Clears all clipboard items
 *******************************************************************/
void Clipboard::clear() {
	for (uint32_t a = 0; a < items.size(); a++) {
		if (items[a]->type == CLIPBOARD_ZIPDIR) {
			zipdir_t* dir = (zipdir_t*)items[a]->data.getData();
			dir->clear(true);
		}
		delete items[a];
	}

	items.clear();
}
