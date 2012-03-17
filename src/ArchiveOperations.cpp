
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
#include "ArchiveManager.h"
#include "TextureXList.h"
#include "ResourceManager.h"
#include "ExtMessageDialog.h"
#include "MainWindow.h"
#include "MapSide.h"
#include "MapSector.h"
#include "Console.h"
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

/* ArchiveOperations::removeEntriesUnchangedFromIWAD
 * Compare the archive's entries with those sharing the same name
 * and namespace in the base resource archive, deleting duplicates
 *******************************************************************/
void ArchiveOperations::removeEntriesUnchangedFromIWAD(Archive* archive) {
	// Do nothing if there is no base resource archive,
	// or if the archive *is* the base resource archive.
	Archive* bra = theArchiveManager->baseResourceArchive();
	if (bra == NULL || bra == archive || archive == NULL)
		return;

	// Get list of all entries in archive
	vector<ArchiveEntry*> entries;
	archive->getEntryTreeAsList(entries);

	// Init search options
	Archive::search_options_t search;
	ArchiveEntry* other = NULL;
	string dups = "";
	size_t count = 0;

	// Go through list
	for (unsigned a = 0; a < entries.size(); a++) {
		// Skip directory entries
		if (entries[a]->getType() == EntryType::folderType())
			continue;

		// Skip markers
		if (entries[a]->getType() == EntryType::mapMarkerType() || entries[a]->getSize() == 0)
			continue;

		// Now, let's look for a counterpart in the IWAD
		search.match_namespace = archive->detectNamespace(entries[a]);
		search.match_name = entries[a]->getName();
		other = bra->findLast(search);
		
		// If there is one, and it is identical, remove it
		if (other != NULL && (other->getMCData().crc() == entries[a]->getMCData().crc())) {
			++count;
			dups += S_FMT("%s\n", CHR(search.match_name));
			archive->removeEntry(entries[a], true);
			entries[a] = NULL;
		}
	}


	// If no duplicates exist, do nothing
	if (count == 0) {
		wxMessageBox("No duplicated entries exist");
		return;
	}

	string message = S_FMT("The following %d entr%s were duplicated from the base resource archive and deleted:",
		count, (count > 1) ? "ies" : "y");

	// Display list of deleted duplicate entries
	ExtMessageDialog msg(theMainWindow, (count > 1) ? "Deleted Entries" : "Deleted Entry");
	msg.setExt(dups);
	msg.setMessage(message);
	msg.ShowModal();
}





// Hardcoded doom defaults for now
int n_tex_anim = 13;
string tex_anim_start[] = {
	"BLODGR1",
	"SLADRIP1",
	"BLODRIP1",
	"FIREWALA",
	"GSTFONT1",
	"FIRELAV3",
	"FIREMAG1",
	"FIREBLU1",
	"ROCKRED1",
	"BFALL1",
	"SFALL1",
	"WFALL1",
	"DBRAIN1",
};
string tex_anim_end[] = {
	"BLODGR4",
	"SLADRIP3",
	"BLODRIP4",
	"FIREWALL",
	"GSTFONT3",
	"FIRELAVA",
	"FIREMAG3",
	"FIREBLU2",
	"ROCKRED3",
	"BFALL4",
	"SFALL4",
	"WFALL4",
	"DBRAIN4",
};

