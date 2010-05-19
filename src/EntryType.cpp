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
#include <wx/dir.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/

///*
// Basic formats system for now
struct id_format_t {
	string 		id;
	uint16_t	format;
};
id_format_t formats[] = {
#define xa(id, name, val)			{ name,	id },
#define xb(id, name)				{ name,	id },
#define xx(id, name, func)			{ name,	id },
#define xy(id, name, func, load)	{ name,	id },
#define xz(id, name)				{ name,	id },
#include "EntryTypeList.h"

	{ "",				EDF_ANY }, // Dummy type to mark end of list
};
//*/

vector<EntryType*>	entry_types;	// The big list of all entry types

// Special entry types
EntryType			etype_unknown;	// The default, 'unknown' entry type
EntryType			etype_folder;	// Folder entry type
EntryType			etype_marker;	// Marker entry type
EntryType			etype_map;		// Map marker type


/*******************************************************************
 * ENTRYDATAFORMAT CLASS FUNCTIONS
 *******************************************************************/
 ///*
bool EntryDataFormat::isFormat(MemChunk& mc, uint16_t format) {
	switch (format) {
#define xa(id, name, val)
#define xb(id, name)
#define xx(id, name, func)			case id: return func(mc);
#define xy(id, name, func, load)	case id: return func(mc);
#include "EntryTypeList.h"
	default:
		return false;
	}
}
//*/

/*******************************************************************
 * ENTRYTYPE CLASS FUNCTIONS
 *******************************************************************/

/* EntryType::EntryType
 * EntryType class constructor
 *******************************************************************/
EntryType::EntryType(string id) {
	this->id = id;
	name = _T("Unknown");
	format = EDF_ANY;
	extension = _T("dat");
	icon = _T("e_default");
	size_limit[0] = -1;
	size_limit[1] = -1;
	editor = _T("default");
	detectable = true;
	section = _T("none");
	reliability = 255;
}

/* EntryType::~EntryType
 * EntryType class constructor
 *******************************************************************/
EntryType::~EntryType() {
	match_extension.clear();
	match_name.clear();
	match_size.clear();
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
	wxLogMessage(s_fmt(_T("Type %s \"%s\", format %d, extension %s"), chr(id), chr(name), format, chr(extension)));
	wxLogMessage(s_fmt(_T("Size limit: %d-%d"), size_limit[0], size_limit[1]));

	for (size_t a = 0; a < match_extension.size(); a++)
		wxLogMessage(s_fmt(_T("Match Extension: \"%s\""), chr(match_extension[a])));

	for (size_t a = 0; a < match_name.size(); a++)
		wxLogMessage(s_fmt(_T("Match Name: \"%s\""), chr(match_name[a])));

	for (size_t a = 0; a < match_size.size(); a++)
		wxLogMessage(s_fmt(_T("Match Size: %d"), match_size[a]));

	for (size_t a = 0; a < size_multiple.size(); a++)
		wxLogMessage(s_fmt(_T("Size Multiple: %d"), size_multiple[a]));

	wxLogMessage(_T("---"));
}

