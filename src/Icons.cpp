
#include "Main.h"
#include "WxStuff.h"
#include "Icons.h"
#include "ArchiveManager.h"
#include "ZipArchive.h"

struct icon_t {
	wxImage	image;
	string	name;
};
vector<icon_t>	icons;
wxBitmap icon_empty;


bool loadIcons() {
	string tempfile = appPath(_T("sladetemp"), DIR_TEMP);

	// Get slade.pk3
	ZipArchive* res_archive = (ZipArchive*)theArchiveManager->resourceArchive();

	// Do nothing if it doesn't exist
	if (!res_archive)
		return false;

	// Get the icons directory of the archive
	zipdir_t* dir_icons = res_archive->getDirectory(_T("icons"));

	// Go through each entry in the directory
	for (int a = 0; a < dir_icons->numEntries(false); a++) {
		ArchiveEntry* entry = dir_icons->entries[a];

		// Export entry data to a temporary file
		entry->exportFile(tempfile);

		// Create / setup icon
		icon_t n_icon;
		n_icon.image.LoadFile(tempfile);	// Load image from temp file
		n_icon.name = entry->getName(true);	// Set icon name

		// Add the icon
		icons.push_back(n_icon);

		// Delete the temporary file
		wxRemoveFile(tempfile);
	}
}

wxBitmap getIcon(string name) {
	for (int a = 0; a < icons.size(); a++) {
		if (icons[a].name.Cmp(name) == 0)
			return wxBitmap(icons[a].image);
	}

	return wxNullBitmap;
}
