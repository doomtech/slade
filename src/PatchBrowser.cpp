
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
#include "ResourceManager.h"
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
	// Do nothing if no patch entry found
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

	// Init browser tree
	items_root->addChild("Base");
	items_root->addChild("Archive");
	items_root->addChild("Unknown");

	// Setup palette chooser
	theMainWindow->getPaletteChooser()->setGlobalFromArchive(table->getParent());

	// Go through patch table
	for (unsigned a = 0; a < table->nPatches(); a++) {
		// Get patch
		patch_t& patch = table->patch(a);

		// Init position to add
		string whereis = "Unknown";

		// Get patch entry
		ArchiveEntry* entry = theResourceManager->getPatchEntry(patch.name);

		// Check its parent archive
		if (entry) {
			Archive* parent_archive = entry->getParent();

			// If it's the base resource archive, put it under 'IWAD', otherwise 'Custom'
			if (parent_archive == theArchiveManager->baseResourceArchive())
				whereis = "Base";
			else
				whereis = "Archive";
		}

		// Add it
		PatchBrowserItem* item = new PatchBrowserItem(patch.name, entry, a);
		item->setPalette(theMainWindow->getPaletteChooser()->getSelectedPalette());
		addItem(item, whereis);
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

bool PatchBrowser::openArchive(Archive* archive) {
	// Check archive was given
	if (!archive)
		return false;

	// Clear any existing browser items
	clearItems();

	// Init browser tree
	items_root->addChild("Patches");
	items_root->addChild("Graphics");
	items_root->addChild("Textures");
	items_root->addChild("Sprites");

	// Setup palette chooser
	theMainWindow->getPaletteChooser()->setGlobalFromArchive(archive);

	// Get a list of all available patch entries
	vector<ArchiveEntry*> patches;
	theResourceManager->getAllPatchEntries(patches, archive);

	// Go through the list
	for (unsigned a = 0; a < patches.size(); a++) {
		ArchiveEntry* entry = patches[a];

		// Skip any without parent archives (shouldn't happen)
		if (!entry->getParent())
			continue;

		// Check entry namespace
		string ns = entry->getParent()->detectNamespace(entry);
		if (ns == "patches") ns = "Patches";
		else if (ns == "sprites") ns = "Sprites";
		else if (ns == "textures") ns = "Textures";
		else ns = "Graphics";

		// Check entry parent archive
		string arch = "Unknown";
		if (entry->getParent() == theArchiveManager->baseResourceArchive())
			arch = "Base";
		else if (entry->getParent() == archive)
			arch = "Archive";
		else
			arch = "Other";

		// Add it
		PatchBrowserItem* item = new PatchBrowserItem(entry->getName().Truncate(8).Upper(), entry, a);
		item->setPalette(theMainWindow->getPaletteChooser()->getSelectedPalette());
		addItem(item, ns + "/" + arch);
	}

	// Open 'patches' node
	openTree((BrowserTreeNode*)items_root->getChild("Patches"));

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
		item->setEntry(theResourceManager->getPatchEntry(item->getName()));
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
