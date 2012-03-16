
#include "Main.h"
#include "WxStuff.h"
#include "MapTextureBrowser.h"
#include "MapEditorWindow.h"
#include "ResourceManager.h"
#include "CTexture.h"
#include "GameConfiguration.h"


MapTexBrowserItem::MapTexBrowserItem(string name, int type, unsigned index) : BrowserItem(name, index) {
	if (type == 0)
		this->type = "texture";
	else if (type == 1)
		this->type = "flat";
}

MapTexBrowserItem::~MapTexBrowserItem() {
}

bool MapTexBrowserItem::loadImage() {
	GLTexture* tex = NULL;

	// Get texture or flat depending on type
	if (type == "texture")
		tex = theMapEditor->textureManager().getTexture(name);
	else if (type == "flat")
		tex = theMapEditor->textureManager().getFlat(name);

	if (tex) {
		image = tex;
		return true;
	}
	else
		return false;
}



MapTextureBrowser::MapTextureBrowser(wxWindow* parent, int type, string texture) : BrowserWindow(parent) {
	// Init variables
	this->type = type;
	setSortType(1);

	// Textures
	if (type == 0 || theGameConfiguration->mixTexFlats()) {
		// Composite textures
		vector<TextureResource::tex_res_t> textures;
		theResourceManager->getAllTextures(textures, NULL);
		for (unsigned a = 0; a < textures.size(); a++)
			addItem(new MapTexBrowserItem(textures[a].tex->getName(), 0, textures[a].tex->getIndex()), "Textures/TEXTUREx");

		// Texture namespace patches (TX_)
		if (theGameConfiguration->txTextures()) {
			vector<ArchiveEntry*> patches;
			theResourceManager->getAllPatchEntries(patches, NULL);
			for (unsigned a = 0; a < patches.size(); a++) {
				if (patches[a]->isInNamespace("textures")) {
					// Determine texture path if it's in a pk3
					string path = patches[a]->getPath();
					if (path.StartsWith("/textures/"))
						path.Remove(0, 9);
					else
						path = "";

					addItem(new MapTexBrowserItem(patches[a]->getName(true), 0, a), "Textures/Textures (TX)" + path);
				}
			}
		}
	}

	// Flats
	if (type == 1 || theGameConfiguration->mixTexFlats()) {
		vector<ArchiveEntry*> flats;
		theResourceManager->getAllFlatEntries(flats, NULL);
		for (unsigned a = 0; a < flats.size(); a++) {
			ArchiveEntry* entry = flats[a];

			// Determine flat path if it's in a pk3
			string path = entry->getPath();
			if (path.StartsWith("/flats/"))
				path.Remove(0, 6);
			else
				path = "";

			addItem(new MapTexBrowserItem(entry->getName(true), 1, entry->getParentDir()->entryIndex(entry)), "Flats" + path);
		}
	}

	populateItemTree();

	// Select initial texture (if any)
	if (!texture.IsEmpty())
		selectItem(texture);
}

MapTextureBrowser::~MapTextureBrowser() {
}
