
#ifndef __MEMCHUNK_H__
#define __MEMCHUNK_H__

class MemChunk {
protected:
	uint8_t*	data;
	uint32_t	cur_ptr;
	uint32_t	size;
	bool		allow_write;

public:
	MemChunk(uint32_t size = 0);
	MemChunk(const uint8_t* data, uint32_t size);
	~MemChunk();

	uint8_t& operator[](int a) { return data[a]; }

	// Accessors
	const uint8_t*	getData() { return data; }
	uint32_t		getSize() { return size; }
	//bool			writeAllowed() { return allow_write; }
	//void			writeAllowed(bool allow) { allow_write = allow; }

	bool hasData();

	bool clear();
	bool reSize(uint32_t new_size, bool preserve_data = true);

	// Data import
	bool	importFile(string filename, uint32_t offset = 0, uint32_t len = 0);
	bool	importFileStream(FILE* fp, uint32_t len = 0);
	bool	importMem(const uint8_t* start, uint32_t len);

	// Data export
	bool exportFile(string filename);

	// C-style reading/writing
	bool		write(const void* data, uint32_t size);
	bool		read(void* buf, uint32_t size);
	bool		seek(uint32_t offset, uint32_t start);
	uint32_t	currentPos() { return cur_ptr; }
};

#endif //__MEMCHUNK_H__
