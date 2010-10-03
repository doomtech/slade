
#include "Main.h"
#include "MainWindow.h"
#include "WxStuff.h"
#include "PatchBrowser.h"
#include "ArchiveManager.h"
#include "Misc.h"


PatchBrowserItem::PatchBrowserItem(string name, ArchiveEntry* entry, unsigned index) : BrowserItem(name, index) {
	// Init variables
	this->entry = entry;
	this->palette = NULL;
}

PatchBrowserItem::~PatchBrowserItem() {
}

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




PatchBrowser::PatchBrowser(wxWindow* parent) : BrowserWindow(parent) {
	// Init variables
	this->patch_table = NULL;

	// Init browser tree
	items_root->addChild("IWAD");
	items_root->addChild("Custom");
	items_root->addChild("Unknown");

	// Init palette chooser
	listenTo(thePaletteAnnouncer);

	// Set dialog title
	SetTitle("Patch Browser");
}

PatchBrowser::~PatchBrowser() {
}

bool PatchBrowser::openPatchTable(PatchTable* table) {
	// Check table was given
	if (!table)
		return false;

	// Clear any existing browser items
	clearItems();

	// Setup palette chooser
	thePaletteChooser->setGlobalFromArchive(table->getParent());

	// Go through patch table
	for (unsigned a = 0; a < table->nPatches(); a++) {
		// Get patch
		patch_t& patch = table->patch(a);

		// Init position to add
		string where = "Unknown";

		// Check it's parent archive
		table->updatePatchEntry(a);
		if (patch.entry) {
			Archive* parent_archive = patch.entry->getParent();

			// If it's the base resource archive, put it under 'IWAD', otherwise 'Custom'
			if (parent_archive == theArchiveManager->baseResourceArchive())
				where = "IWAD";
			else
				where = "Custom";
		}

		// Add it
		PatchBrowserItem* item = new PatchBrowserItem(patch.name, patch.entry, a);
		item->setPalette(thePaletteChooser->getSelectedPalette());
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

int PatchBrowser::getSelectedPatch() {
	// Get selected item
	PatchBrowserItem* item = (PatchBrowserItem*)getSelectedItem();

	if (item)
		return item->getIndex();
	else
		return -1;
}

void PatchBrowser::updateItemPalettes(BrowserTreeNode* node) {
	// Root node if none given
	if (!node)
		node = items_root;

	// Go through items and update their palettes
	for (unsigned a = 0; a < node->nItems(); a++)
		((PatchBrowserItem*)node->getItem(a))->setPalette(thePaletteChooser->getSelectedPalette());

	// Go through child nodes and update their items
	for (unsigned a = 0; a < node->nChildren(); a++)
		updateItemPalettes((BrowserTreeNode*)node->getChild(a));
}

/* PatchBrowser::onAnnouncement
 * Handles any announcements
 *******************************************************************/
void PatchBrowser::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	if (announcer != thePaletteAnnouncer)
		return;

	if (event_name == "main_palette_changed") {
		// Update all item palettes and reload them
		updateItemPalettes();
		reloadItems();
		Refresh();
	}
}
