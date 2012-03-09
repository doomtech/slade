
#include "Main.h"
#include "WxStuff.h"
#include "MapTextureBrowser.h"
#include "MapEditorWindow.h"


MapTexBrowserItem::MapTexBrowserItem(string name) : BrowserItem(name) {
}

MapTexBrowserItem::~MapTexBrowserItem() {
}

bool MapTexBrowserItem::loadImage() {
	GLTexture* tex = theMapEditor->textureManager().getTexture(name);
	if (tex) {
		image = tex;
		return true;
	}
	else
		return false;
}



MapTextureBrowser::MapTextureBrowser(wxWindow* parent) : BrowserWindow(parent) {
}

MapTextureBrowser::~MapTextureBrowser() {
}
