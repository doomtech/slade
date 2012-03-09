
#include "Main.h"
#include "WxStuff.h"
#include "ThingTypeBrowser.h"
#include "MapEditorWindow.h"
#include "GameConfiguration.h"
#include "Drawing.h"


ThingBrowserItem::ThingBrowserItem(string name, ThingType* type, unsigned index) : BrowserItem(name, index) {
	// Init variables
	this->type = type;
}

ThingBrowserItem::~ThingBrowserItem() {
}

bool ThingBrowserItem::loadImage() {
	// Get sprite
	GLTexture* tex = theMapEditor->textureManager().getSprite(type->getSprite(), type->getTranslation(), type->getPalette());
	if (!tex) {
		// Sprite not found, try an icon
		tex = theMapEditor->textureManager().getEditorImage(S_FMT("thing/%s", CHR(type->getIcon())));
	}
	if (!tex) {
		// Icon not found either, use unknown icon
		tex = theMapEditor->textureManager().getEditorImage("thing/unknown");
	}

	if (tex) {
		image = tex;
		return true;
	}
	else
		return false;
}



ThingTypeBrowser::ThingTypeBrowser(wxWindow* parent, int type) : BrowserWindow(parent) {
	// Populate tree
	vector<tt_t> types = theGameConfiguration->allThingTypes();
	for (unsigned a = 0; a < types.size(); a++)
		addItem(new ThingBrowserItem(types[a].type->getName(), types[a].type, types[a].number), types[a].type->getGroup());
	populateItemTree();

	// Set browser options
	setFont(Drawing::FONT_CONDENSED);
	showNames(BrowserCanvas::NAMES_SELECTED);
	setItemSize(64);

	// Select initial item if any
	if (type >= 0)
		selectItem(theGameConfiguration->thingType(type)->getName());
	else
		openTree(items_root);	// Otherwise open 'all' category
}

ThingTypeBrowser::~ThingTypeBrowser() {
}

int ThingTypeBrowser::getSelectedType() {
	BrowserItem* selected = getSelectedItem();
	if (selected)
		return selected->getIndex();
	else
		return -1;
}