int n_flat_anim = 9;
string flat_anim_start[] = {
	"NUKAGE1",
	"FWATER1",
	"SWATER1",
	"LAVA1",
	"BLOOD1",
	"RROCK05",
	"SLIME01",
	"SLIME05",
	"SLIME09",
};
string flat_anim_end[] = {
	"NUKAGE3",
	"FWATER4",
	"SWATER4",
	"LAVA4",
	"BLOOD3",
	"RROCK08",
	"SLIME04",
	"SLIME08",
	"SLIME12",
};

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
	int total_maps = 0;

	// Get all SIDEDEFS entries
	Archive::search_options_t opt;
	opt.match_type = EntryType::getType("map_sidedefs");
	vector<ArchiveEntry*> sidedefs = archive->findAll(opt);
	total_maps += sidedefs.size();

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

	// Get all TEXTMAP entries
	opt.match_name = "TEXTMAP";
	opt.match_type = EntryType::getType("udmf_textmap");
	vector<ArchiveEntry*> udmfmaps = archive->findAll(opt);
	total_maps += udmfmaps.size();

	// Go through and add used textures to list
	Tokenizer tz;
	tz.setSpecialCharacters("{};=");
	for (unsigned a = 0; a < udmfmaps.size(); a++) {
		// Open in tokenizer
		tz.openMem(udmfmaps[a]->getData(), udmfmaps[a]->getSize(), "UDMF TEXTMAP");

		// Go through text tokens
		string token = tz.getToken();
		while (!token.IsEmpty()) {
			// Check for sidedef definition
			if (token == "sidedef") {
				tz.getToken();	// Skip {

				token = tz.getToken();
				while (token != "}") {
					// Check for texture property
					if (token == "texturetop" || token == "texturemiddle" || token == "texturebottom") {
						tz.getToken();	// Skip =
						used_textures[tz.getToken()].used = true;
					}

					token = tz.getToken();
				}
			}

			// Next token
			token = tz.getToken();
		}
	}

	// Check if any maps were found
	if (total_maps == 0)
		return;

	// Find all TEXTUREx entries
	opt.match_name = "";
	opt.match_type = EntryType::getType("texturex");
	vector<ArchiveEntry*> tx_entries = archive->findAll(opt);

	// Go through texture lists
	PatchTable ptable;	// Dummy patch table, patch info not needed here
	wxArrayString unused_tex;
	for (unsigned a = 0; a < tx_entries.size(); a++) {
		TextureXList txlist;
		txlist.readTEXTUREXData(tx_entries[a], ptable);

		// Go through textures
		bool anim = false;
		for (unsigned t = 1; t < txlist.nTextures(); t++) {
			string texname = txlist.getTexture(t)->getName();

			// Check for animation start
			for (int b = 0; b < n_tex_anim; b++) {
				if (texname == tex_anim_start[b]) {
					anim = true;
					break;
				}
			}

			// Check for animation end
			bool thisend = false;
			for (int b = 0; b < n_tex_anim; b++) {
				if (texname == tex_anim_end[b]) {
					anim = false;
					thisend = true;
					break;
				}
			}

			// Mark if unused and not part of an animation
			if (!used_textures[texname].used && !anim && !thisend)
				unused_tex.Add(CHR(txlist.getTexture(t)->getName()));
		}
	}

	// Pop up a dialog with a checkbox list of unused textures
	wxMultiChoiceDialog dialog(theMainWindow, "The following textures are not used in any map,\nselect which textures to delete", "Delete Unused Textures", unused_tex);
	
	// Determine which textures to check initially
	wxArrayInt selection;
	for (unsigned a = 0; a < unused_tex.size(); a++) {
		bool swtex = false;

		// Check for switch texture
		if (unused_tex[a].StartsWith("SW1")) {
			// Get counterpart switch name
			string swname = unused_tex[a];
			swname.Replace("SW1", "SW2", false);

			// Check if it's counterpart is used
			if (used_textures[swname].used)
				swtex = true;
		}
		else if (unused_tex[a].StartsWith("SW2")) {
			// Get counterpart switch name
			string swname = unused_tex[a];
			swname.Replace("SW2", "SW1", false);

			// Check if it's counterpart is used
			if (used_textures[swname].used)
				swtex = true;
		}

		if (!swtex)
			selection.Add(a);
	}
	dialog.SetSelections(selection);
	
	int n_removed = 0;
	if (dialog.ShowModal() == wxID_OK) {
		// Get selected textures
		selection = dialog.GetSelections();

		// Go through texture lists
		for (unsigned a = 0; a < tx_entries.size(); a++) {
			TextureXList txlist;
			txlist.readTEXTUREXData(tx_entries[a], ptable);

			// Go through selected textures to delete
			for (unsigned t = 0; t < selection.size(); t++) {
				// Get texture index
				int index = txlist.textureIndex(unused_tex[selection[t]]);

				// Delete it from the list (if found)
				if (index >= 0) {
					txlist.removeTexture(index);
					n_removed++;
				}
			}

			// Write texture list data back to entry
			txlist.writeTEXTUREXData(tx_entries[a], ptable);
		}
	}

	wxMessageBox(S_FMT("Removed %d unused textures", n_removed));
}

