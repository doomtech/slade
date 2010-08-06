/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryType.cpp
 * Description: Entry Type detection system
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
#include "EntryType.h"
#include "Tokenizer.h"
#include "Console.h"
#include "ArchiveManager.h"
#include "ZipArchive.h"
#include "WadArchive.h"
#include "BinaryControlLump.h"
#include "Parser.h"
#include "EntryDataFormat.h"
#include "ConsoleHelpers.h"
#include <wx/dir.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/

vector<EntryType*>	entry_types;	// The big list of all entry types

// Special entry types
EntryType			etype_unknown;	// The default, 'unknown' entry type
EntryType			etype_folder;	// Folder entry type
EntryType			etype_marker;	// Marker entry type
EntryType			etype_map;		// Map marker type


/*******************************************************************
 * ENTRYTYPE CLASS FUNCTIONS
 *******************************************************************/

/* EntryType::EntryType
 * EntryType class constructor
 *******************************************************************/
EntryType::EntryType(string id) {
	// Init info variables
	this->id = id;
	name = "Unknown";
	extension = "dat";
	icon = "e_default";
	editor = "default";
	reliability = 255;
	category = "Data";

	// Init match criteria
	format = "any";
	size_limit[0] = -1;
	size_limit[1] = -1;
	detectable = true;
	section = "none";
}

/* EntryType::~EntryType
 * EntryType class constructor
 *******************************************************************/
EntryType::~EntryType() {
	match_extension.clear();
	match_name.clear();
	match_size.clear();
	match_archive.clear();
	size_multiple.clear();
}

/* EntryType::addToList
 * Adds the type to the list of entry types
 *******************************************************************/
void EntryType::addToList() {
	entry_types.push_back(this);
	index = entry_types.size() - 1;
}

/* EntryType::dump
 * Dumps entry type info to the log
 *******************************************************************/
void EntryType::dump() {
	wxLogMessage(s_fmt("Type %s \"%s\", format %s, extension %s", chr(id), chr(name), chr(format), chr(extension)));
	wxLogMessage(s_fmt("Size limit: %d-%d", size_limit[0], size_limit[1]));

	for (size_t a = 0; a < match_archive.size(); a++)
		wxLogMessage(s_fmt("Match Archive: \"%s\"", chr(match_archive[a])));

	for (size_t a = 0; a < match_extension.size(); a++)
		wxLogMessage(s_fmt("Match Extension: \"%s\"", chr(match_extension[a])));

	for (size_t a = 0; a < match_name.size(); a++)
		wxLogMessage(s_fmt("Match Name: \"%s\"", chr(match_name[a])));

	for (size_t a = 0; a < match_size.size(); a++)
		wxLogMessage(s_fmt("Match Size: %d", match_size[a]));

	for (size_t a = 0; a < size_multiple.size(); a++)
		wxLogMessage(s_fmt("Size Multiple: %d", size_multiple[a]));

	wxLogMessage("---");
}

void EntryType::copyToType(EntryType* target) {
	// Copy type attributes
	target->editor = editor;
	target->extension = extension;
	target->icon = icon;
	target->name = name;
	target->reliability = reliability;

	// Copy type match criteria
	target->format = format;
	target->size_limit[0] = size_limit[0];
	target->size_limit[1] = size_limit[1];
	target->section = section;
	target->match_extension = match_extension;
	target->match_name = match_name;
	target->match_size = match_size;
	target->match_extension = match_extension;
	target->match_archive = match_archive;
}

string EntryType::getFileFilterString() {
	string ret = name + " files (*.";
	ret += extension;
	ret += ")|*.";
	ret += extension;

	return ret;
}

/* EntryType::isThisType
 * Returns true if [entry] matches the EntryType's criteria, false
 * otherwise
 *******************************************************************/
