
class MemLump
{
private:
	BYTE*	data;
	DWORD	size;

public:
	MemLump();
	MemLump(DWORD isize, BYTE* idata = NULL);
	~MemLump();

	BYTE*	get_data() { return data; }
	DWORD	get_size() { return size; }

	bool has_data() { return !!data; }

	bool allocate(DWORD asize, bool clear = true);
	void deallocate();
	void clear();
	void dump_to_file(string filename);
};
