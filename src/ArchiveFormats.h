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

#endif //ARCHIVEFORMATS_H