bool EntryType::isThisType(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Check type is detectable
	if (!detectable)
		return false;

	// Get full entry name as filename
	wxFileName fn(entry->getName());

	// Check min size
	if (size_limit[0] >= 0 && entry->getSize() < (unsigned)size_limit[0])
		return false;

	// Check max size
	if (size_limit[1] >= 0 && entry->getSize() > (unsigned)size_limit[1])
		return false;

	// Check for archive match if needed
	if (match_archive.size() > 0) {
		bool match = false;
		for (size_t a = 0; a < match_archive.size(); a++) {
			if (entry->getParent() && entry->getParent()->getFormat() == match_archive[a]) {
				match = true;
				break;
			}
		}
		if (!match)
			return false;
	}

	// Check for size match if needed
	if (match_size.size() > 0) {
		bool match = false;
		for (size_t a = 0; a < match_size.size(); a++) {
			if (entry->getSize() == match_size[a]) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for name match if needed
	if (match_name.size() > 0) {
		string name = fn.GetName().Lower();
		bool match = false;
		for (size_t a = 0; a < match_name.size(); a++) {
			if (name.Matches(match_name[a].Lower())) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for extension match if needed
	if (match_extension.size() > 0) {
		bool match = false;
		for (size_t a = 0; a < match_extension.size(); a++) {
			if (!fn.GetExt().CmpNoCase(match_extension[a])) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for size multiple match if needed
	if (size_multiple.size() > 0) {
		bool match = false;
		for (size_t a = 0; a < size_multiple.size(); a++) {
			if (entry->getSize() % size_multiple[a] == 0) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for entry section match if needed
	if (section != "none") {
		// Check entry is part of an archive (if not it can't be in a section)
		if (!entry->getParent())
			return false;

		string e_section = entry->getParent()->detectNamespace(entry);

		if (e_section != section)
			return false;
	}

	// Check for data format match if needed
	if (format == "text") {
		// Text is a special case, as other data formats can sometimes be detected as 'text',
		// we'll only check for it if text data is specified in the entry type
		if (memchr(entry->getData(), 0, entry->getSize()-1) != NULL)
			return false;
	}
	else if (format != "any") {
		//EntryDataFormat::anyFormat()->isThisFormat(entry->getMCData());
		if (!(EntryDataFormat::getFormat(format)->isThisFormat(entry->getMCData())))
			return false;
	}

	// Passed all checks, so we have a match
	return true;
}

/* EntryType::readEntryTypeDefinition
 * Reads in a block of entry type definitions. Returns false if there
 * was a parsing error, true otherwise
 *******************************************************************/
bool EntryType::readEntryTypeDefinition(MemChunk& mc) {
	// Parse the definition
	Parser p;
	p.parseText(mc);

	// Get entry_types tree
	ParseTreeNode* pt_etypes = (ParseTreeNode*)(p.parseTreeRoot()->getChild("entry_types"));

	// Check it exists
	if (!pt_etypes)
		return false;

	// Go through all parsed types
	for (unsigned a = 0; a < pt_etypes->nChildren(); a++) {
		// Get child as ParseTreeNode
		ParseTreeNode* typenode = (ParseTreeNode*)pt_etypes->getChild(a);

		// Create new entry type
		EntryType* ntype = new EntryType(typenode->getName().Lower());

		// Copy from existing type if inherited
		if (!typenode->getInherit().IsEmpty()) {
			EntryType* parent_type = EntryType::getType(typenode->getInherit().Lower());

			if (parent_type != EntryType::unknownType())
				parent_type->copyToType(ntype);
			else
				wxLogMessage("Warning: Entry type %s inherits from unknown type %s", chr(ntype->getId()), chr(typenode->getInherit()));
		}

		// Go through all parsed fields
		for (unsigned b = 0; b < typenode->nChildren(); b++) {
			// Get child as ParseTreeNode
			ParseTreeNode* fieldnode = (ParseTreeNode*)typenode->getChild(b);

			// Process it
			if (s_cmpnocase(fieldnode->getName(), "name")) {				// Name field
				ntype->name = fieldnode->getStringValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "detectable")) {		// Detectable field
				ntype->detectable = fieldnode->getBoolValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "export_ext")) {		// Export Extension field
				ntype->extension = fieldnode->getStringValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "format")) {			// Format field
				ntype->format = fieldnode->getStringValue();

				// Warn if undefined format
				if (EntryDataFormat::getFormat(ntype->getFormat()) == EntryDataFormat::anyFormat())
					wxLogMessage("Warning: Entry type %s requires undefined format %s", chr(ntype->getId()), chr(ntype->getFormat()));
			}
			else if (s_cmpnocase(fieldnode->getName(), "icon")) {			// Icon field
				ntype->icon = fieldnode->getStringValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "editor")) {			// Editor field (to be removed)
				ntype->editor = fieldnode->getStringValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "section")) {		// Section field
				ntype->section = fieldnode->getStringValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "match_ext")) {		// Match Extension field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->match_extension.push_back(fieldnode->getStringValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), "match_name")) {		// Match Name field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->match_name.push_back(fieldnode->getStringValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), "size")) {			// Size field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->match_size.push_back(fieldnode->getIntValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), "min_size")) {		// Min Size field
				ntype->size_limit[0] = fieldnode->getIntValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "max_size")) {		// Max Size field
				ntype->size_limit[1] = fieldnode->getIntValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "size_multiple")) {	// Size Multiple field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->size_multiple.push_back(fieldnode->getIntValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), "reliability")) {	// Reliability field
				ntype->reliability = fieldnode->getIntValue();
			}
			else if (s_cmpnocase(fieldnode->getName(), "match_archive")) {		// Archive field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->match_archive.push_back(fieldnode->getStringValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), "extra")) {			// Extra properties
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->extra.addFlag(fieldnode->getStringValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), "category")) {		// Type category
				ntype->category = fieldnode->getStringValue();
			}
		}

		//ntype->dump();
		ntype->addToList();
	}

	return true;
}

