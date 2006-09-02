// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << byteswap.cpp - Byteswapping stuff    >>
// << for endianness-related issues        >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "byteswap.h"
#include "main.h"

size_t swapped_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t retval = fread(ptr, size, nmemb, stream);

	// For whatever reason, MSVC doesn't like these functions
#ifdef __BIG_ENDIAN__
	if(size == 2)
	{
		WORD *shortPtr = (WORD *)ptr;
		for(int i = 0; i < nmemb; ++i)
			shortPtr[i] = swap16(shortPtr[i]);
	}
	else if(size == 4)
	{
		DWORD *longPtr = (DWORD *)ptr;
		for(int i = 0; i < nmemb; ++i)
			longPtr[i] = swap32(longPtr[i]);
	}
#endif

	return retval;
}


size_t swapped_fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t retval;

#ifdef __BIG_ENDIAN__
	if(size == 2)
	{
		WORD *origPtr = (WORD *)ptr;
		WORD *shortPtr = new WORD[nmemb];
		for(int i = 0; i < nmemb; ++i)
			shortPtr[i] = swap16(origPtr[i]);
		retval = fwrite(shortPtr, size, nmemb, stream);
		delete[] shortPtr;
	}
	else if(size == 4)
	{
		DWORD *origPtr = (DWORD *)ptr;
		DWORD *longPtr = new DWORD[nmemb];
		for(int i = 0; i < nmemb; ++i)
			longPtr[i] = swap32(origPtr[i]);
		retval = fwrite(longPtr, size, nmemb, stream);
		delete[] longPtr;
	}
	else
	{
		retval = fwrite(ptr, size, nmemb, stream);
	}
#endif

	return retval;
}

void *swapped_memcpy(void *dst, const void *src, size_t len)
{
	void *retval;

#ifdef __BIG_ENDIAN__
	if(len == 2)
	{
		WORD swapped = swap16(*(WORD *)src);
		retval = memcpy(dst, &swapped, 2);
	}
	else if(len == 4)
	{
		DWORD swapped = swap32(*(DWORD *)src);
		retval = memcpy(dst, &swapped, 4);
	}
	else
	{
		retval = memcpy(dst, src, len);
	}
#endif

	return retval;
}
