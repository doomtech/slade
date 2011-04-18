
#ifndef __SIFORMAT_H__
#define __SIFORMAT_H__

#include "SImage.h"

struct simginfo_t {
	int		width;
	int		height;
	int		colformat;
	string	format;
};

class SIFormat {
protected:
	string	id;
	string	name;
	string	extension;

	uint8_t* imageData(SImage& image) { return image.data; }
	uint8_t* imageMask(SImage& image) { return image.mask; }

public:
	SIFormat(string id);
	~SIFormat();

	virtual bool	isThisFormat(MemChunk& mc) { return false; }

	// Reading
	virtual simginfo_t	getInfo(MemChunk& mc) { return simginfo_t(); }
	virtual bool		loadImage(SImage& image, MemChunk& data) { return false; }

	// Writing
	virtual bool	canWrite(SImage& image) { return false; }
	virtual bool	writeImage(SImage& image, MemChunk& data) { return false; }


	static void			initFormats();
	static SIFormat*	getFormat(string name);
};

#endif//__SIFORMAT_H__