/* EntryType::loadEntryTypes
 * Loads all built-in and custom user entry types
 *******************************************************************/
bool EntryType::loadEntryTypes() {
	// Setup unknown type
	etype_unknown.icon = "e_unknown";
	etype_unknown.detectable = false;
	etype_unknown.reliability = 0;
	etype_unknown.addToList();

	// Setup folder type
	etype_folder.icon = "e_folder";
	etype_folder.name = "Folder";
	etype_folder.detectable = false;
	etype_folder.addToList();

	// Setup marker type
	etype_marker.icon = "e_marker";
	etype_marker.name = "Marker";
	etype_marker.detectable = false;
	etype_marker.addToList();

	// Setup map marker type
	etype_map.icon = "e_map";
	etype_map.name = "Map Marker";
	etype_map.detectable = false;
	etype_map.addToList();

	// -------- READ BUILT-IN TYPES ---------

	// Get builtin entry types from resource archive
	Archive* res_archive = theArchiveManager->programResourceArchive();

	// Check resource archive exists
	if (!res_archive) {
		wxLogMessage("Error: No resource archive open!");
		return false;
	}

	// Get entry types directory
	ArchiveTreeNode* et_dir = res_archive->getDir("config/entry_types/");

	// Check it exists
	if (!et_dir) {
		wxLogMessage("Error: config/entry_types does not exist in slade.pk3");
		return false;
	}

	// Read in each file in the directory
	bool etypes_read = false;
	for (unsigned a = 0; a < et_dir->numEntries(); a++) {
		if (readEntryTypeDefinition(et_dir->getEntry(a)->getMCData()))
			etypes_read = true;
	}

	// Warn if no types were read (this shouldn't happen unless the resource archive is corrupted)
	if (!etypes_read)
		wxLogMessage("Warning: No built-in entry types could be loaded from slade.pk3");

	// -------- READ CUSTOM TYPES ---------

	// If the directory doesn't exist create it
	if (!wxDirExists(appPath("entry_types", DIR_USER)))
		wxMkdir(appPath("entry_types", DIR_USER));

	// Open the custom palettes directory
	wxDir res_dir;
	res_dir.Open(appPath("entry_types", DIR_USER));

	// Go through each file in the directory
	string filename = wxEmptyString;
	bool files = res_dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (files) {
		// Load file data
		MemChunk mc;
		mc.importFile(res_dir.GetName() + "/" + filename);

		// Parse file
		readEntryTypeDefinition(mc);

		// Next file
		files = res_dir.GetNext(&filename);
	}

	return true;
}

/* EntryType::detectEntryType
 * Attempts to detect the given entry's type
 *******************************************************************/
bool EntryType::detectEntryType(ArchiveEntry* entry) {
	// Do nothing if the entry is a folder or a map marker
	if (!entry || entry->getType() == &etype_folder || entry->getType() == &etype_map)
		return false;

	// If the entry's size is zero, set it to marker type
	if (entry->getSize() == 0) {
		entry->setType(&etype_marker);
		return true;
	}

	// Reset entry type
	entry->setType(&etype_unknown);

	// Go through all registered types
	for (size_t a = 0; a < entry_types.size(); a++) {
		// If the current type is more 'reliable' than this one, skip it
		if (entry->getType()->getReliability() >= entry_types[a]->getReliability())
			continue;

		// Check for possible type match
		if (entry_types[a]->isThisType(entry)) {
			// Type matches, set it
			entry->setType(entry_types[a]);

			// No need to continue if the type is 100% reliable
			if (entry_types[a]->getReliability() >= 255)
				return true;
		}
	}

	// Return t/f depending on if a matching type was found
	if (entry->getType() == &etype_unknown)
		return false;
	else
		return true;
}

