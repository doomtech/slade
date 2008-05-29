
#ifndef __MEMCHUNK_H__
#define __MEMCHUNK_H__

class MemChunk
{
private:
	BYTE*	data;
	DWORD	size;

public:
	MemChunk(DWORD size = 0);
	~MemChunk();

	// Accessors
	BYTE*	getData() { return data; }
	DWORD	getSize() { return size; }

	bool	hasData();

	bool	clear();
	bool	reSize(DWORD new_size, bool preserve_data = true);

	bool	loadFile(string filename, DWORD offset = 0, DWORD len = 0);
	bool	loadFileStream(FILE* fp, DWORD len = 0);
	bool	loadMem(BYTE* start, DWORD len);
};

#endif //__MEMCHUNK_H__