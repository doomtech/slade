
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PatchBrowser.cpp
 * Description: A specialisation of the Browser class for browsing
 *              the contents of a patch table. Splits the patches
 *              into three categories - Base, Archive and Unknown
 *              for patches existing in the base resource, the
 *              current archive, and entries not found, respectively
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
#include "MainWindow.h"
#include "WxStuff.h"
#include "PatchBrowser.h"
#include "ArchiveManager.h"
#include "Misc.h"


/*******************************************************************
 * PATCHBROWSERITEM CLASS FUNCTIONS
 *******************************************************************/

/* PatchBrowserItem::PatchBrowserItem
 * PatchBrowserItem class constructor
 *******************************************************************/
PatchBrowserItem::PatchBrowserItem(string name, ArchiveEntry* entry, unsigned index) : BrowserItem(name, index) {
	// Init variables
	this->entry = entry;
	this->palette = NULL;
}

/* PatchBrowserItem::~PatchBrowserItem
 * PatchBrowserItem class destructor
 *******************************************************************/
PatchBrowserItem::~PatchBrowserItem() {
}

/* PatchBrowserItem::loadImage
 * Loads the item's image from its associated entry (if any)
 *******************************************************************/
bool PatchBrowserItem::loadImage() {
	// Do nothing if no patch entry defined
	if (!entry)
		return false;

	// Load image from entry if it exists
	SImage* temp_image = new SImage();
	Misc::loadImageFromEntry(temp_image, entry);

	// Load texture from image
	bool ok = image.loadImage(temp_image, palette);

	// Clean up
	delete temp_image;

	return ok;
}


/*******************************************************************
 * PATCHBROWSER CLASS FUNCTIONS
 *******************************************************************/

/* PatchBrowser::PatchBrowser
 * PatchBrowser class constructor
 *******************************************************************/
PatchBrowser::PatchBrowser(wxWindow* parent) : BrowserWindow(parent) {
	// Init variables
	this->patch_table = NULL;

	// Init browser tree
	items_root->addChild("IWAD");
	items_root->addChild("Custom");
	items_root->addChild("Unknown");

	// Init palette chooser
	listenTo(theMainWindow->getPaletteChooser());

	// Set dialog title
	SetTitle("Patch Browser");
}

/* PatchBrowser::~PatchBrowser
 * PatchBrowser class destructor
 *******************************************************************/
PatchBrowser::~PatchBrowser() {
}

/* PatchBrowser::openPatchTable
 * Opens contents of the patch table [table] for browsing
 *******************************************************************/
bool PatchBrowser::openPatchTable(PatchTable* table) {
	// Check table was given
	if (!table)
		return false;

	// Clear any existing browser items
	clearItems();

	// Setup palette chooser
	theMainWindow->getPaletteChooser()->setGlobalFromArchive(table->getParent());

	// Go through patch table
	for (unsigned a = 0; a < table->nPatches(); a++) {
		// Get patch
		patch_t& patch = table->patch(a);

		// Init position to add
		string where = "Unknown";

		// Check its parent archive
		//table->updatePatchEntry(a);
		if (patch.entry) {
			Archive* parent_archive = patch.entry->getParent();

			// If it's the base resource archive, put it under 'IWAD', otherwise 'Custom'
			if (parent_archive == theArchiveManager->baseResourceArchive())
				where = "Base";
			else
				where = "Archive";
		}

		// Add it
		PatchBrowserItem* item = new PatchBrowserItem(patch.name, patch.entry, a);
		item->setPalette(theMainWindow->getPaletteChooser()->getSelectedPalette());
		addItem(item, where);
	}

	// Update variables
	if (patch_table) stopListening(patch_table);
	patch_table = table;
	listenTo(table);

	// Open 'all' node
	openTree(items_root);

	// Update tree control
	populateItemTree();

	return true;
}

/* PatchBrowser::getSelectedPatch
 * Returns the index of the currently selected patch, or -1 if none
 * are selected
 *******************************************************************/
int PatchBrowser::getSelectedPatch() {
	// Get selected item
	PatchBrowserItem* item = (PatchBrowserItem*)getSelectedItem();

	if (item)
		return item->getIndex();
	else
		return -1;
}

/* PatchBrowser::updateItems
 * Updates the palette and entry for each browser item
 *******************************************************************/
void PatchBrowser::updateItems(BrowserTreeNode* node) {
	// Root node if none given
	if (!node)
		node = items_root;

	// Go through items
	for (unsigned a = 0; a < node->nItems(); a++) {
		PatchBrowserItem* item = (PatchBrowserItem*)node->getItem(a);

		// Update palette
		item->setPalette(theMainWindow->getPaletteChooser()->getSelectedPalette());

		// Update image entry
		item->setEntry(patch_table->patch(item->getIndex()).entry);
	}

	// Go through child nodes and update their items
	for (unsigned a = 0; a < node->nChildren(); a++)
		updateItems((BrowserTreeNode*)node->getChild(a));
}

/* PatchBrowser::onAnnouncement
 * Handles any announcements
 *******************************************************************/
void PatchBrowser::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	if (announcer != theMainWindow->getPaletteChooser())
		return;

	if (event_name == "main_palette_changed") {
		// Update all item palettes and reload them
		updateItems();
		reloadItems();
		Refresh();
	}
}