/* EntryType::getType
 * Returns the entry type with the given id, or etype_unknown if no
 * id match is found
 *******************************************************************/
EntryType* EntryType::getType(string id) {
	for (size_t a = 0; a < entry_types.size(); a++) {
		if (entry_types[a]->getId() == id)
			return entry_types[a];
	}

	return &etype_unknown;
}

/* EntryType::unknownType
 * Returns the global 'unknown' entry type
 *******************************************************************/
EntryType* EntryType::unknownType() {
	return &etype_unknown;
}

/* EntryType::folderType
 * Returns the global 'folder' entry type
 *******************************************************************/
EntryType* EntryType::folderType() {
	return &etype_folder;
}

/* EntryType::mapMarkerType
 * Returns the global 'map marker' entry type
 *******************************************************************/
EntryType* EntryType::mapMarkerType() {
	return &etype_map;
}

wxArrayString EntryType::getIconList() {
	wxArrayString list;

	for (size_t a = 0; a < entry_types.size(); a++)
		list.Add(entry_types[a]->getIcon());

	return list;
}

void EntryType::cleanupEntryTypes() {
	for (size_t a = 3; a < entry_types.size(); a++) {
		EntryType* e = entry_types[a];
		if (e != &etype_unknown && e != &etype_folder && e != &etype_marker && e != &etype_map)
			delete entry_types[a];
	}
}

vector<EntryType*> EntryType::allTypes() {
	return entry_types;
}


/* Console Command - "test_entry_types"
 * Testingggg
 *******************************************************************/
CONSOLE_COMMAND (test_entry_types, 0) {
	EntryType::loadEntryTypes();
}


/* Console Command - "test_entry_type_detection"
 * Testingggg moar
 *******************************************************************/
CONSOLE_COMMAND (test_entry_type_detection, 0) {
	Archive* archive = theArchiveManager->getArchive(0);

	for (uint32_t a = 0; a < archive->numEntries(); a++) {
		EntryType::detectEntryType(archive->getEntry(a));
	}
}

CONSOLE_COMMAND (type, 0) {
	vector<EntryType*> all_types = EntryType::allTypes();
	if (args.size() == 0) {
		// List existing types and their IDs
		string listing = "List of entry types:\n\t";
		string separator = "]\n\t"; string colon = ": "; string paren = " [";
		for (size_t a = 3; a < all_types.size(); a++) {
			listing += all_types[a]->getName();
			listing += paren;
			listing += all_types[a]->getId();
			listing += colon;
			listing += all_types[a]->getFormat();
			listing += separator;
		}
		wxLogMessage(listing);
	} else {
		// Find type by id or first matching format
		bool match = false;
		size_t a;
		for (a = 3; a < all_types.size(); a++) {
			if (!args[0].CmpNoCase(all_types[a]->getFormat()) || !args[0].CmpNoCase(all_types[a]->getId())) {
				match = true;
				break;
			}
		}
		if (!match) {
			wxLogMessage("Type %s does not exist (use \"type\" without parameter for a list)", args[0].mb_str());
			return;
		}

		// Allow to force type change even if format checks fails (use at own risk!)
		bool okay = false, force = !(args.size() < 2 || args[1].CmpNoCase("force"));
		ArchiveEntry * meep = CH::getCurrentArchiveEntry();
		if (!meep) {
			wxLogMessage("No entry selected");
			return;
		}

		// Check if format corresponds to entry
		EntryDataFormat* foo = EntryDataFormat::getFormat(all_types[a]->getFormat());
		if (foo) {
			wxLogMessage("Identifying as %s", all_types[a]->getName().mb_str());
			okay = foo->isThisFormat(meep->getMCData());
			if (okay) wxLogMessage("Identification successful");
			else wxLogMessage("Identification failed");
		} else wxLogMessage("No data format for this type!");

		// Change type
		if (force || okay) {
			meep->setType(all_types[a]);
			wxLogMessage("Type changed.");
		}
	}
}

CONSOLE_COMMAND (size, 0) {
	ArchiveEntry * meep = CH::getCurrentArchiveEntry();
	if (!meep) {
		wxLogMessage("No entry selected");
		return;
	}
	wxLogMessage("%s: %i bytes", meep->getName().mb_str(), meep->getSize());
}
