
#include "Main.h"
#include "MapTextureManager.h"
#include "ResourceManager.h"
#include "CTexture.h"
#include "MainWindow.h"
#include "ArchiveManager.h"

MapTextureManager::MapTextureManager(Archive* archive) {
	// Init variables
	this->archive = archive;
	thing_images_loaded = false;

	// Listen to the resource manager
	listenTo(theResourceManager);
}

MapTextureManager::~MapTextureManager() {
}

GLTexture* MapTextureManager::getTexture(string name) {
	// Get texture matching name
	map_tex_t& mtex = textures[name.Upper()];

	// Return it if found
	if (mtex.texture)
		return mtex.texture;

	// Texture not found, look for it
	Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();

	// Try composite textures first
	CTexture* ctex = theResourceManager->getTexture(name, archive);
	if (ctex) {
		SImage image;
		ctex->toImage(image, archive, pal);
		mtex.texture = new GLTexture(false);
		mtex.texture->loadImage(&image, pal);
		return mtex.texture;
	}

	// TODO: TX_ textures, mixed flats+textures

	return NULL;
}

GLTexture* MapTextureManager::getFlat(string name) {
	// Get flat matching name
	map_tex_t& mtex = flats[name.Upper()];

	// Return it if found
	if (mtex.texture)
		return mtex.texture;

	// Flat not found, look for it
	Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();
	ArchiveEntry* entry = theResourceManager->getFlatEntry(name, archive);
	if (entry) {
		SImage image;
		Misc::loadImageFromEntry(&image, entry);
		mtex.texture = new GLTexture(false);
		mtex.texture->loadImage(&image, pal);
		return mtex.texture;
	}

	// TODO: mixed flats+textures

	return NULL;
}

GLTexture* MapTextureManager::getSprite(string name) {
	// Get sprite matching name
	map_tex_t& mtex = sprites[name.Upper()];

	// Return it if found
	if (mtex.texture)
		return mtex.texture;

	// Sprite not found, look for it
	Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();
	ArchiveEntry* entry = theResourceManager->getPatchEntry(name, "sprites", archive);
	if (!entry) entry = theResourceManager->getPatchEntry(name, "", archive);
	if (entry) {
		SImage image;
		Misc::loadImageFromEntry(&image, entry);
		mtex.texture = new GLTexture(false);
		mtex.texture->setFilter(GLTexture::NEAREST_LINEAR_MIN);
		mtex.texture->loadImage(&image, pal);
		return mtex.texture;
	}

	return NULL;
}

GLTexture* MapTextureManager::getThingImage(string name) {
	// Load thing image textures if they haven't already
	if (!thing_images_loaded) {
		// Load all thing images to textures
		Archive* slade_pk3 = theArchiveManager->programResourceArchive();
		ArchiveTreeNode* dir = slade_pk3->getDir("images/thing");
		if (dir) {
			SImage image;
			for (unsigned a = 0; a < dir->numEntries(); a++) {
				ArchiveEntry* entry = dir->getEntry(a);

				// Load entry to image
				if (image.open(entry->getMCData())) {
					// Create texture in hashmap
					map_tex_t& mtex = thing_images[entry->getName(true)];
					mtex.texture = new GLTexture(false);
					mtex.texture->setFilter(GLTexture::MIPMAP);
					mtex.texture->loadImage(&image);
				}
			}
		}

		thing_images_loaded = true;
	}

	return thing_images[name].texture;
}

void MapTextureManager::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Only interested in the resource manager
	if (announcer != theResourceManager)
		return;

	// If the resources have been updated
	if (event_name == "resources_updated") {
		// Just clear all cached textures
		textures.clear();
		flats.clear();
		sprites.clear();
		wxLogMessage("texture manager cleared");
	}
}
