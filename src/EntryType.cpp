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
#include <wx/dir.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/

// Basic formats system for now
struct id_format_t {
	string 		id;
	uint16_t	format;
};
id_format_t formats[] = {
	{ "any", EDF_ANY },
	{ "png", EDF_PNG },
	{ "bmp", EDF_BMP },
	{ "jpeg", EDF_JPEG },
	{ "gfx_doom", EDF_GFX_DOOM },
	{ "gfx_flat", EDF_GFX_FLAT },
	{ "gfx_doom_alpha", EDF_GFX_DOOM_ALPHA },
	{ "gfx_doom_beta", EDF_GFX_DOOM_BETA },
	{ "wad", EDF_WAD },
	{ "mus", EDF_MUS },
	{ "midi", EDF_MIDI },
	{ "mod_it", EDF_MOD_IT },
	{ "mod_xm", EDF_MOD_XM },
	{ "mod_s3m", EDF_MOD_S3M },
	{ "mod_mod", EDF_MOD_MOD },
	{ "snd_doom", EDF_SND_DOOM },
	{ "snd_wav", EDF_SND_WAV },
	{ "text", EDF_TEXT },
};

vector<EntryType*>	entry_types;	// The big list of all entry types

// Special entry types
EntryType			etype_unknown;	// The default, 'unknown' entry type
EntryType			etype_folder;	// Folder entry type
EntryType			etype_map;		// Map marker type


/*******************************************************************
 * ENTRYDATAFORMAT CLASS FUNCTIONS
 *******************************************************************/
uint16_t EntryDataFormat::detectFormat(MemChunk& mc) {
	// Default to unknown
	uint16_t type = EDF_UNKNOWN;

	// Detect PNG
	if (detectPng(mc))
		return EDF_PNG;

	// Detect BMP
	if (detectBmp(mc))
		return EDF_BMP;

	// JPEG
	if (detectJpeg(mc))
		return EDF_JPEG;

	// MUS
	if (detectMus(mc))
		return EDF_MUS;

	// MIDI
	if (detectMidi(mc))
		return EDF_MIDI;

	// IT Module
	if (detectModIt(mc))
		return EDF_MOD_IT;

	// S3M Module
	if (detectModS3m(mc))
		return EDF_MOD_S3M;

	// WAV Sound
	if (detectSndWav(mc))
		return EDF_SND_WAV;

	// XM Module
	if (detectModXm(mc))
		return EDF_MOD_XM;

	// MOD Module
	if (detectModMod(mc))
		return EDF_MOD_MOD;

	// Detect doom patch format
	if (detectDoomGfx(mc))
		return EDF_GFX_DOOM;

	// Doom Sound
	if (detectSndDoom(mc))
		return EDF_SND_DOOM;

	// Detect doom wad
	if (detectWad(mc))
		return EDF_WAD;

	// Detect doom flat format
	if (detectDoomFlat(mc))
		return EDF_GFX_FLAT;

	return type;
}

bool EntryDataFormat::isFormat(MemChunk& mc, uint16_t format) {
	switch (format) {
	case EDF_BMP:
		return detectBmp(mc);
	case EDF_GFX_DOOM:
		return detectDoomGfx(mc);
	case EDF_GFX_DOOM_ALPHA:
		return detectDoomGfxAlpha(mc);
	case EDF_GFX_DOOM_BETA:
		return detectDoomGfxBeta(mc);
	case EDF_GFX_FLAT:
		return detectDoomFlat(mc);
	case EDF_JPEG:
		return detectJpeg(mc);
	case EDF_MIDI:
		return detectMidi(mc);
	case EDF_MOD_IT:
		return detectModIt(mc);
	case EDF_MOD_MOD:
		return detectModMod(mc);
	case EDF_MOD_S3M:
		return detectModS3m(mc);
	case EDF_MOD_XM:
		return detectModXm(mc);
	case EDF_MUS:
		return detectMus(mc);
	case EDF_PNG:
		return detectPng(mc);
	case EDF_SND_DOOM:
		return detectSndDoom(mc);
	case EDF_SND_WAV:
		return detectSndWav(mc);
	case EDF_WAD:
		return detectWad(mc);
	default:
		return false;
	}
	
	return false;
}

