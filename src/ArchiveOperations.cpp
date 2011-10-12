
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ArchiveOperations.cpp
 * Description: Functions that perform specific operations on archives
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "ArchiveOperations.h"
#include "TextureXList.h"
#include "ResourceManager.h"
#include "ExtMessageDialog.h"
#include "MainWindow.h"
#include "MapSide.h"
#include <wx/hashmap.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
WX_DECLARE_STRING_HASH_MAP(int, StrIntMap);


/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

/* ArchiveOperations::removeUnusedPatches
 * Removes any patches and associated entries from [archive] that
 * are not used in any texture definitions
 *******************************************************************/
bool ArchiveOperations::removeUnusedPatches(Archive* archive) {
	if (!archive)
		return false;

	// Find PNAMES entry
	Archive::search_options_t opt;
	opt.match_type = EntryType::getType("pnames");
	ArchiveEntry* pnames = archive->findLast(opt);

	// Find TEXTUREx entries
	opt.match_type = EntryType::getType("texturex");
	vector<ArchiveEntry*> tx_entries = archive->findAll(opt);

	// Can't do anything without PNAMES/TEXTUREx
	if (!pnames || tx_entries.size() == 0)
		return false;

	// Open patch table
	PatchTable ptable;
	ptable.loadPNAMES(pnames, archive);

	// Open texturex entries to update patch usage
	vector<TextureXList*> tx_lists;
	for (unsigned a = 0; a < tx_entries.size(); a++) {
		TextureXList* texturex = new TextureXList();
		texturex->readTEXTUREXData(tx_entries[a], ptable);
		for (unsigned t = 0; t < texturex->nTextures(); t++)
			ptable.updatePatchUsage(texturex->getTexture(t));
		tx_lists.push_back(texturex);
	}

	// Go through patch table
	unsigned removed = 0;
	vector<ArchiveEntry*> to_remove;
	for (unsigned a = 0; a < ptable.nPatches(); a++) {
		patch_t& p = ptable.patch(a);

		// Check if used in any texture
		if (p.used_in.size() == 0) {
			// Unused

			// If its entry is in the archive, flag it to be removed
			ArchiveEntry* entry = theResourceManager->getPatchEntry(p.name, "patches", archive);
			if (entry && entry->getParent() == archive)
				to_remove.push_back(entry);

			// Update texturex list patch indices
			for (unsigned t = 0; t < tx_lists.size(); t++)
				tx_lists[t]->removePatch(p.name);

			// Remove the patch from the patch table
			wxLogMessage("Removed patch %s", p.name);
			removed++;
			ptable.removePatch(a--);
		}
	}

	// Remove unused patch entries
	for (unsigned a = 0; a < to_remove.size(); a++) {
		wxLogMessage("Removed entry %s", to_remove[a]->getName());
		archive->removeEntry(to_remove[a]);
	}

	// Write PNAMES changes
	ptable.writePNAMES(pnames);

	// Write TEXTUREx changes
	for (unsigned a = 0; a < tx_lists.size(); a++)
		tx_lists[a]->writeTEXTUREXData(tx_entries[a], ptable);

	// Cleanup
	for (unsigned a = 0; a < tx_lists.size(); a++)
		delete tx_lists[a];

	// Notify user
	wxMessageBox(S_FMT("Removed %d patches and %d entries. See console log for details.", removed, to_remove.size()), "Removed Unused Patches", wxOK|wxICON_INFORMATION);

	return true;
}

/* ArchiveOperations::checkDuplicateEntryNames
 * Checks [archive] for multiple entries of the same name, and
 * displays a list of duplicate entry names if any are found
 *******************************************************************/
bool ArchiveOperations::checkDuplicateEntryNames(Archive* archive) {
	StrIntMap map_namecounts;
	
	// Get list of all entries in archive
	vector<ArchiveEntry*> entries;
	archive->getEntryTreeAsList(entries);

	// Go through list
	for (unsigned a = 0; a < entries.size(); a++) {
		// Skip directory entries
		if (entries[a]->getType() == EntryType::folderType())
			continue;

		// Increment count for entry name
		map_namecounts[entries[a]->getPath(true)] += 1;
	}

	// Generate string of duplicate entry names
	string dups;
	StrIntMap::iterator i = map_namecounts.begin();
	while (i != map_namecounts.end()) {
		if (i->second > 1) {
			string name = i->first;
			name.Remove(0, 1);
			dups += S_FMT("%s appears %d times\n", CHR(name), i->second);
		}
		i++;
	}

	// If no duplicates exist, do nothing
	if (dups.IsEmpty()) {
		wxMessageBox("No duplicated entry names exist");
		return false;
	}

	// Display list of duplicate entry names
	ExtMessageDialog msg(theMainWindow, "Duplicate Entries");
	msg.setExt(dups);
	msg.setMessage("The following entry names are duplicated:");
	msg.ShowModal();

	return true;
}


struct texused_t {
	bool used;
	texused_t() { used = false; }
};
WX_DECLARE_STRING_HASH_MAP(texused_t, TexUsedMap);
void ArchiveOperations::removeUnusedTextures(Archive* archive) {
	// Check archive was given
	if (!archive)
		return;

	// --- Build list of used textures ---
	TexUsedMap used_textures;

	// Get all SIDEDEFS entries
	Archive::search_options_t opt;
	opt.match_type = EntryType::getType("map_sidedefs");
	vector<ArchiveEntry*> sidedefs = archive->findAll(opt);

	// Go through and add used textures to list
	doomside_t sdef;
	string tex_lower, tex_middle, tex_upper;
	for (unsigned a = 0; a < sidedefs.size(); a++) {
		int nsides = sidedefs[a]->getSize() / 30;
		sidedefs[a]->seek(0, SEEK_SET);
		for (int s = 0; s < nsides; s++) {
			// Read side data
			sidedefs[a]->read(&sdef, 30);

			// Get textures
			tex_lower = wxString::FromAscii(sdef.tex_lower, 8);
			tex_middle = wxString::FromAscii(sdef.tex_middle, 8);
			tex_upper = wxString::FromAscii(sdef.tex_upper, 8);

			// Add to used textures list
			used_textures[tex_lower].used = true;
			used_textures[tex_middle].used = true;
			used_textures[tex_upper].used = true;
		}
	}

	// Find all TEXTUREx entries
	opt.match_type = EntryType::getType("texturex");
	vector<ArchiveEntry*> tx_entries = archive->findAll(opt);

	// Go through texture lists
	PatchTable ptable;	// Dummy patch table, patch info not needed here
	for (unsigned a = 0; a < tx_entries.size(); a++) {
		TextureXList txlist;
		txlist.readTEXTUREXData(tx_entries[a], ptable);

		// Go through textures
		for (unsigned t = 0; t < txlist.nTextures(); t++) {
			// Remove texture if unused
			if (!used_textures[txlist.getTexture(a)->getName()].used) {
				wxLogMessage("Removed unused texture %s", CHR(txlist.getTexture(a)->getName()));
				txlist.removeTexture(a);
				a--;
			}
		}
	}
}
