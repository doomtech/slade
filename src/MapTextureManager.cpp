
#include "Main.h"
#include "MapTextureManager.h"
#include "ResourceManager.h"
#include "CTexture.h"
#include "MainWindow.h"

MapTextureManager::MapTextureManager(Archive* archive) {
	// Init variables
	this->archive = archive;
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
