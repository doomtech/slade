
#ifndef __MEMCHUNK_H__
#define __MEMCHUNK_H__

class MemChunk {
private:
	uint8_t* data;
	uint32_t cur_ptr;
	uint32_t size;

public:
	MemChunk(uint32_t size = 0);
	~MemChunk();

	// Accessors
	uint8_t* getData() { return data; }
	uint32_t getSize() { return size; }

	bool hasData();

	bool clear();
	bool reSize(uint32_t new_size, bool preserve_data = true);

	// Data import
	bool loadFile(string filename, uint32_t offset = 0, uint32_t len = 0);
	bool loadFileStream(FILE* fp, uint32_t len = 0);
	bool loadMem(uint8_t* start, uint32_t len);

	// C-style reading/writing
	bool		write(void* data, uint32_t size);
	bool		read(void* buf, uint32_t size);
	bool		seek(uint32_t offset, uint32_t start);
	uint32_t	currentPos() { return cur_ptr; }
};

#endif //__MEMCHUNK_H__
