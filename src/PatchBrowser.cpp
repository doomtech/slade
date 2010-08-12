
#include "Main.h"
#include "PatchBrowser.h"
#include "ArchiveManager.h"
#include "Misc.h"


PatchBrowserItem::PatchBrowserItem(string name, ArchiveEntry* entry) : BrowserItem(name) {
	// Init variables
	this->entry = entry;
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
	bool ok = image.loadImage(temp_image);	// TODO: Palette, how?

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
}

PatchBrowser::~PatchBrowser() {
}

bool PatchBrowser::openPatchTable(PatchTable* table) {
	// Check table was given
	if (!table)
		return false;

	// Clear any existing browser items
	clearItems();

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
		PatchBrowserItem* item = new PatchBrowserItem(patch.name, patch.entry);
		addItem(item, where);
	}

	// Update variables
	if (patch_table) stopListening(patch_table);
	patch_table = table;
	listenTo(table);

	return true;
}