bool EntryDataFormat::detectPng(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 8) {
		// Check for PNG header
		if (mc[0] == 137 && mc[1] == 80 &&
		        mc[2] == 78 && mc[3] == 71 &&
		        mc[4] == 13 && mc[5] == 10 &&
		        mc[6] == 26 && mc[7] == 10)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectBmp(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 14) {
		// Check for BMP header
		if (mc[0] == 'B' && mc[1] == 'M' && mc[6] == 0 && mc[7] == 0 && mc[8] == 0 && mc[9] == 0)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectJpeg(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 128) {
		// Check for JPEG header
		if ((mc[6] == 'J' && mc[7] == 'F' && mc[8] == 'I' && mc[9] == 'F') ||
		        (mc[6] == 'E' && mc[7] == 'x' && mc[8] == 'i' && mc[9] == 'f')) {
			if (mc[0] == 255 && mc[1] == 216 && mc[2] == 255) {
				return true;
			}
		}
	}

	return false;
}

bool EntryDataFormat::detectDoomGfx(MemChunk& mc) {
	const uint8_t* data = mc.getData();

	// Check size
	if (mc.getSize() > sizeof(patch_header_t)) {
		const patch_header_t *header = (const patch_header_t *)data;

		// Check header values are 'sane'
		if (header->height > 0 && header->height < 4096 &&
		        header->width > 0 && header->width < 4096 &&
		        header->top > -2000 && header->top < 2000 &&
		        header->left > -2000 && header->left < 2000) {
			uint32_t *col_offsets = (uint32_t *)((const uint8_t *)data + sizeof(patch_header_t));

			// Check there is room for needed column pointers
			if (mc.getSize() < sizeof(patch_header_t) + (header->width * sizeof(uint32_t)))
				return false;

			// Check column pointers are within range
			for (int a = 0; a < header->width; a++) {
				if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(patch_header_t))
					return false;
			}

			// Passed all checks, so probably is doom gfx
			return true;
		}
	}

	return false;
}

bool EntryDataFormat::detectDoomGfxAlpha(MemChunk& mc) {
	// Disabled for now since it's too easy for non-gfx entries to be
	// detected as this format
	return false;
	
	const uint8_t* data = mc.getData();

	// Check size
	if (mc.getSize() > sizeof(oldpatch_header_t)) {
		const oldpatch_header_t *header = (const oldpatch_header_t *)data;

		// Check header values are 'sane'
		if (true/*header->width > 0 && header->height > 0*/) {
			uint16_t *col_offsets = (uint16_t *)((const uint8_t *)data + sizeof(oldpatch_header_t));

			// Check there is room for needed column pointers
			if (mc.getSize() < sizeof(oldpatch_header_t) + (header->width * sizeof(uint16_t)))
				return false;

			// Check column pointers are within range
			for (int a = 0; a < header->width; a++) {
				if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(oldpatch_header_t))
					return false;
			}

			// Passed all checks, so probably is doom gfx
			return true;
		}
	}

	return false;
}

bool EntryDataFormat::detectDoomGfxBeta(MemChunk& mc) {
	const uint8_t* data = mc.getData();

	// Check size
	if (mc.getSize() > sizeof(patch_header_t)) {
		const patch_header_t *header = (const patch_header_t *)data;

		// Check header values are 'sane'
		if (header->height > 0 && header->height < 4096 &&
		        header->width > 0 && header->width < 4096 &&
		        header->top > -2000 && header->top < 2000 &&
		        header->left > -2000 && header->left < 2000) {
			uint16_t *col_offsets = (uint16_t *)((const uint8_t *)data + sizeof(patch_header_t));

			// Check there is room for needed column pointers
			if (mc.getSize() < sizeof(patch_header_t) + (header->width * sizeof(uint16_t)))
				return false;

			// Check column pointers are within range
			for (int a = 0; a < header->width; a++) {
				if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(patch_header_t))
					return false;
			}

			// Passed all checks, so probably is doom gfx
			return true;
		}
	}

	return false;
}

bool EntryDataFormat::detectDoomFlat(MemChunk& mc) {
	// Not too sure how I should go about detecting this - just checking size as
	// it is now seems too cheap, but checking both size and position in the
	// archive could make things confusing for the user. Blah.
	uint32_t size = mc.getSize();

	if (size == 4096 ||
		size == 8192 ||
		size == 16384 ||
		size == 65536 ||
		size == 64000)
		return true;
	else
		return false;
}

bool EntryDataFormat::detectWad(MemChunk& mc) {
	// Check size
	if (mc.getSize() < 12)
		return false;

	// Check for IWAD/PWAD header
	if (!(mc[1] == 'W' && mc[2] == 'A' && mc[3] == 'D' &&
	        (mc[0] == 'P' || mc[0] == 'I')))
		return false;

	// Get number of lumps and directory offset
	uint32_t num_lumps = 0;
	uint32_t dir_offset = 0;
	mc.seek(4, SEEK_SET);
	mc.read(&num_lumps, 4);
	mc.read(&dir_offset, 4);

	// Reset MemChunk (just in case)
	mc.seek(0, SEEK_SET);

	// Byteswap values for big endian if needed
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check directory offset is decent
	if ((dir_offset + (num_lumps * 16)) > mc.getSize() ||
	        dir_offset < 12)
		return false;

	// If it's passed to here it's probably a wad file
	return true;
}

