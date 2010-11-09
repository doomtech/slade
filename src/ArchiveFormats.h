
#ifndef ARCHIVEFORMATS_H
#define ARCHIVEFORMATS_H

class WadDataFormat : public EntryDataFormat {
public:
	WadDataFormat() : EntryDataFormat("archive_wad") {};
	~WadDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return WadArchive::isWadArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class ZipDataFormat : public EntryDataFormat {
public:
	ZipDataFormat() : EntryDataFormat("archive_zip") {};
	~ZipDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return ZipArchive::isZipArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class LibDataFormat : public EntryDataFormat {
public:
	LibDataFormat() : EntryDataFormat("archive_lib") {};
	~LibDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return LibArchive::isLibArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class DatDataFormat : public EntryDataFormat {
public:
	DatDataFormat() : EntryDataFormat("archive_dat") {};
	~DatDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return DatArchive::isDatArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class ResDataFormat : public EntryDataFormat {
public:
	ResDataFormat() : EntryDataFormat("archive_res") {};
	~ResDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return ResArchive::isResArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class PakDataFormat : public EntryDataFormat {
public:
	PakDataFormat() : EntryDataFormat("archive_pak") {};
	~PakDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return PakArchive::isPakArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class Wad2DataFormat : public EntryDataFormat {
public:
	Wad2DataFormat() : EntryDataFormat("archive_wad2") {}
	~Wad2DataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return Wad2Archive::isWad2Archive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class WadJDataFormat : public EntryDataFormat {
public:
	WadJDataFormat() : EntryDataFormat("archive_wadj") {}
	~WadJDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return WadJArchive::isWadJArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class GrpDataFormat : public EntryDataFormat {
public:
	GrpDataFormat() : EntryDataFormat("archive_grp") {};
	~GrpDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return GrpArchive::isGrpArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class RffDataFormat : public EntryDataFormat {
public:
	RffDataFormat() : EntryDataFormat("archive_rff") {};
	~RffDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return RffArchive::isRffArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

class WolfDataFormat : public EntryDataFormat {
public:
	WolfDataFormat() : EntryDataFormat("archive_wolf") {};
	~WolfDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return WolfArchive::isWolfArchive(mc) ? EDF_TRUE : EDF_FALSE;
	}
};

#endif //ARCHIVEFORMATS_H
