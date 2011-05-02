
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
	wxMessageBox(S_FMT("Removed %d patches and %d entries. See console log for details.", removed, to_remove.size()), "Removed Unused Patches");

	return true;
}