bool EntryDataFormat::detectMus(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 16) {
		// Check for MUS header
		if (mc[0] == 'M' && mc[1] == 'U' && mc[2] == 'S' && mc[3] == 0x1A)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectMidi(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 16) {
		// Check for MIDI header
		if (mc[0] == 'M' && mc[1] == 'T' && mc[2] == 'h' && mc[3] == 'd')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectModIt(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 32) {
		// Check for IT header
		if (mc[0] == 'I' && mc[1] == 'M' && mc[2] == 'P' && mc[3] == 'M')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectModXm(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 80) {
		// Check for mod header
		char temp[17] = "";
		memcpy(temp, mc.getData(), 17);
		if (!s_fmt(_T("%s"), temp).Cmp(_T("Extended module: "))) {
			if (mc[37] == 0x1a) {
				return true;
			}
		}
	}

	return false;
}

bool EntryDataFormat::detectModS3m(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 60) {
		// Check for s3m header
		if (mc[44] == 'S' && mc[45] == 'C' && mc[46] == 'R' && mc[47] == 'M')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectModMod(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 1084) {
		// Check format
		if (mc[950] >= 1 && mc[950] <= 128 && mc[951] == 127) {
			if ((mc[1080] == 'M' && mc[1081] == '.' && mc[1082] == 'K' && mc[1083] == '.') ||
			        (mc[1080] == 'M' && mc[1081] == '!' && mc[1082] == 'K' && mc[1083] == '!') ||
			        (mc[1080] == 'F' && mc[1081] == 'L' && mc[1082] == 'T' && mc[1083] == '4') ||
			        (mc[1080] == 'F' && mc[1081] == 'L' && mc[1082] == 'T' && mc[1083] == '8') ||
			        (mc[1081] == 'C' && mc[1082] == 'H' && mc[1083] == 'N')) {
				return true;
			}
		}
	}

	return false;
}

bool EntryDataFormat::detectSndDoom(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 8) {
		// Check header
		uint16_t head, samplerate, samples, tail;
		mc.read(&head, 2);
		mc.read(&samplerate, 2);
		mc.read(&samples, 2);
		mc.read(&tail, 2);

		if (head == 3 && tail == 0 && samples == mc.getSize() - 8)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectSndWav(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 8) {
		// Check for wav header
		if (mc[0] == 'R' && mc[1] == 'I' && mc[2] == 'F' && mc[3] == 'F')
			return true;
	}

	return false;
}


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
}

/* EntryType::~EntryType
 * EntryType class constructor
 *******************************************************************/
EntryType::~EntryType() {
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
	if (size_limit[0] >= 0 && entry->getSize() < size_limit[0])
		return false;

	// Check max size
	if (size_limit[1] >= 0 && entry->getSize() > size_limit[1])
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
		bool match = false;
		for (size_t a = 0; a < match_name.size(); a++) {
			if (!fn.GetName().CmpNoCase(match_name[a])) {
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

				token = tz.getToken();
			}

			//if (!inherit.IsEmpty())
			//	wxLogMessage(_T("EntryType %s inherit from type %s"), ntype->getName().c_str(), inherit.c_str());

			//ntype->dump();
			ntype->addToList();
			token = tz.getToken();
		}
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
	etype_unknown.addToList();
	
	// Setup folder type
	etype_folder.setIcon(_T("e_folder"));
	etype_folder.setName(_T("Folder"));
	etype_folder.setDetectable(false);
	etype_folder.addToList();
	
	// Setup map marker type
	etype_map.setIcon(_T("e_map"));
	etype_map.setName(_T("Map Marker"));
	etype_map.setDetectable(false);
	etype_map.addToList();

	// -------- READ BUILT-IN TYPES ---------

	// Get builtin entry types from resource archive
	Archive* res_archive = theArchiveManager->resourceArchive();

	// Check resource archive exists
	if (!res_archive) {
		wxLogMessage(_T("Error: No resource archive open!"));
		return false;
	}

	// Get entry_types.txt
	ArchiveEntry* et_entry = res_archive->getEntry(_T("config/entry_types.txt"));

	// Check it exists
	if (!et_entry) {
		wxLogMessage(_T("Error: config/entry_types.txt not found in slade.pk3!"));
		return false;
	}

	// Read in entry types definition
	if (!readEntryTypeDefinition(et_entry->getMCData()))
		return false;

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

	// Go through all registered types
	for (size_t a = 0; a < entry_types.size(); a++) {
		if (entry_types[a]->isThisType(entry)) {
			entry->setType(entry_types[a]);
			return true;
		}
	}

	// No matching type found, set to unknown
	entry->setType(&etype_unknown);

	return false;
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
