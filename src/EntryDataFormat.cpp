/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryDataFormat.cpp
 * Description: Entry data format detection system
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
#include "EntryDataFormat.h"
#include "ZipArchive.h"
#include "WadArchive.h"
#include "LibArchive.h"
#include "DatArchive.h"
#include "ResArchive.h"
#include "PakArchive.h"
#include "Wad2Archive.h"
#include "WadJArchive.h"
#include "BinaryControlLump.h"
#include "Parser.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
// Declare hash map class to hold EntryDataFormats
WX_DECLARE_STRING_HASH_MAP(EntryDataFormat*, EDFMap);
EDFMap data_formats;
EntryDataFormat*	edf_any = 0;


/*******************************************************************
 * ENTRYDATAFORMAT CLASS FUNCTIONS
 *******************************************************************/

EntryDataFormat::EntryDataFormat(string id) {
	// Init variables
	size_min = 0;

	// Add to hash map
	data_formats[id] = this;
}

EntryDataFormat::~EntryDataFormat() {
}

int EntryDataFormat::isThisFormat(MemChunk& mc) {
	return EDF_TRUE;
}

void EntryDataFormat::copyToFormat(EntryDataFormat& target) {
	target.patterns = patterns;
	target.size_min = size_min;
}



EntryDataFormat* EntryDataFormat::getFormat(string id) {
	EDFMap::iterator i = data_formats.find(id);
	if (i == data_formats.end())
		return edf_any;
	else
		return i->second;
}

EntryDataFormat* EntryDataFormat::anyFormat() {
	return edf_any;
}

bool EntryDataFormat::readDataFormatDefinition(MemChunk& mc) {
	// Parse the definition
	Parser p;
	p.parseText(mc);

	// Get data_formats tree
	ParseTreeNode* pt_formats = (ParseTreeNode*)(p.parseTreeRoot()->getChild("data_formats"));

	// Check it exists
	if (!pt_formats)
		return false;

	// Go through all parsed types
	for (unsigned a = 0; a < pt_formats->nChildren(); a++) {
		// Get child as ParseTreeNode
		ParseTreeNode* formatnode = (ParseTreeNode*)pt_formats->getChild(a);

		// Create+add new data format
		EntryDataFormat* edf = new EntryDataFormat(formatnode->getName().Lower());

		// Copy from existing type if inherited
		if (!formatnode->getInherit().IsEmpty()) {
			/*
			EntryType* parent_type = EntryType::getType(formatnode->getInherit());

			if (parent_type != EntryType::unknownType())
				parent_type->copyToType(ntype);
			else
				wxLogMessage("Warning: Entry type %s inherits from unknown type %s", chr(ntype->getId()), chr(typenode->getInherit()));
			*/
		}
	}
	return true;
}

// Special format that always returns false on detection
// Used when a format is requested that doesn't exist
class AnyDataFormat : public EntryDataFormat {
public:
	AnyDataFormat() : EntryDataFormat("any") {}
	~AnyDataFormat() {}

	int isThisFormat(MemChunk& mc) { return EDF_FALSE; }
};

// Format enumeration moved to separate files
#include "ArchiveFormats.h"
#include "AudioFormats.h"
#include "ImageFormats.h"
#include "LumpFormats.h"
#include "MiscFormats.h"
#include "ModelFormats.h"

void EntryDataFormat::initBuiltinFormats() {
	// Create the 'any' format
	edf_any = new AnyDataFormat();

	// Just need to create an instance of each builtin format class
	new PNGDataFormat();
	new BMPDataFormat();
	new GIFDataFormat();
	new PCXDataFormat();
	new TGADataFormat();
	new TIFFDataFormat();
	new JPEGDataFormat();
	new DoomGfxDataFormat();
	new DoomGfxAlphaDataFormat();
	new DoomGfxBetaDataFormat();
	new DoomSneaDataFormat();
	new DoomArahDataFormat();
	new ShadowCasterSpriteFormat();
	new ShadowCasterWallFormat();
	new AnaMipImageFormat();
	new IMGZDataFormat();
	new LegacyGfxDataFormat();
	new WadDataFormat();
	new ZipDataFormat();
	new LibDataFormat();
	new DatDataFormat();
	new ResDataFormat();
	new PakDataFormat();
	new Wad2DataFormat();
	new WadJDataFormat();
	new MUSDataFormat();
	new MIDIDataFormat();
	new XMIDataFormat();
	new HMIDataFormat();
	new HMPDataFormat();
	new ITModuleDataFormat();
	new XMModuleDataFormat();
	new S3MModuleDataFormat();
	new MODModuleDataFormat();
	new DoomSoundDataFormat();
	new DoomPCSpeakerDataFormat();
	new VocDataFormat();
	new WAVDataFormat();
	new OggDataFormat();
	new FLACDataFormat();
	new TextureXDataFormat();
	new PNamesDataFormat();
	new BoomAnimatedDataFormat();
	new BoomSwitchesDataFormat();
	new Font0DataFormat();
	new Font1DataFormat();
	new Font2DataFormat();
	new BMFontDataFormat();
	new ZNodesDataFormat();
	new ZGLNodesDataFormat();
	new ZGLNodes2DataFormat();
	new XNodesDataFormat();
	new XGLNodesDataFormat();
	new XGLNodes2DataFormat();
	new DMDModelDataFormat();
	new MDLModelDataFormat();
	new MD2ModelDataFormat();
	new MD3ModelDataFormat();
	new RLE0DataFormat();

	// And here are some dummy formats needed for certain image formats
	// that can't be detected by anything but size (which is done in EntryType detection anyway)
	new EntryDataFormat("img_raw");
	new EntryDataFormat("img_planar");
	new EntryDataFormat("img_4bitchunk");
	new EntryDataFormat("font_mono");

	// Another dummy for the generic text format
	new EntryDataFormat("text");
}