void EntryType::copyToType(EntryType* target) {
	// Copy type attributes
	target->setEditor(editor);
	target->setExtension(extension);
	target->setIcon(icon);
	target->setName(name);
	target->setReliability(reliability);

	// Copy type match criteria
	target->setFormat(format);
	target->setMaxSize(size_limit[1]);
	target->setMinSize(size_limit[0]);
	target->setSection(section);

	// Copy match names
	for (size_t a = 0; a < match_name.size(); a++)
		target->addMatchName(match_name[a]);

	// Copy match extensions
	for (size_t a = 0; a < match_extension.size(); a++)
		target->addMatchExtension(match_extension[a]);

	// Copy match sizes
	for (size_t a = 0; a < match_size.size(); a++)
		target->addMatchSize(match_size[a]);

	// Copy size multiples
	for (size_t a = 0; a < size_multiple.size(); a++)
		target->addSizeMultiple(size_multiple[a]);
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
	if (section != _T("none")) {
		// Check entry is part of an archive (if not it can't be in a section)
		if (!entry->getParent())
			return false;

		string e_section = entry->getParent()->detectEntrySection(entry);

		if (e_section != section)
			return false;
	}

	// Check for data format match if needed
	if (format == EDF_TEXT) {
		// Text is a special case, as other data formats can sometimes be detected as 'text',
		// we'll only check for it if text data is specified in the entry type
		if (memchr(entry->getData(), 0, entry->getSize()-1) != NULL)
			return false;
	}
	else if (format != EDF_ANY) {
		if (!EntryDataFormat::isFormat(entry->getMCData(), format))
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
	/*
	Tokenizer tz;

	// Open the given text data
	if (!tz.openMem((const char*)mc.getData(), mc.getSize())) {
		wxLogMessage(_T("Unable to open file"));
		return false;
	}

	// Get first token
	string token = tz.getToken();

	// If it's an entry_types definition, read it
	if (!token.Cmp(_T("entry_types"))) {
		// Check for opening brace
		if (!tz.checkToken(_T("{")))
			return false;

		// Parse all definitions until closing brace
		token = tz.getToken(); // Get type id
		while (token.Cmp(_T("}"))) {
			// Check if we reached the end of the file for some reason
			if (!token.Cmp(_T("")))
				return false;

			// Check for inherited type
			string inherit = _T("");
			if (tz.peekToken() == _T(":")) {
				// Skip :
				tz.getToken();

				// Read inherited type id
				inherit = tz.getToken();
			}

			// Check for opening brace
			if (!tz.checkToken(_T("{")))
				return false;

			// Begin parsing entry type
			EntryType* ntype = new EntryType(token);
			token = tz.getToken();

			// Copy from existing type if inherited
			if (!inherit.IsEmpty()) {
				EntryType* parent_type = EntryType::getType(inherit);

				if (parent_type != EntryType::unknownType())
					parent_type->copyToType(ntype);
				else
					wxLogMessage(_T("Warning: Entry type %s inherits from unknown type %s"), chr(ntype->getId()), chr(inherit));
			}

			// Read all fields until we get to the closing brace
			while (token.Cmp(_T("}"))) {
				// Name field
				if (!token.Cmp(_T("name"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string name = tz.getToken();	// Get name value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setName(name);			// Set type name
				}

				// Detectable field
				if (!token.Cmp(_T("detectable"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					bool detect = tz.getBool();		// Get detectable value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setDetectable(detect);	// Set detectable
				}

				// Extension field
				if (!token.Cmp(_T("export_ext"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string ext = tz.getToken();		// Get extension value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setExtension(ext);		// Set type extension
				}

				// Format field
				if (!token.Cmp(_T("format"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string format = tz.getToken();	// Get format value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					// Get format type matching format string
					bool fmt_exists = false;
					for (int a = 0; a < EDF_UNKNOWN; a++) {
						if (formats[a].id.IsEmpty())
							break;

						if (!format.Cmp(formats[a].id)) {
							ntype->setFormat(formats[a].format);
							fmt_exists = true;
						}
					}

					if (!fmt_exists)
						ntype->setFormat(EDF_UNKNOWN);
				}

				// Icon field
				if (!token.Cmp(_T("icon"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string icon = tz.getToken();	// Get format value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setIcon(icon);			// Set type icon
				}

				// Editor field
				if (!token.Cmp(_T("editor"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string editor = tz.getToken();	// Get name value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setEditor(editor);		// Set type editor
				}

				// Section field
				if (!token.Cmp(_T("section"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string section = tz.getToken();	// Get name value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setSection(section);		// Set type section
				}

				// MatchExtension field
				if (!token.Cmp(_T("match_ext"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						string ext = tz.getToken();	// Get extension value

						// Add the extension to the type
						ntype->addMatchExtension(ext);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// MatchName field
				if (!token.Cmp(_T("match_name"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						string name = tz.getToken();	// Get name value

						// Add the match name to the type
						ntype->addMatchName(name);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// Size field
				if (!token.Cmp(_T("size"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						int size = tz.getInteger();	// Get size value

						// Add the match size to the type
						ntype->addMatchSize(size);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// MinSize field
				if (!token.Cmp(_T("min_size"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					int size = tz.getInteger();		// Get size value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setMinSize(size);		// Set min size
				}

				// MaxSize field
				if (!token.Cmp(_T("max_size"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					int size = tz.getInteger();		// Get size value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setMaxSize(size);		// Set max size
				}

				// SizeMultiple field
				if (!token.Cmp(_T("size_multiple"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						int size = tz.getInteger();	// Get size value

						// Add the size multiple to the type
						ntype->addSizeMultiple(size);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// Reliability field
				if (s_cmp(token, _T("reliability"))) {
					if (!tz.checkToken(_T("=")))		// Check for =
						return false;

					int reliability = tz.getInteger();	// Get reliability value

					if (!tz.checkToken(_T(";")))		// Check for ;
						return false;

					ntype->setReliability(reliability);	// Set reliability
				}

				token = tz.getToken();
			}

			//if (!inherit.IsEmpty())
			//	wxLogMessage(_T("EntryType %s inherit from type %s"), ntype->getName().c_str(), inherit.c_str());

			//ntype->dump();
			ntype->addToList();
			token = tz.getToken();
		}
	}
	*/

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
		EntryType* ntype = new EntryType(typenode->getName());

		// Copy from existing type if inherited
		if (!typenode->getInherit().IsEmpty()) {
			EntryType* parent_type = EntryType::getType(typenode->getInherit());

			if (parent_type != EntryType::unknownType())
				parent_type->copyToType(ntype);
			else
				wxLogMessage(_T("Warning: Entry type %s inherits from unknown type %s"), chr(ntype->getId()), chr(typenode->getInherit()));
		}

		// Go through all parsed fields
		for (unsigned b = 0; b < typenode->nChildren(); b++) {
			// Get child as ParseTreeNode
			ParseTreeNode* fieldnode = (ParseTreeNode*)typenode->getChild(b);

			// Process it
			if (s_cmpnocase(fieldnode->getName(), _T("name"))) {			// Name field
				ntype->setName(fieldnode->getStringValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("detectable"))) {	// Detectable field
				ntype->setDetectable(fieldnode->getBoolValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("export_ext"))) {	// Export Extension field
				ntype->setExtension(fieldnode->getStringValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("format"))) {	// Format field
				// Get format type matching format string
				bool fmt_exists = false;
				for (unsigned f = 0; f < EDF_UNKNOWN; f++) {
					if (formats[f].id.IsEmpty())
						break;

					if (s_cmpnocase(fieldnode->getStringValue(), formats[f].id)) {
						ntype->setFormat(formats[f].format);
						fmt_exists = true;
					}
				}

				if (!fmt_exists)
					ntype->setFormat(EDF_UNKNOWN);
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("icon"))) {	// Icon field
				ntype->setIcon(fieldnode->getStringValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("editor"))) {	// Editor field (to be removed)
				ntype->setEditor(fieldnode->getStringValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("section"))) {	// Section field
				ntype->setSection(fieldnode->getStringValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("match_ext"))) {	// Match Extension field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->addMatchExtension(fieldnode->getStringValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("match_name"))) {	// Match Name field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->addMatchName(fieldnode->getStringValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("size"))) {	// Size field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->addMatchSize(fieldnode->getIntValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("min_size"))) {	// Min Size field
				ntype->setMinSize(fieldnode->getIntValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("max_size"))) {	// Max Size field
				ntype->setMaxSize(fieldnode->getIntValue());
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("size_multiple"))) {	// Size Multiple field
				for (unsigned v = 0; v < fieldnode->nValues(); v++)
					ntype->addSizeMultiple(fieldnode->getIntValue(v));
			}
			else if (s_cmpnocase(fieldnode->getName(), _T("reliability"))) {	// Reliability field
				ntype->setReliability(fieldnode->getIntValue());
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
	etype_unknown.setIcon(_T("e_unknown"));
	etype_unknown.setDetectable(false);
	etype_unknown.setReliability(0);
	etype_unknown.addToList();

	// Setup folder type
	etype_folder.setIcon(_T("e_folder"));
	etype_folder.setName(_T("Folder"));
	etype_folder.setDetectable(false);
	etype_folder.addToList();

	// Setup marker type
	etype_marker.setIcon(_T("e_marker"));
	etype_marker.setName(_T("Marker"));
	etype_marker.setDetectable(false);
	etype_marker.addToList();

	// Setup map marker type
	etype_map.setIcon(_T("e_map"));
	etype_map.setName(_T("Map Marker"));
	etype_map.setDetectable(false);
	etype_map.addToList();

	// -------- READ BUILT-IN TYPES ---------

	// Get builtin entry types from resource archive
	Archive* res_archive = theArchiveManager->programResourceArchive();

	// Check resource archive exists
	if (!res_archive) {
		wxLogMessage(_T("Error: No resource archive open!"));
		return false;
	}

	// Get entry types directory
	zipdir_t* et_dir = ((ZipArchive*)res_archive)->getDirectory(_T("config/entry_types"));

	// Check it exists
	if (!et_dir) {
		wxLogMessage(_T("Error: config/entry_types does not exist in slade.pk3"));
		return false;
	}

	// Read in each file in the directory
	bool etypes_read = false;
	for (unsigned a = 0; a < et_dir->numEntries(); a++) {
		if (readEntryTypeDefinition(et_dir->entries[a]->getMCData()))
			etypes_read = true;
	}

	// Warn if no types were read (this shouldn't happen unless the resource archive is corrupted)
	if (!etypes_read)
		wxLogMessage(_T("Warning: No built-in entry types could be loaded from slade.pk3"));

	// -------- READ CUSTOM TYPES ---------

	// If the directory doesn't exist create it
	if (!wxDirExists(appPath(_T("entry_types"), DIR_USER)))
		wxMkdir(appPath(_T("entry_types"), DIR_USER));

	// Open the custom palettes directory
	wxDir res_dir;
	res_dir.Open(appPath(_T("entry_types"), DIR_USER));

	// Go through each file in the directory
	string filename = wxEmptyString;
	bool files = res_dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (files) {
		// Load file data
		MemChunk mc;
		mc.importFile(res_dir.GetName() + _T("/") + filename);

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
	if (entry->getType() == &etype_folder || entry->getType() == &etype_map)
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
		// Check for possible type match
		if (entry_types[a]->isThisType(entry)) {
			// Type matches, test against currently detected type's reliability
			if (entry->getType()->getReliability() < entry_types[a]->getReliability()) {
				// This type is more reliable, so set it
				entry->setType(entry_types[a]);

				// No need to continue if the type is 100% reliable
				if (entry_types[a]->getReliability() == 255)
					return true;
			}
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


/* Console Command - "test_entry_types"
 * Testingggg
 *******************************************************************/
void c_test_entry_types(vector<string> args) {
	EntryType::loadEntryTypes();
}
ConsoleCommand et_test_entry_types(_T("test_entry_types"), &c_test_entry_types, 0);


/* Console Command - "test_entry_type_detection"
 * Testingggg moar
 *******************************************************************/
void c_test_entry_type_detection(vector<string> args) {
	Archive* archive = theArchiveManager->getArchive(0);

	for (uint32_t a = 0; a < archive->numEntries(); a++) {
		EntryType::detectEntryType(archive->getEntry(a));
	}
}
ConsoleCommand et_test_entry_type_detection(_T("test_entry_type_detection"), &c_test_entry_type_detection, 0);
