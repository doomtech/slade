
#include "Main.h"
#include "MapTextureManager.h"
#include "ResourceManager.h"
#include "CTexture.h"
#include "MainWindow.h"
#include "ArchiveManager.h"
#include "MapEditorWindow.h"

MapTextureManager::MapTextureManager(Archive* archive) {
	// Init variables
	this->archive = archive;
	editor_images_loaded = false;

	// Listen to the various managers
	listenTo(theResourceManager);
	listenTo(theArchiveManager);
	listenTo(thePaletteChooser);
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

	// Look for stand-alone textures first
	ArchiveEntry * etex = theResourceManager->getTextureEntry(name, "hires", archive);
	int textypefound = TEXTYPE_HIRES;
	if (etex == NULL) {
		etex = theResourceManager->getTextureEntry(name, "textures", archive);
		textypefound = TEXTYPE_TEXTURE;
	}
	if (etex == NULL) {
		etex = theResourceManager->getTextureEntry(name, "flats", archive);
		textypefound = TEXTYPE_FLAT;
	}
	if (etex) {
		SImage image;
		// Get image format hint from type, if any
		string format_hint = "";
		if (etex->getType()->extraProps().propertyExists("image_format"))
			format_hint = etex->getType()->extraProps()["image_format"].getStringValue();
		if (image.open(etex->getMCData(), 0, format_hint)) {
			mtex.texture = new GLTexture(false);
			mtex.texture->loadImage(&image, pal);
		}
	}

	// Try composite textures then
	CTexture* ctex = theResourceManager->getTexture(name, archive);
	if (ctex && (!mtex.texture || textypefound == TEXTYPE_FLAT)) {
		textypefound = TEXTYPE_WALLTEXTURE;
		SImage image;
		if (ctex->toImage(image, archive, pal)) {
			mtex.texture = new GLTexture(false);
			mtex.texture->loadImage(&image, pal);
		}
	}
	return mtex.texture;

}

GLTexture* MapTextureManager::getFlat(string name) {
	// Get flat matching name
	map_tex_t& mtex = flats[name.Upper()];

	// Return it if found
	if (mtex.texture)
		return mtex.texture;

	// Flat not found, look for it
	Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();
	ArchiveEntry * entry = theResourceManager->getTextureEntry(name, "hires", archive);
	if (entry == NULL)
		entry = theResourceManager->getTextureEntry(name, "textures", archive);
	if (entry == NULL)
		entry = theResourceManager->getFlatEntry(name, archive);
	if (entry) {
		SImage image;
		if (Misc::loadImageFromEntry(&image, entry)) {
			mtex.texture = new GLTexture(false);
			mtex.texture->loadImage(&image, pal);
		}
	}

	// Try composite textures then
	CTexture* ctex = theResourceManager->getTexture(name, archive);
	if (ctex && !mtex.texture) {
		SImage image;
		if (ctex->toImage(image, archive, pal)) {
			mtex.texture = new GLTexture(false);
			mtex.texture->loadImage(&image, pal);
		}
	}

	return mtex.texture;
}

GLTexture* MapTextureManager::getSprite(string name, string translation, string palette) {
	// Get sprite matching name
	string hashname = name.Upper();
	if (!translation.IsEmpty())
		hashname += translation.Lower();
	if (!palette.IsEmpty())
		hashname += palette.Upper();
	map_tex_t& mtex = sprites[hashname];

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
		// Apply translation
		if (!translation.IsEmpty()) image.applyTranslation(translation, pal);
		// Apply palette override
		if (!palette.IsEmpty()) {
			ArchiveEntry * newpal = theResourceManager->getPaletteEntry(palette, archive);
			if (newpal && newpal->getSize() == 768) {
				pal = image.getPalette();
				MemChunk mc = newpal->getMCData();
				mc.seek(0, SEEK_SET);
				pal->loadMem(mc);
			}
		}
		mtex.texture = new GLTexture(false);
		mtex.texture->setFilter(GLTexture::NEAREST_LINEAR_MIN);
		mtex.texture->loadImage(&image, pal);
		return mtex.texture;
	}

	return NULL;
}

void importEditorImages(MapTexHashMap& map, ArchiveTreeNode* dir, string path) {
	SImage image;

	// Go through entries
	for (unsigned a = 0; a < dir->numEntries(); a++) {
		ArchiveEntry* entry = dir->getEntry(a);

		// Load entry to image
		if (image.open(entry->getMCData())) {
			// Create texture in hashmap
			string name = path + entry->getName(true);
			//wxLogMessage("Loading editor texture %s", CHR(name));
			map_tex_t& mtex = map[name];
			mtex.texture = new GLTexture(false);
			mtex.texture->setFilter(GLTexture::MIPMAP);
			mtex.texture->loadImage(&image);
		}
	}

	// Go through subdirs
	for (unsigned a = 0; a < dir->nChildren(); a++) {
		ArchiveTreeNode* subdir = (ArchiveTreeNode*)dir->getChild(a);
		importEditorImages(map, subdir, path + subdir->getName() + "/");
	}
}

GLTexture* MapTextureManager::getEditorImage(string name) {
	// Load thing image textures if they haven't already
	if (!editor_images_loaded) {
		// Load all thing images to textures
		Archive* slade_pk3 = theArchiveManager->programResourceArchive();
		ArchiveTreeNode* dir = slade_pk3->getDir("images");
		if (dir)
			importEditorImages(editor_images, dir, "");

		editor_images_loaded = true;
	}

	return editor_images[name].texture;
}

void MapTextureManager::refreshResources() {
	// Just clear all cached textures
	textures.clear();
	flats.clear();
	sprites.clear();
	thePaletteChooser->setGlobalFromArchive(archive);
	wxLogMessage("texture manager cleared");
}

void MapTextureManager::setArchive(Archive* archive) {
	this->archive = archive;
	refreshResources();
}

void MapTextureManager::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Only interested in the resource manager, 
	// archive manager and palette chooser.
	if (announcer != theResourceManager
		&& announcer != thePaletteChooser
		&& announcer != theArchiveManager)
		return;

	// If the map's archive is being closed,
	// we need to close the map editor
	if (event_name == "archive_closing") {
		event_data.seek(0, SEEK_SET);
		int32_t ac_index;
		event_data.read(&ac_index, 4);
		if (theArchiveManager->getArchive(ac_index) == archive) {
			theMapEditor->Hide();
			theMapEditor->mapEditor().clearMap();
			archive = NULL;
		}
	}

	// If the resources have been updated
	if (event_name == "resources_updated")
		refreshResources();

	if (event_name == "main_palette_changed")
		refreshResources();
}
