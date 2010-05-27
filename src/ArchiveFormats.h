#ifndef ARCHIVEFORMATS_H
#define ARCHIVEFORMATS_H

class WadDataFormat : public EntryDataFormat {
public:
	WadDataFormat() : EntryDataFormat("archive_wad") {};
	~WadDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return WadArchive::isWadArchive(mc);
	}
};

class ZipDataFormat : public EntryDataFormat {
public:
	ZipDataFormat() : EntryDataFormat("archive_zip") {};
	~ZipDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return ZipArchive::isZipArchive(mc);
	}
};

class LibDataFormat : public EntryDataFormat {
public:
	LibDataFormat() : EntryDataFormat("archive_lib") {};
	~LibDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return LibArchive::isLibArchive(mc);
	}
};

class DatDataFormat : public EntryDataFormat {
public:
	DatDataFormat() : EntryDataFormat("archive_dat") {};
	~DatDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return DatArchive::isDatArchive(mc);
	}
};

#endif //ARCHIVEFORMATS_H