
#include "Main.h"
#include "BrowserItem.h"

BrowserItem::BrowserItem(string name) {
	this->name = name;
}

BrowserItem::~BrowserItem() {
}

bool BrowserItem::loadImage() {
	return false;
}

void BrowserItem::draw(int size) {
	// Load texture if needed
	if (!image.isLoaded())
		loadImage();
}