void ArchiveOperations::removeUnusedFlats(Archive* archive) {
	// Check archive was given
	if (!archive)
		return;

	// --- Build list of used flats ---
	TexUsedMap used_textures;
	int total_maps = 0;

	// Get all SECTORS entries
	Archive::search_options_t opt;
	opt.match_type = EntryType::getType("map_sectors");
	vector<ArchiveEntry*> sectors = archive->findAll(opt);
	total_maps += sectors.size();

	// Go through and add used flats to list
	doomsector_t sec;
	string tex_floor, tex_ceil;
	for (unsigned a = 0; a < sectors.size(); a++) {
		int nsec = sectors[a]->getSize() / 26;
		sectors[a]->seek(0, SEEK_SET);
		for (int s = 0; s < nsec; s++) {
			// Read sector data
			sectors[a]->read(&sec, 26);

			// Get textures
			tex_floor = wxString::FromAscii(sec.f_tex, 8);
			tex_ceil = wxString::FromAscii(sec.c_tex, 8);

			// Add to used textures list
			used_textures[tex_floor].used = true;
			used_textures[tex_ceil].used = true;
		}
	}

	// Get all TEXTMAP entries
	opt.match_name = "TEXTMAP";
	opt.match_type = EntryType::getType("udmf_textmap");
	vector<ArchiveEntry*> udmfmaps = archive->findAll(opt);
	total_maps += udmfmaps.size();

	// Go through and add used flats to list
	Tokenizer tz;
	tz.setSpecialCharacters("{};=");
	for (unsigned a = 0; a < udmfmaps.size(); a++) {
		// Open in tokenizer
		tz.openMem(udmfmaps[a]->getData(), udmfmaps[a]->getSize(), "UDMF TEXTMAP");

		// Go through text tokens
		string token = tz.getToken();
		while (!token.IsEmpty()) {
			// Check for sector definition
			if (token == "sector") {
				tz.getToken();	// Skip {

				token = tz.getToken();
				while (token != "}") {
					// Check for texture property
					if (token == "texturefloor" || token == "textureceiling") {
						tz.getToken();	// Skip =
						used_textures[tz.getToken()].used = true;
					}

					token = tz.getToken();
				}
			}

			// Next token
			token = tz.getToken();
		}
	}

	// Check if any maps were found
	if (total_maps == 0)
		return;

	// Find all flats
	opt.match_name = "";
	opt.match_namespace = "flats";
	opt.match_type = NULL;
	vector<ArchiveEntry*> flats = archive->findAll(opt);

	// Create list of all unused flats
	wxArrayString unused_tex;
	bool anim = false;
	for (unsigned a = 0; a < flats.size(); a++) {
		// Skip markers
		if (flats[a]->getSize() == 0)
			continue;

		// Check for animation start
		string flatname = flats[a]->getName(true);
		for (int b = 0; b < n_flat_anim; b++) {
			if (flatname == flat_anim_start[b]) {
				anim = true;
				wxLogMessage("%s anim start", CHR(flatname));
				break;
			}
		}

		// Check for animation end
		bool thisend = false;
		for (int b = 0; b < n_flat_anim; b++) {
			if (flatname == flat_anim_end[b]) {
				anim = false;
				thisend = true;
				wxLogMessage("%s anim end", CHR(flatname));
				break;
			}
		}

		// Add if not animated
		if (!used_textures[flatname].used && !anim && !thisend)
			unused_tex.Add(flatname);
	}

	// Pop up a dialog with a checkbox list of unused textures
	wxMultiChoiceDialog dialog(theMainWindow, "The following textures are not used in any map,\nselect which textures to delete", "Delete Unused Textures", unused_tex);
	
	// Select all flats initially
	wxArrayInt selection;
	for (unsigned a = 0; a < unused_tex.size(); a++)
		selection.push_back(a);
	dialog.SetSelections(selection);

	int n_removed = 0;
	if (dialog.ShowModal() == wxID_OK) {
		// Go through selected flats
		selection = dialog.GetSelections();
		opt.match_namespace = "flats";
		for (unsigned a = 0; a < selection.size(); a++) {
			opt.match_name = unused_tex[selection[a]];
			ArchiveEntry* entry = archive->findFirst(opt);
			archive->removeEntry(entry);
			n_removed++;
		}
	}

	wxMessageBox(S_FMT("Removed %d unused flats", n_removed));
}


CONSOLE_COMMAND(test_cleantex, 0) {
	Archive* current = theMainWindow->getCurrentArchive();
	if (current) ArchiveOperations::removeUnusedTextures(current);
}

CONSOLE_COMMAND(test_cleanflats, 0) {
	Archive* current = theMainWindow->getCurrentArchive();
	if (current) ArchiveOperations::removeUnusedFlats(current);
}
