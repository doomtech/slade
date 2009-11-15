
#ifndef __PALETTEMANAGER_H__
#define	__PALETTEMANAGER_H__

#include "Palette.h"

class PaletteManager {
private:
	vector<Palette8bit*>	palettes;
	vector<string>			pal_names;

	static PaletteManager*	instance;

public:
	PaletteManager();
	~PaletteManager();

	static PaletteManager*	getInstance() {
		if (!instance)
			instance = new PaletteManager();

		return instance;
	}

	bool	loadResourcePalettes();
	bool	loadCustomPalettes();
};

// Define for less cumbersome PaletteManager::getInstance()
#define thePaletteManager PaletteManager::getInstance()

#endif //__PALETTEMANAGER_H__
