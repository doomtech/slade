
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

	// Stuff to access protected image data
	uint8_t*		imageData(SImage& image) { return image.data; }
	uint8_t*		imageMask(SImage& image) { return image.mask; }
	Palette8bit&	imagePalette(SImage& image) { return image.palette; }
	
	virtual bool	readImage(SImage& image, MemChunk& data) { return false; }
	virtual bool	writeImage(SImage& image, MemChunk& data, Palette8bit* pal) { return false; }

public:
	SIFormat(string id);
	~SIFormat();

	virtual bool	isThisFormat(MemChunk& mc) { return false; }

	// Reading
	virtual simginfo_t	getInfo(MemChunk& mc) { return simginfo_t(); }
	
	bool loadImage(SImage& image, MemChunk& data) {
		// Check format
		if (!isThisFormat(data))
			return false;
			
		// Attempt to read image data
		bool ok = readImage(image, data);
		
		// Set format if successful
		if (ok)
			image.format = this;
		
		return ok;
	}

	// Writing
	virtual bool	canWrite(SImage& image) { return false; }
	
	bool saveImage(SImage& image, MemChunk& out, Palette8bit* pal = NULL) {
		// Attempt to write image data
		bool ok = writeImage(image, out, pal);
		
		// Set format if successful
		if (ok)
			image.format = this;
		
		return ok;
	}

	static void			initFormats();
	static SIFormat*	getFormat(string name);
};

#endif//__SIFORMAT_H__
