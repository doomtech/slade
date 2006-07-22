#include <stdio.h>

#ifdef __BIG_ENDIAN__

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#define swap32(x) OSSwapInt32(x)
#define swap16(x) OSSwapInt16(x)
#else
#error Define efficient byteswaps please
#endif

#define lefread(ptr, size, nmemb, stream) swapped_fread(ptr, size, nmemb, stream)
#define lefwrite(ptr, size, nmemb, stream) swapped_fwrite(ptr, size, nmemb, stream)
#define lememcpy(dst, src, len) swapped_memcpy(dst, src, len)

#else

#define swap32(x)
#define swap16(x)

#define lefread(ptr, size, nmemb, stream) fread(ptr, size, nmemb, stream)
#define lefwrite(ptr, size, nmemb, stream) fwrite(ptr, size, nmemb, stream)
#define lememcpy(dst, src, len) memcpy(dst, src, len)

#endif

size_t swapped_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t swapped_fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
void *swapped_memcpy(void *dst, const void *src, size_t len);
