
#include "main.h"
#include "memlump.h"

MemLump::MemLump()
{
	data = NULL;
	size = 0;
}

MemLump::MemLump(DWORD isize, BYTE* idata)
{
	data = NULL;
	size = 0;

	if (!allocate(isize))
		return;

	if (idata)
		memcpy(data, idata, size);
}

MemLump::~MemLump()
{
	if (data)
		delete[] data;
}

bool MemLump::allocate(DWORD asize, bool clear)
{
	if (asize < 0)
		return false;

	if (asize == 0)
	{
		deallocate();
		return true;
	}

	size = asize;

	if (data)
	{
		delete[] data;
		data = NULL;
	}

	data = new BYTE[size];

	if (clear)
		this->clear();

	return true;
}

void MemLump::deallocate()
{
	size = 0;

	if (data)
		delete[] data;
}

void MemLump::clear()
{
	if (data)
		memset(data, 0, size);
}

void MemLump::dump_to_file(string filename)
{
	if (!has_data())
		return;

	FILE* fp = fopen(chr(filename), "wb");
	fwrite(data, size, 1, fp);
	fclose(fp);
}
